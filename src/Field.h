// Copyright (c) 2026 Bruce Blay
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>

// A bounded, deterministic generative noise machine shared by firmware and
// audition. No allocation, locks, or transcendental functions in a way that
// would break the per-sample budget. Three filtered-noise textures, no
// sample playback, in the spirit of a vintage electronic sound conditioner
// (Marsona-style tone/rate/range knobs) rather than a bank of field
// recordings: one texture active at a time, its filter cutoff and transient
// density riding a bar-synced swell so the wash reads as rhythmic instead of
// flat hiss. The swell and step clock are deliberately the same shape as
// Rill's proposed ensemble tempo/bar broadcast (see SYNC-DESIGN.md in the
// Rill repository) so a future conductor packet only needs to set this
// engine's tempo and bar phase, not restructure it.
namespace field {
constexpr uint32_t rate = 32000;
constexpr float pi = 3.14159265358979323846f;
constexpr unsigned steps = 16;
enum Texture : unsigned { Water = 0, Rain, Wind, textureCount };

class Engine {
  struct Character {
    float cutoffHz, q;              // resting filter color
    float swellDepthHz, swellBars;  // slow bar-synced brighten/darken, like Surf Rate/Range
    float rippleHz, rippleDepthHz;  // fast free-running shimmer (water only)
    float transientChance;          // probability a scheduled step actually fires
    float transientDecay, transientColorHz, transientQ, transientGain;
    float bedGain;
  };
  struct Transient {
    bool active = false;
    float amp = 0, decay = 1, svfLow = 0, svfBand = 0, svfF = 0.5f, svfQ = 0.5f;
  };

  uint32_t rng, noiseRng = 0x9e3779b9u, scoreRng = 1, transientRng = 1;
  unsigned texture = 0, generation = 0;
  unsigned tempo = 56;
  uint32_t stepSamples = rate * 60 / (56 * 4);
  uint64_t clock = 0, nextStep = 0;
  unsigned stepIndex = 0, bar = 0;
  uint16_t accentPattern = 0;

  float bedSvfLow = 0, bedSvfBand = 0;
  float swellPhase = 0, swellStep = 0;
  float ripplePhase = 0, rippleStep = 0;
  std::array<Transient, 4> transients{};
  unsigned nextTransient = 0;
  bool firedThisBlock = false;

  // Crossfade across a texture change so a new spectrum never arrives as a click.
  enum Xfade : unsigned { Steady = 0, FadingOut, FadingIn };
  unsigned xfadeState = Steady;
  float xfadeGain = 1;

  float outputRamp = 0, target = 1, level = 0;
  float dcIn = 0, dcOut = 0;

  uint32_t random() { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
  float unit() { return float(random() >> 8) / 16777216.0f; }
  uint32_t scoreRandom() { scoreRng ^= scoreRng << 13; scoreRng ^= scoreRng >> 17; scoreRng ^= scoreRng << 5; return scoreRng; }
  float scoreUnit() { return float(scoreRandom() >> 8) / 16777216.0f; }
  uint32_t transientRandom() { transientRng ^= transientRng << 13; transientRng ^= transientRng >> 17; transientRng ^= transientRng << 5; return transientRng; }
  float transientUnit() { return float(transientRandom() >> 8) / 16777216.0f; }
  float noise() {
    noiseRng ^= noiseRng << 13; noiseRng ^= noiseRng >> 17; noiseRng ^= noiseRng << 5;
    return float(int32_t(noiseRng)) / 2147483648.0f;
  }
  static float svfCoeff(float hz) {
    // Kept with real headroom below Nyquist/6: at Q above ~1 the Chamberlin
    // state-variable filter starts to self-oscillate as its coefficient
    // approaches that bound, aliasing into a full-scale buzz rather than a
    // colored transient (found by a host jump-bound test failing on Rain's
    // droplet filter at 5200 Hz / Q 1.3).
    float clamped = std::min(hz, rate * 0.12f);
    return 2.0f * std::sin(pi * clamped / rate);
  }

  const Character& active_() const { return characters()[texture]; }
  static const std::array<Character, textureCount>& characters() {
    // Tuned by ear against the built-in speaker's rolloff, not against a
    // reference monitor. Rain and Wind lean on transientChance for their
    // rhythm; Water instead leans on rippleHz, the way a stream's texture
    // moves without ever hitting discrete events.
    static const std::array<Character, textureCount> table{{
      // Water: a running-stream band, brightened by a fast shimmer as well
      // as the slow bar swell. No discrete transients.
      {1100, 1.1f,  700, 3,   0.9f, 260,   0.0f, 0.15f, 2200, 0.6f, 0.0f,  0.42f},
      // Rain: a high, soft hiss bed plus frequent short bright droplets
      // scheduled on the step grid.
      {3000, 0.55f, 400, 4,   0.0f, 0,     0.34f, 0.12f, 3400, 0.85f, 0.55f, 0.30f},
      // Wind / leaves: a low-mid band that wanders slowly, with occasional
      // longer gusts and sparse leaf-rustle transients.
      {450,  0.7f,  900, 6,   0.0f, 0,     0.16f, 0.35f, 1500, 0.5f, 0.35f, 0.46f},
    }};
    return table;
  }

  void scheduleAccents() {
    // A sparse, semi-random pattern of scheduled steps; whether one actually
    // fires still passes through transientChance in stepClock().
    unsigned pulses = 3 + scoreRandom() % 5;
    pulses = std::min(pulses, steps);
    uint16_t bits = 0;
    for (unsigned i = 0; i < steps; ++i) if ((i * pulses) % steps < pulses) bits |= uint16_t(1u << i);
    unsigned rotation = scoreRandom() % steps;
    accentPattern = uint16_t(((bits >> rotation) | (bits << (steps - rotation))) & 0xffffu);
  }

  void triggerTransient() {
    const Character& c = active_();
    Transient& t = transients[nextTransient];
    nextTransient = (nextTransient + 1) % transients.size();
    t.active = true;
    t.amp = c.transientGain * (0.6f + 0.4f * transientUnit());
    t.decay = std::exp(-1.0f / (rate * c.transientDecay * (0.7f + 0.6f * transientUnit())));
    t.svfF = svfCoeff(c.transientColorHz * (0.85f + 0.3f * transientUnit()));
    t.svfQ = c.transientQ;
    t.svfLow = t.svfBand = 0;
    firedThisBlock = true;
  }

  void stepClock() {
    if (clock < nextStep) return;
    const Character& c = active_();
    if ((accentPattern >> stepIndex) & 1u) {
      if (transientUnit() < c.transientChance) triggerTransient();
    }
    nextStep += stepSamples;
    if (++stepIndex == steps) {
      stepIndex = 0;
      ++bar;
    }
  }

  void beginXfade() { xfadeState = FadingOut; }

  void applyCharacter() {
    tempo = 56 + random() % 37; // 56-92 BPM, overlapping Rill's and Rill Drums' ranges
    stepSamples = rate * 60 / (tempo * 4);
    scoreRng = (rng ^ 0x51ed270bu) | 1u;
    transientRng = (rng ^ 0xa341316cu) | 1u;
    stepIndex = 0; bar = 0; nextStep = clock;
    scheduleAccents();
    swellStep = 2 * pi / (active_().swellBars * steps * stepSamples);
    rippleStep = 2 * pi * active_().rippleHz / rate;
    swellPhase = unit() * 2 * pi;
    ripplePhase = 0;
    bedSvfLow = bedSvfBand = 0;
    for (auto& t : transients) t.active = false;
  }

 public:
  explicit Engine(uint32_t value = 0x6669656c) { seed(value); }
  void seed(uint32_t value) {
    rng = value ? value : 1;
    // xorshift32 correlates its first output with a small seed value; a few
    // throwaway iterations avoid every low seed picking the same initial
    // texture (found by rendering seeds 1-20 for a listen and getting Water
    // every time).
    for (unsigned i = 0; i < 6; ++i) random();
    generation = 0;
    generate();
  }
  void generate() {
    texture = generation ? (texture + 1 + random() % (textureCount - 1)) % textureCount : random() % textureCount;
    ++generation;
    if (generation == 1) applyCharacter(); else beginXfade();
  }
  void newVariation() { generate(); }
  void setPlaying(bool playing) { target = playing ? 1.0f : 0.0f; }

  unsigned variation() const { return generation; }
  unsigned bpm() const { return tempo; }
  unsigned currentTexture() const { return texture; }
  unsigned currentStep() const { return stepIndex; }
  unsigned barCount() const { return bar; }
  uint32_t displayInfo() const { return (generation << 9) | (texture << 7) | tempo; }
  bool drainHit() { bool h = firedThisBlock; firedThisBlock = false; return h; }

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

    swellPhase += swellStep; if (swellPhase > 2 * pi) swellPhase -= 2 * pi;
    float swell = 0.5f + 0.5f * std::sin(swellPhase);
    float cutoff = c.cutoffHz + c.swellDepthHz * swell;
    if (c.rippleDepthHz > 0) {
      ripplePhase += rippleStep; if (ripplePhase > 2 * pi) ripplePhase -= 2 * pi;
      cutoff += c.rippleDepthHz * std::sin(ripplePhase);
    }
    float bedF = svfCoeff(std::max(60.0f, cutoff));
    float input = noise();
    float notch = input - c.q * bedSvfBand;
    bedSvfLow += bedF * bedSvfBand;
    float high = notch - bedSvfLow;
    bedSvfBand += bedF * high;
    float bed = bedSvfBand * c.bedGain * (0.85f + 0.15f * swell);

    float transientSum = 0;
    for (auto& t : transients) {
      if (!t.active) continue;
      float in = noise();
      float tn = in - t.svfQ * t.svfBand;
      t.svfLow += t.svfF * t.svfBand;
      float th = tn - t.svfLow;
      t.svfBand += t.svfF * th;
      transientSum += t.svfBand * t.amp;
      t.amp *= t.decay;
      if (std::abs(t.amp) < 0.0005f) t.active = false;
    }

    float mix = (bed + transientSum) * xfadeGain;
    float clean = mix - dcIn + 0.999f * dcOut;
    dcIn = mix; dcOut = clean;
    level += (target - level) / (rate * 0.2f);
    outputRamp = std::min(1.0f, outputRamp + 1.0f / (rate * 0.08f));
    float x = clean * level * outputRamp * 1.6f;
    return x / (1 + std::abs(x));
  }
  void render(int16_t* output, unsigned count) {
    for (unsigned i = 0; i < count; ++i) output[i] = int16_t(sample() * 32767);
  }
};
}
