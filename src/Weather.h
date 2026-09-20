// Copyright (c) 2026 Bruce Blay
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

// One continuous drifting-particle visual, deliberately not a catalog of
// families the way Rill's and Rill Drums' visuals are: this project's focus
// is the audio, and the screen only needs to read as alive. A bar boundary
// briefly brightens a particle near the top, a visible tell for the rhythm
// that's otherwise only in the audio's filter sweep. Shared by firmware and
// the host preview tool.
//
// Color follows the same drawing language as Rill and Rill Drums: flat
// opaque ink, no additive glow, and a fainter particle is paler (mixed
// toward the ground colour) rather than darker (mixed toward black) --
// mixing toward black is what made early passes on those projects read as
// a neon screensaver. This project keeps a dark ground rather than their
// daylight one (it suits rain at night), including one palette with a
// genuinely blue ground rather than near-black, but the mixing rule and the
// four fixed shade steps (not a continuous ramp) carry over unchanged.
//
// Deliberately decoupled from Field.h -- this header knows nothing of
// field::Bank -- but its own bankCount and per-bank palette/drift tables are
// meant to track field::bankCount 1:1, kept in sync by hand. Adding a bank
// (body sounds, insects) means a new entry in bankPalettes() and a new
// DriftKind case, not a restructure.
namespace weather {
class Scene {
 public:
  static constexpr unsigned width = 240, height = 135;
  static constexpr unsigned particleCount = 48;
  static constexpr unsigned bankCount = 2;

 private:
  struct Color { float r, g, b; };
  struct Palette { Color ground, ink; };
  struct BankPalettes { const Palette* palettes; unsigned count; };
  struct Particle { float x, y, speed, size, phase; };
  enum DriftKind : unsigned { Fall = 0, Drift };
  std::array<uint16_t, width * height> frame{};
  uint32_t rng;
  unsigned bank = 0, palette = 0, count = 0;
  float phase = 0, breath = 0;
  Color ground{}, ink{};
  uint16_t backgroundPacked = 0;
  std::array<Particle, particleCount> particles{};

  unsigned random() { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
  float unit() { return float(random() >> 8) / 16777216.0f; }
  static uint16_t pack(Color c) {
    unsigned r = unsigned(std::max(0.0f, std::min(255.0f, c.r)));
    unsigned g = unsigned(std::max(0.0f, std::min(255.0f, c.g)));
    unsigned b = unsigned(std::max(0.0f, std::min(255.0f, c.b)));
    return uint16_t(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
  }
  // A fainter particle mixes toward the ground colour, not toward black,
  // and lands on one of four fixed steps rather than a continuous ramp.
  uint16_t mix(float shade) const {
    static const float steps[4] = {0.25f, 0.5f, 0.75f, 1.0f};
    float s = steps[std::min<unsigned>(3, unsigned(std::max(0.0f, shade) * 4))];
    return pack({ground.r + (ink.r - ground.r) * s,
                 ground.g + (ink.g - ground.g) * s,
                 ground.b + (ink.b - ground.b) * s});
  }
  void dot(float cx, float cy, float r, float shade) {
    int top = std::max(0, int(cy - r)), bottom = std::min(int(height) - 1, int(cy + r));
    int left = std::max(0, int(cx - r)), right = std::min(int(width) - 1, int(cx + r));
    uint16_t c = mix(shade);
    for (int y = top; y <= bottom; ++y)
      for (int x = left; x <= right; ++x) {
        float dx = x - cx, dy = y - cy;
        if (dx * dx + dy * dy <= r * r) frame[unsigned(y) * width + unsigned(x)] = c;
      }
  }
  void resetParticles() {
    for (auto& p : particles) {
      p.x = unit() * width; p.y = unit() * height;
      p.speed = 0.4f + unit() * 0.8f;
      p.size = 0.8f + unit() * 1.6f;
      p.phase = unit() * 6.283185f;
    }
  }
  static const std::array<BankPalettes, bankCount>& bankPalettes() {
    static const Palette rain[3] = {
      {{16, 20, 30}, {190, 200, 215}},  // charcoal-blue ground, pale grey-blue ink
      {{18, 26, 64}, {205, 218, 240}},  // genuinely blue ground, ice-blue ink
      {{15, 15, 19}, {212, 212, 217}},  // near-neutral dark ground, pale silver ink
    };
    static const Palette birds[3] = {
      {{18, 22, 16}, {200, 205, 180}},  // dark mossy-green ground, pale warm ink (forest/dawn)
      {{24, 18, 28}, {215, 195, 210}},  // dark plum ground, pale lilac ink (dusk)
      {{18, 24, 58}, {200, 212, 235}},  // blue ground, the same family accent as Rain's blue palette
    };
    static const std::array<BankPalettes, bankCount> table{{ {rain, 3}, {birds, 3} }};
    return table;
  }
  static DriftKind driftForBank(unsigned b) { return b == 0 ? Fall : Drift; }

 public:
  explicit Scene(uint32_t value = 23) { seed(value); }
  void seed(uint32_t value) {
    rng = value ? value : 1;
    // xorshift32 correlates its first output with a small seed value; a few
    // throwaway iterations avoid every low seed picking the same initial
    // palette (same fix as field::Engine::seed(), same underlying bug).
    for (unsigned i = 0; i < 6; ++i) random();
    count = 0;
    regenerate();
  }
  void setBank(unsigned b) { bank = std::min(b, bankCount - 1); }
  void regenerate() {
    const BankPalettes& bp = bankPalettes()[bank];
    palette = count ? (palette + 1 + random() % std::max(1u, bp.count - 1)) % bp.count : random() % bp.count;
    ++count; phase = unit() * 6.283185f; breath = 0;
    ground = bp.palettes[palette].ground;
    ink = bp.palettes[palette].ink;
    backgroundPacked = pack(ground);
    resetParticles();
    frame.fill(backgroundPacked);
  }
  unsigned generation() const { return count; }
  const uint16_t* pixels() const { return frame.data(); }
  void render(float dt, bool beat) {
    dt = std::max(0.0f, std::min(0.1f, dt));
    phase += dt; if (phase > 628.3185f) phase -= 628.3185f;
    breath += (float(beat) - breath) * std::min(1.0f, dt * 3);
    frame.fill(backgroundPacked);
    DriftKind drift = driftForBank(bank);
    for (auto& p : particles) {
      switch (drift) {
        case Drift: // birds: gentle wander, no fixed direction
          p.x += std::sin(phase * 0.5f + p.phase) * p.speed * dt * 10;
          p.y += std::cos(phase * 0.3f + p.phase) * p.speed * dt * 6;
          if (p.x < -2) p.x = width + 2; else if (p.x > width + 2) p.x = -2;
          if (p.y < -2) p.y = height + 2; else if (p.y > height + 2) p.y = -2;
          break;
        default: // Fall
          p.y += p.speed * dt * 60;
          if (p.y > height + 2) { p.y = -2; p.x = unit() * width; }
      }
      float shimmer = 0.5f + 0.5f * std::sin(phase * 1.3f + p.phase);
      dot(p.x, p.y, p.size, 0.4f + shimmer * 0.6f);
    }
    if (beat) dot(unit() * width, height * 0.12f, 3, 0.75f + breath * 0.25f);
  }
};
}
