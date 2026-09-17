// Copyright (c) 2026 Bruce Blay
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/Field.h"
#include <cassert>
#include <iostream>
#include <memory>

int main() {
  unsigned texturesSeen = 0;
  uint32_t hitsTotal = 0;
  for (uint32_t seed : {1u, 0x6669656cu, 0xffffffffu}) {
    auto engine = std::unique_ptr<field::Engine>(new field::Engine(seed));
    float previous = 0, peak = 0, jump = 0;
    double energy = 0;
    for (unsigned i = 0; i < field::rate * 300; ++i) {
      // Cycle textures a few times over the run, including a quick repeated tap.
      if (i && i % (field::rate * 11) == 0) engine->newVariation();
      if (i % (field::rate * 11) == field::rate / 10) engine->newVariation();
      float s = engine->sample();
      if (engine->drainHit()) ++hitsTotal;
      assert(std::isfinite(s) && std::abs(s) < 0.9f);
      assert(engine->bpm() >= 56 && engine->bpm() <= 92);
      assert(engine->currentTexture() < field::textureCount);
      assert(engine->currentStep() < field::steps);
      texturesSeen |= 1u << engine->currentTexture();
      peak = std::max(peak, std::abs(s));
      jump = std::max(jump, std::abs(s - previous));
      previous = s; energy += s * s;
    }
    assert(peak > 0.02f && jump < 1.3f);
    assert(std::sqrt(energy / (field::rate * 300)) > 0.002);
    engine->setPlaying(false);
    for (unsigned i = 0; i < field::rate * 8; ++i) previous = engine->sample();
    assert(std::abs(previous) < 0.0001f);
    engine->setPlaying(true);
    energy = 0;
    for (unsigned i = 0; i < field::rate * 8; ++i) { float s = engine->sample(); energy += s * s; }
    assert(energy > 0.0005);
    std::cout << "seed " << seed << ": five-minute stability, headroom, fade/resume passed; peak=" << peak << " jump=" << jump << '\n';
  }
  assert(texturesSeen == 0x7u); // Water, Rain and Wind all appeared.
  assert(hitsTotal > 20); // Rain/Wind transients are audibly alive, not silent.

  auto a = std::unique_ptr<field::Engine>(new field::Engine(42));
  auto b = std::unique_ptr<field::Engine>(new field::Engine(42));
  int16_t block[512];
  for (unsigned i = 0; i < 300; ++i) {
    a->render(block, 512);
    for (auto s : block) assert(s == int16_t(b->sample() * 32767));
  }
  std::cout << "Seed reproducibility and block rendering passed\n";

  auto c = std::unique_ptr<field::Engine>(new field::Engine(7));
  unsigned firstTexture = c->currentTexture();
  c->newVariation();
  assert(c->variation() == 2 && c->currentTexture() != firstTexture);
  std::cout << "New variation advances the generation and always changes texture\n";
}
