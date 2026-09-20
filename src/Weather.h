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
// a neon screensaver. That rule holds regardless of which colour is
// lighter: mix toward the ground either way, land on one of four fixed
// shade steps rather than a continuous ramp.
//
// The ground colour is fixed per bank -- it's how a bank reads at a glance,
// not something that shuffles -- while a handful of ink colours vary within
// a bank on regenerate(). Rain keeps a dark, moody charcoal-blue ground
// with pale ink (it suits rain at night). Birds, Insects and Ocean all
// invert that: light grounds (sky-blue, green, turquoise) with dark ink,
// so their particles read as things against daylight rather than stars
// against night.
//
// Deliberately decoupled from Field.h -- this header knows nothing of
// field::Bank -- but its own bankCount and per-bank palette/drift/shape
// tables are meant to track field::bankCount 1:1, kept in sync by hand
// (bank index order: Rain, Birds, Insects, Ocean, Cave). Adding a bank
// means a new entry in each table, not a restructure.
namespace weather {
class Scene {
 public:
  static constexpr unsigned width = 240, height = 135;
  static constexpr unsigned particleCount = 48;
  static constexpr unsigned bankCount = 5;

 private:
  struct Color { float r, g, b; };
  struct BankPalette { Color background; const Color* inks; unsigned inkCount; };
  struct Particle { float x, y, speed, size, phase; };
  enum DriftKind : unsigned { Fall = 0, Drift, Dart, Rise };
  enum ShapeKind : unsigned { Round = 0, Chevron };
  std::array<uint16_t, width * height> frame{};
  uint32_t rng;
  unsigned bank = 0, inkIndex = 0, count = 0;
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
  void plot(int x, int y, uint16_t c) {
    if (x >= 0 && x < int(width) && y >= 0 && y < int(height)) frame[unsigned(y) * width + unsigned(x)] = c;
  }
  void strokeLine(float x0, float y0, float x1, float y1, uint16_t c) {
    int steps = int(std::max(std::abs(x1 - x0), std::abs(y1 - y0))) + 1;
    for (int i = 0; i <= steps; ++i) {
      float t = float(i) / steps;
      plot(int(x0 + (x1 - x0) * t), int(y0 + (y1 - y0) * t), c);
    }
  }
  // A tiny two-stroke chevron -- the plainest possible bird silhouette --
  // flapping between wings-up and wings-down as it drifts.
  void bird(float cx, float cy, float r, float flapPhase, float shade) {
    uint16_t c = mix(shade);
    float wing = r * std::sin(flapPhase);
    strokeLine(cx - r, cy + wing * 0.5f, cx, cy - wing, c);
    strokeLine(cx, cy - wing, cx + r, cy + wing * 0.5f, c);
  }
  void resetParticles() {
    // Birds get a bit more wingspan than rain gets dot size, so the
    // two-stroke chevron actually reads at this resolution. Insects go the
    // other way -- tiny darting specks.
    float sizeMul = shapeForBank(bank) == Chevron ? 1.6f
                  : driftForBank(bank) == Dart ? 0.6f : 1.0f;
    for (auto& p : particles) {
      p.x = unit() * width; p.y = unit() * height;
      p.speed = 0.4f + unit() * 0.8f;
      p.size = (0.8f + unit() * 1.6f) * sizeMul;
      p.phase = unit() * 6.283185f;
    }
  }
  static const std::array<BankPalette, bankCount>& bankPalettes() {
    static const Color rainInks[3] = {
      {190, 200, 215},  // pale grey-blue
      {205, 218, 240},  // ice blue
      {212, 212, 217},  // pale silver
    };
    static const Color birdInks[3] = {
      {70, 90, 45},   // dark olive (forest/dawn)
      {90, 45, 68},   // dark wine (dusk)
      {35, 45, 75},   // dark navy (neutral)
    };
    static const Color insectInks[3] = {
      {30, 60, 25},   // dark forest green
      {90, 60, 20},   // dark amber-brown
      {25, 35, 55},   // dark navy
    };
    static const Color oceanInks[3] = {
      {15, 55, 60},   // deep teal
      {20, 35, 70},   // dark navy
      {70, 50, 25},   // deep sandy brown
    };
    static const Color caveInks[3] = {
      {200, 195, 185}, // pale limestone
      {180, 190, 200}, // pale cool grey
      {210, 200, 175}, // pale sandstone
    };
    static const std::array<BankPalette, bankCount> table{{
      {{16, 20, 30}, rainInks, 3},     // Rain: moody charcoal-blue ground
      {{130, 195, 240}, birdInks, 3},  // Birds: light sky-blue ground, dark ink -- birds against open sky
      {{160, 215, 130}, insectInks, 3},// Insects: light green ground, dark ink
      {{110, 195, 220}, oceanInks, 3}, // Ocean: lighter turquoise water, dark ink, bubbles rising
      {{28, 26, 24}, caveInks, 3},     // Cave: dark stone ground, pale mineral ink, drips falling
    }};
    return table;
  }
  static DriftKind driftForBank(unsigned b) {
    switch (b) {
      case 0: return Fall;
      case 2: return Dart;
      case 3: return Rise;
      case 4: return Fall;
      default: return Drift;
    }
  }
  static ShapeKind shapeForBank(unsigned b) { return b == 1 ? Chevron : Round; }

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
    const BankPalette& bp = bankPalettes()[bank];
    inkIndex = count ? (inkIndex + 1 + random() % std::max(1u, bp.inkCount - 1)) % bp.inkCount : random() % bp.inkCount;
    ++count; phase = unit() * 6.283185f; breath = 0;
    ground = bp.background;
    ink = bp.inks[inkIndex];
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
    ShapeKind shape = shapeForBank(bank);
    for (auto& p : particles) {
      switch (drift) {
        case Drift: // birds: gentle wander, no fixed direction
          p.x += std::sin(phase * 0.5f + p.phase) * p.speed * dt * 10;
          p.y += std::cos(phase * 0.3f + p.phase) * p.speed * dt * 6;
          if (p.x < -2) p.x = width + 2; else if (p.x > width + 2) p.x = -2;
          if (p.y < -2) p.y = height + 2; else if (p.y > height + 2) p.y = -2;
          break;
        case Dart: // insects: quick jittery darting, no fixed direction
          p.x += std::sin(phase * 9.0f + p.phase * 7.0f) * p.speed * dt * 40;
          p.y += std::cos(phase * 11.0f + p.phase * 5.0f) * p.speed * dt * 40;
          if (p.x < -2) p.x = width + 2; else if (p.x > width + 2) p.x = -2;
          if (p.y < -2) p.y = height + 2; else if (p.y > height + 2) p.y = -2;
          break;
        case Rise: // ocean: bubbles drifting upward with a slight wobble
          p.y -= p.speed * dt * 20;
          p.x += std::sin(phase * 0.4f + p.phase) * dt * 4;
          if (p.y < -2) { p.y = height + 2; p.x = unit() * width; }
          break;
        default: // Fall
          p.y += p.speed * dt * 60;
          if (p.y > height + 2) { p.y = -2; p.x = unit() * width; }
      }
      float shimmer = 0.5f + 0.5f * std::sin(phase * 1.3f + p.phase);
      float shade = 0.4f + shimmer * 0.6f;
      if (shape == Chevron) bird(p.x, p.y, p.size, phase * 8.0f + p.phase * 3.0f, shade);
      else dot(p.x, p.y, p.size, shade);
    }
    if (beat) dot(unit() * width, height * 0.12f, 3, 0.75f + breath * 0.25f);
  }
};
}
