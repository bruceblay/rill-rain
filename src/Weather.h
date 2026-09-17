// Copyright (c) 2026 Bruce Blay
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <array>
#include <algorithm>
#include <cmath>
#include <cstdint>

// One continuous drifting-particle visual, deliberately not a catalog of
// families the way Rill's and Rill Drums' visuals are: this project's focus
// is the audio texture and its sync behavior, and the screen only needs to
// read as "which weather is this" and "is it breathing." Particle drift
// direction and palette follow the active texture; a transient hit briefly
// brightens a particle near the top, the way a raindrop or gust arrives.
// Shared by firmware and the host preview tool.
namespace weather {
class Scene {
 public:
  static constexpr unsigned width = 240, height = 135;
  static constexpr unsigned particleCount = 48;

 private:
  struct Color { float r, g, b; };
  struct Particle { float x, y, speed, size, phase; };
  std::array<uint16_t, width * height> frame{};
  uint32_t rng;
  unsigned texture = 0, palette = 0, count = 0;
  float phase = 0, breath = 0;
  Color ink{};
  uint16_t background = 0;
  std::array<Particle, particleCount> particles{};

  unsigned random() { rng ^= rng << 13; rng ^= rng >> 17; rng ^= rng << 5; return rng; }
  float unit() { return float(random() >> 8) / 16777216.0f; }
  static uint16_t color(Color c, float shade = 1) {
    unsigned r = unsigned(std::max(0.0f, std::min(255.0f, c.r * shade)));
    unsigned g = unsigned(std::max(0.0f, std::min(255.0f, c.g * shade)));
    unsigned b = unsigned(std::max(0.0f, std::min(255.0f, c.b * shade)));
    return uint16_t(((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3));
  }
  void dot(float cx, float cy, float r, Color c, float shade) {
    int top = std::max(0, int(cy - r)), bottom = std::min(int(height) - 1, int(cy + r));
    int left = std::max(0, int(cx - r)), right = std::min(int(width) - 1, int(cx + r));
    for (int y = top; y <= bottom; ++y)
      for (int x = left; x <= right; ++x) {
        float dx = x - cx, dy = y - cy;
        if (dx * dx + dy * dy <= r * r) frame[unsigned(y) * width + unsigned(x)] = color(c, shade);
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

 public:
  explicit Scene(uint32_t value = 23) : rng(value ? value : 1) { regenerate(); }
  void seed(uint32_t value) { rng = value ? value : 1; count = 0; regenerate(); }
  void setTexture(unsigned t) { texture = t; }
  void regenerate() {
    palette = count ? (palette + 1 + random() % 2) % 3 : random() % 3;
    ++count; phase = unit() * 6.283185f; breath = 0;
    static const Color palettes[3] = {
      {88, 168, 214},   // Water: cool blue
      {160, 176, 196},  // Rain: grey-blue
      {158, 200, 140},  // Wind: green-tan
    };
    ink = palettes[palette];
    background = color({10, 12, 18});
    resetParticles();
    frame.fill(background);
  }
  unsigned generation() const { return count; }
  const uint16_t* pixels() const { return frame.data(); }
  void render(float dt, bool hit) {
    dt = std::max(0.0f, std::min(0.1f, dt));
    phase += dt; if (phase > 628.3185f) phase -= 628.3185f;
    breath += (float(hit) - breath) * std::min(1.0f, dt * 3);
    frame.fill(background);
    for (auto& p : particles) {
      switch (texture) {
        case 0: p.x -= p.speed * dt * 24; if (p.x < -2) p.x = width + 2; break;              // Water: flows left
        case 1: p.y += p.speed * dt * 60; if (p.y > height + 2) { p.y = -2; p.x = unit() * width; } break; // Rain: falls
        default: p.x -= p.speed * dt * 10; p.y += p.speed * dt * 6;
                 if (p.x < -2 || p.y > height + 2) { p.x = width + 2; p.y = unit() * height * 0.6f; }
      }
      float shimmer = 0.5f + 0.5f * std::sin(phase * 1.3f + p.phase);
      dot(p.x, p.y, p.size, ink, 0.32f + shimmer * 0.40f);
    }
    if (hit) dot(unit() * width, height * 0.12f, 3, ink, 0.7f + breath * 0.3f);
  }
};
}
