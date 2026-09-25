// Copyright (c) 2026 Bruce Blay
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/Field.h"
#include <cassert>
#include <cstdlib>
#include <iostream>

// The ensemble's loop, as main.cpp runs it: every 120 ms, trim a quarter of
// the gap between the shared bar and the engine's own. Against a bar well off
// the engine's, it has to settle onto it and stay: through taps and shakes
// too, which change the texture on a bar line without moving the bar.
int main() {
  for (uint32_t seed : {1u, 9u, 0x6669656cu}) {
    field::Engine engine(seed);
    engine.followTempo(engine.bpm());  // as the ensemble does, every 120 ms
    const unsigned first = engine.variation();
    const int64_t span = engine.barSpan(), check = field::rate * 120 / 1000;
    const int64_t origin = span / 3;
    int64_t worst = 0;
    for (int64_t i = 1; i <= int64_t(field::rate) * 40; ++i) {
      engine.sample();
      if (i > int64_t(field::rate) * 15 && i % (field::rate * 3) == 0) {
        if ((i / (field::rate * 3)) % 2) engine.newVariation(); else engine.newBank();
      }
      if (i % check) continue;
      int64_t want = ((i - origin) % span + span) % span;
      int64_t error = ((want - int64_t(engine.barPhaseSamples())) % span + span) % span;
      if (error > span / 2) error -= span;
      engine.trimGrid(int32_t(error / 4));
      if (i > int64_t(field::rate) * 15) worst = std::max(worst, std::llabs(error));
    }
    assert(worst < field::rate * 2 / 1000);
    assert(engine.variation() > first + 5);  // the taps and shakes did land
    std::cout << "seed " << seed << ": settles on the shared bar, worst " << worst << " samples after 15 s, through taps and shakes\n";
  }
}
