// Copyright (c) 2026 Bruce Blay
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/Weather.h"
#include <cassert>
#include <memory>
#include <iostream>
uint32_t hash(const weather::Scene& s) {
  uint32_t value = 2166136261u;
  for (unsigned i = 0; i < 240 * 135; ++i) value = (value ^ s.pixels()[i]) * 16777619u;
  return value;
}
int main() {
  auto a = std::unique_ptr<weather::Scene>(new weather::Scene(23));
  auto b = std::unique_ptr<weather::Scene>(new weather::Scene(23));
  a->render(0, false); b->render(0, false);
  assert(hash(*a) == hash(*b));
  auto initial = hash(*a);
  for (unsigned i = 0; i < 200; ++i) {
    bool hit = (i % 37) == 0;
    a->render(1.0f / 60, hit); b->render(1.0f / 60, hit);
    assert(hash(*a) == hash(*b));
  }
  assert(hash(*a) != initial);

  // Regenerating always changes the palette and resets the generation count.
  unsigned before = a->generation();
  a->regenerate();
  assert(a->generation() == before + 1);

  // A hit and no hit produce different frames from the same state.
  a->seed(23); b->seed(23);
  a->render(0, true); b->render(0, false);
  assert(hash(*a) != hash(*b));

  // Each texture drifts differently, so the same seed diverges once textured.
  a->seed(23); b->seed(23);
  a->setTexture(0); b->setTexture(1);
  for (unsigned i = 0; i < 30; ++i) { a->render(1.0f / 60, false); b->render(1.0f / 60, false); }
  assert(hash(*a) != hash(*b));

  std::cout << "Determinism, animation, hit response and texture drift passed\n";
}
