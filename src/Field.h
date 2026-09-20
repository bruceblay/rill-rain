// Copyright (c) 2026 Bruce Blay
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include "Samples.h"

// A bounded, deterministic playback engine shared by firmware and audition.
// No allocation, locks, or unbounded work in the per-sample path. Real field
// recordings (see docs/SOURCES.md), not generated noise -- an earlier
// from-scratch filtered-noise synthesis was tried first and rejected by ear
// as universally bad, the same conclusion Rill Drums reached about its own
// procedural noise voices.
//
// A continuous sine sweep on the filter, at a wide range, read as "the main
// event" instead of texture -- narrowed the range and slowed it down.
// A stepped, sample-and-hold version (a new random cutoff on a rhythmic
// pattern, held rather than swept) was tried in between and didn't work out
// by ear either; reverted to the sweep. Five punch-in effects (Pitch
// Wobble, Delay Throw, Crush, Reverb, Smear) still layer on top for variety
// across a longer sit-and-listen, the same direction Rill Drums' punch-in
// effects took: occasional, self-clearing, and evolving while active rather
// than one flat setting. Crush sweeps its hold depth in and back out rather
// than snapping to a fixed amount; Smear is Delay Throw's blurrier sibling,
// wobbling its tap length and damping each repeat so the echoes smear into
// the bed instead of reading as a discrete, clean echo.
//
// This still stands in for a future ensemble conductor's shared tempo and
// bar boundary (see SYNC-DESIGN.md in the Rill repository): the bar clock
// here is exactly what a conductor packet would eventually drive.
namespace field {
constexpr uint32_t rate = 32000;
constexpr float pi = 3.14159265358979323846f;
constexpr unsigned steps = 16;
// Two banks so far: six recordings of rain hitting different surfaces
// (umbrella cloth, puddle, concrete, terrace tile, a plastic tarpaulin, a
// metal wheelbarrow), and three bird ambiences (forest, dawn chorus,
// evening). The engine and its effects are generic over "whatever clips are
// in the current bank," so a further bank (body sounds, insects) is just
// more Texture entries, more Character rows and a new BankRange -- tap
// still only picks within the current bank, shake (newBank()) is the only
// thing that crosses a bank boundary.
enum Texture : unsigned { Rain = 0, RainPuddle, RainConcrete, RainTerrace, RainTarpaulin, RainWheelbarrow,
                           BirdForest, BirdWake, BirdEvening, textureCount };
enum Bank : unsigned { BankRain = 0, BankBirds, bankCount };
enum Punch : unsigned { PunchNone = 0, PunchPitchWobble, PunchDelayThrow, PunchCrush, PunchReverb, PunchSmear, punchCount };

class Engine {
  struct Character {
    float cutoffLowHz, cutoffHighHz; // sweep range (dark <-> open)
    float q, gain, swellBars;        // swellBars: bars per full breath
  };

  struct BankRange { unsigned first, count; };

  uint32_t rng, punchRng = 1;
  unsigned bank = 0, texture = 0, generation = 0;
  unsigned tempo = 56;
  uint32_t stepSamples = rate * 60 / (56 * 4);
  uint32_t barSamples = stepSamples * steps, barPhase = 0;
  uint64_t clock = 0;
  unsigned bar = 0;
  bool barTick = false;

  float readPos = 0, playRate = 1;
  static constexpr uint32_t crossfadeLen = 4000; // ~125 ms, avoids a click at the loop point

  float svfLow = 0, svfBand = 0;
  float swellPhase = 0, swellStep = 0;

  enum Xfade : unsigned { Steady = 0, FadingOut, FadingIn };
  unsigned xfadeState = Steady;
  float xfadeGain = 1;

  // Punch-in variety, one at a time, self-clearing after its window.
  unsigned punchType = PunchNone;
  uint64_t punchStartAt = 0, punchEndAt = 0;
  float punchPitchTarget = 1;
  std::array<int16_t, rate> delay{}; // 1 s, shared by Delay Throw and Smear
  unsigned delayWrite = 0, delayTapSamples = rate / 6;
  float delayFeedback = 0, delayMix = 0, delayDamp = 0;
  float smearWobblePhase = 0, smearWobbleStep = 0;
  unsigned crushMaxHold = 1, crushHoldCounter = 0;
  float crushHeldSample = 0;
  // A short comb + allpass diffuser for Reverb, the same shape as Rill
  // Drums' room send, just fully off except during the punch window.
  std::array<float, 1601> room{};
  unsigned roomIndex = 0;
  float roomDamping = 0;
  std::array<float, 233> diffuser{};
  unsigned diffuserIndex = 0;
  float reverbMix = 0;

  float outputRamp = 0, target = 1, level = 0;
  float dcIn = 0, dcOut = 0;

  uint32_t random() { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
  float unit() { return float(random() >> 8) / 16777216.0f; }
  uint32_t punchRandom() { punchRng ^= punchRng << 13; punchRng ^= punchRng >> 17; punchRng ^= punchRng << 5; return punchRng; }
  float punchUnit() { return float(punchRandom() >> 8) / 16777216.0f; }
  static float svfCoeff(float hz) {
    float clamped = std::min(hz, rate * 0.2f);
    return 2.0f * std::sin(pi * clamped / rate);
  }

  const Character& active_() const { return characters()[texture]; }
  static const std::array<Character, textureCount>& characters() {
    // Narrower ranges and slower swells than the first pass, which was
    // called too fast and too much the predominant thing being heard.
    // Q kept low so the sweep colors the recording rather than resonating.
    // Floors kept at or above ~700 Hz: this speaker (confirmed by Rill
    // Drums' own measurements, see its NOTES.md) barely reproduces
    // anything lower, so a sweep that dips below that doesn't read as
    // "dark," it reads as silence.
    static const std::array<Character, textureCount> table{{
      {1000, 3200, 0.35f, 0.9f, 12},  // Rain (umbrella): steady wash
      {1000, 3200, 0.4f, 0.95f, 10},  // Rain on puddle: percussive drops
      {1000, 3200, 0.4f, 0.95f, 10},  // Rain on concrete: percussive drops
      {1000, 3200, 0.4f, 0.9f, 11},   // Rain on terrace: big storm drops
      {1200, 3600, 0.45f, 0.9f, 9},   // Rain on tarpaulin: brighter, plasticky
      {1200, 4000, 0.5f, 0.85f, 9},   // Rain on wheelbarrow: metallic, most resonant
      {1200, 4200, 0.35f, 0.85f, 12}, // Bird forest ambience: steady bed, chirps laced through
      {1200, 3800, 0.35f, 0.7f, 12},  // Birds waking: dawn chorus -- toned down, ran busy at full brightness/gain
      {1100, 3800, 0.35f, 0.85f, 13}, // Evening birds: calmer, built to loop
    }};
    return table;
  }

  static const std::array<BankRange, bankCount>& bankRanges() {
    static const std::array<BankRange, bankCount> table{{
      {Rain, 6},       // all six rain clips
      {BirdForest, 3}, // all three bird clips
    }};
    return table;
  }

  // Punch magnitudes vary by bank, not just by punch type: Birds runs
  // bigger, wobblier smears and a wider pitch-wobble range than Rain, so it
  // occasionally tips into something a little surreal rather than staying
  // a tasteful accent throughout.
  struct PunchStyle {
    float smearFeedback, smearMix, smearWobbleAmp;
    unsigned smearTapBase, smearTapRange;
    float pitchLow, pitchRange;
  };
  static const std::array<PunchStyle, bankCount>& punchStyles() {
    static const std::array<PunchStyle, bankCount> table{{
      {0.45f, 0.50f, 40.0f, 3, 4, 0.82f, 0.32f}, // Rain: as originally tuned
      {0.60f, 0.65f, 90.0f, 4, 8, 0.65f, 0.55f}, // Birds: bigger smear, wider pitch swing
    }};
    return table;
  }

  unsigned pickTextureInBank(unsigned b, bool avoidCurrent) {
    const BankRange& r = bankRanges()[b];
    if (!avoidCurrent || r.count <= 1) return r.first + random() % r.count;
    unsigned localCurrent = texture - r.first;
    return r.first + (localCurrent + 1 + random() % (r.count - 1)) % r.count;
  }

  void applyCharacter() {
    tempo = 56 + random() % 37; // 56-92 BPM, overlapping Rill's and Rill Drums' ranges
    stepSamples = rate * 60 / (tempo * 4);
    barSamples = stepSamples * steps;
    punchRng = (rng ^ 0xc2b2ae35u) | 1u;
    barPhase = 0; bar = 0;
    swellStep = 2 * pi / (active_().swellBars * barSamples);
    swellPhase = unit() * 2 * pi;
    svfLow = svfBand = 0;
    readPos = 0; playRate = 1;
    punchType = PunchNone; punchPitchTarget = 1;
    delayFeedback = delayMix = delayDamp = 0;
    crushHoldCounter = 0;
    reverbMix = roomDamping = 0;
    room.fill(0); diffuser.fill(0);
    smearWobblePhase = 0;
  }

  void beginXfade() { xfadeState = FadingOut; }

  void stepClock() {
    if (++barPhase >= barSamples) { barPhase = 0; ++bar; barTick = true; maybePunch(); }
  }

  void maybePunch() {
    if (punchType != PunchNone || punchUnit() > 0.30f) return;
    punchType = 1 + punchRandom() % (punchCount - 1);
    punchStartAt = clock;
    punchEndAt = clock + uint64_t(stepSamples) * steps * (1 + punchRandom() % 2); // one or two bars
    const PunchStyle& ps = punchStyles()[bank];
    switch (punchType) {
      case PunchPitchWobble:
        punchPitchTarget = ps.pitchLow + punchUnit() * ps.pitchRange;
        break;
      case PunchDelayThrow:
        delayTapSamples = std::min<unsigned>(delay.size() - 1, stepSamples * (2 + punchRandom() % 5));
        break;
      case PunchCrush:
        crushMaxHold = 3 + punchRandom() % 5; // light: 3-7 samples held at the deepest point
        crushHoldCounter = 0;
        break;
      case PunchSmear:
        delayTapSamples = std::min<unsigned>(delay.size() - 1, stepSamples * (ps.smearTapBase + punchRandom() % ps.smearTapRange));
        smearWobbleStep = 2 * pi * (0.1f + punchUnit() * 0.15f) / rate; // slow, ~0.1-0.25 Hz
        break;
      default: break;
    }
  }

  float punchProgress() const {
    if (punchEndAt <= punchStartAt) return 1;
    return std::min(1.0f, float(clock - punchStartAt) / float(punchEndAt - punchStartAt));
  }

  float readSample() {
    const samples::Clip& clip = samples::textures[texture];
    uint32_t length = clip.length;
    // texture switches immediately on newVariation() while the crossfade
    // still plays out the old clip's tail, so readPos may briefly belong to
    // a differently-sized clip; guard rather than assume the lengths match.
    if (readPos >= length) readPos = std::fmod(readPos, float(length));
    uint32_t i0 = uint32_t(readPos);
    uint32_t i1 = (i0 + 1 < length) ? i0 + 1 : 0;
    float frac = readPos - i0;
    float value = clip.data[i0] / 32768.0f + (clip.data[i1] / 32768.0f - clip.data[i0] / 32768.0f) * frac;
    uint32_t tailStart = length - crossfadeLen;
    if (i0 >= tailStart) {
      float t = std::min(1.0f, (readPos - tailStart) / crossfadeLen);
      uint32_t h0 = i0 - tailStart;
      uint32_t h1 = h0 + 1 < crossfadeLen ? h0 + 1 : h0;
      float head = clip.data[h0] / 32768.0f + (clip.data[h1] / 32768.0f - clip.data[h0] / 32768.0f) * frac;
      value = value * (1 - t) + head * t;
    }
    readPos += playRate;
    if (readPos >= length) readPos -= length;
    return value;
  }

 public:
  explicit Engine(uint32_t value = 0x6669656c) { seed(value); }
  void seed(uint32_t value) {
    rng = value ? value : 1;
    for (unsigned i = 0; i < 6; ++i) random(); // avoid small-seed correlation on the first texture pick
    generation = 0;
    generate();
  }
  void generate() {
    texture = pickTextureInBank(bank, generation != 0);
    ++generation;
    if (generation == 1) applyCharacter(); else beginXfade();
  }
  void newVariation() { generate(); }
  // Shake: cross into a different bank (a no-op today, with only one bank
  // to switch to -- becomes live the moment a second bank's data exists,
  // no further engine changes needed).
  void newBank() {
    if (bankCount <= 1) return;
    bank = (bank + 1 + random() % (bankCount - 1)) % bankCount;
    texture = pickTextureInBank(bank, false);
    ++generation;
    beginXfade();
  }
  void setPlaying(bool playing) { target = playing ? 1.0f : 0.0f; }

  unsigned variation() const { return generation; }
  unsigned bpm() const { return tempo; }
  unsigned currentTexture() const { return texture; }
  unsigned currentBank() const { return bank; }
  unsigned barCount() const { return bar; }
  unsigned currentPunch() const { return punchType; }
  uint32_t displayInfo() const { return (generation << 11) | (texture << 7) | tempo; }
  bool drainBarTick() { bool t = barTick; barTick = false; return t; }

  float sample() {
    if (xfadeState == FadingOut) {
      xfadeGain -= 1.0f / (rate * 0.18f);
      if (xfadeGain <= 0) { xfadeGain = 0; applyCharacter(); xfadeState = FadingIn; }
    } else if (xfadeState == FadingIn) {
      xfadeGain += 1.0f / (rate * 0.18f);
      if (xfadeGain >= 1) { xfadeGain = 1; xfadeState = Steady; }
    }
    stepClock(); ++clock;
    const Character& c = active_();

    playRate += ((punchType == PunchPitchWobble ? punchPitchTarget : 1.0f) - playRate) / (rate * 0.15f);
    swellPhase += swellStep; if (swellPhase > 2 * pi) swellPhase -= 2 * pi;
    float swell = 0.5f + 0.5f * std::sin(swellPhase);
    float cutoff = c.cutoffLowHz + (c.cutoffHighHz - c.cutoffLowHz) * swell;
    float f = svfCoeff(cutoff);
    float input = readSample();
    float notch = input - c.q * svfBand;
    svfLow += f * svfBand;
    float high = notch - svfLow;
    svfBand += f * high;
    float filtered = svfLow * c.gain;

    bool smear = punchType == PunchSmear;
    const PunchStyle& ps = punchStyles()[bank];
    float delayProgress = (punchType == PunchDelayThrow || smear) ? std::sin(punchProgress() * pi) : 0;
    delayFeedback += ((smear ? ps.smearFeedback : 0.30f) * delayProgress - delayFeedback) / (rate * 0.05f);
    delayMix += ((smear ? ps.smearMix : 0.35f) * delayProgress - delayMix) / (rate * 0.05f);
    // Smear wobbles its tap length and darkens each repeat, so the echoes
    // blur into the bed instead of reading as a discrete, clean echo.
    smearWobblePhase += smearWobbleStep; if (smearWobblePhase > 2 * pi) smearWobblePhase -= 2 * pi;
    float wobbleSamples = smear ? ps.smearWobbleAmp * delayProgress : 0.0f;
    unsigned tapNow = unsigned(std::max(1.0f, delayTapSamples + std::sin(smearWobblePhase) * wobbleSamples));
    unsigned readIndex = unsigned((delayWrite + delay.size() - std::min<unsigned>(delay.size() - 1, tapNow)) % delay.size());
    float delayed = delay[readIndex] / 32768.0f;
    delayDamp += (smear ? 0.5f : 0.15f) * (delayed - delayDamp);
    float delayedTone = smear ? delayDamp : delayed;
    float delayWriteValue = filtered + delayedTone * delayFeedback;
    delay[delayWrite] = int16_t(std::max(-0.98f, std::min(0.98f, delayWriteValue)) * 32767);
    if (++delayWrite == delay.size()) delayWrite = 0;
    float withDelay = filtered + delayedTone * delayMix;

    // Sample-rate crush: holds the output for a slowly swept number of
    // samples (1 at rest, up to crushMaxHold at the peak of the window and
    // back), rather than a fixed crush depth snapping on and off.
    float crushed = withDelay;
    if (punchType == PunchCrush) {
      float depth = std::sin(punchProgress() * pi);
      unsigned holdN = 1 + unsigned(depth * crushMaxHold);
      if (crushHoldCounter == 0) crushHeldSample = withDelay;
      crushHoldCounter = (crushHoldCounter + 1) % std::max(1u, holdN);
      crushed = crushHeldSample;
    } else {
      crushHoldCounter = 0;
    }

    // Reverb: a short comb + allpass diffuser, silent except during its
    // punch window, where it fades in and back out with the signal.
    float reverbTarget = punchType == PunchReverb ? 0.35f * std::sin(punchProgress() * pi) : 0.0f;
    reverbMix += (reverbTarget - reverbMix) / (rate * 0.05f);
    float delayedRoom = room[roomIndex % room.size()];
    roomDamping += 0.30f * (delayedRoom - roomDamping);
    room[roomIndex % room.size()] = crushed * 0.5f + roomDamping * 0.35f;
    ++roomIndex;
    float diffA = diffuser[diffuserIndex];
    diffuser[diffuserIndex] = delayedRoom + diffA * 0.5f;
    float wet = diffA - diffuser[diffuserIndex] * 0.5f;
    if (++diffuserIndex == diffuser.size()) diffuserIndex = 0;
    float withReverb = crushed + wet * reverbMix;

    if (punchType != PunchNone && clock >= punchEndAt) {
      punchType = PunchNone; punchPitchTarget = 1;
    }

    float mix = withReverb * xfadeGain;
    float clean = mix - dcIn + 0.999f * dcOut;
    dcIn = mix; dcOut = clean;
    level += (target - level) / (rate * 0.2f);
    outputRamp = std::min(1.0f, outputRamp + 1.0f / (rate * 0.08f));
    float x = clean * level * outputRamp;
    return x / (1 + std::abs(x));
  }
  void render(int16_t* output, unsigned count) {
    for (unsigned i = 0; i < count; ++i) output[i] = int16_t(sample() * 32767);
  }
};
}
