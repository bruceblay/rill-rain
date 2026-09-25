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
// A tempo change, as the ensemble makes one: the shared grid takes the new
// tempo on a line, and the engine hears of it at its next 120 ms service.
// It has to stay on the grid through the change, not lose its place.
static void checkTempoChange() {
  for (uint32_t seed : {2u, 3u}) {
    field::Engine engine(seed);
    engine.followTempo(engine.bpm());
    const unsigned slower = engine.bpm() - 8;
    field::Engine sizer(seed);
    sizer.followTempo(slower);
    const int64_t check = field::rate * 120 / 1000;
    int64_t span = engine.barSpan(), origin = span / 3, worst = 0, during = 0, switchAt = -1;
    bool told = false;
    for (int64_t i = 1; i <= int64_t(field::rate) * 50; ++i) {
      engine.sample();
      // The shared grid changes tempo on its first line after 20 s.
      if (switchAt < 0 && i > int64_t(field::rate) * 20 && ((i - origin) % span) == 0) {
        switchAt = i; origin = i; span = sizer.barSpan();
      }
      if (i % check) continue;
      if (switchAt >= 0 && !told) { engine.followTempo(slower); told = true; }
      int64_t want = ((i - origin) % span + span) % span;
      int64_t error = ((want - int64_t(engine.barPhaseSamples())) % span + span) % span;
      if (error > span / 2) error -= span;
      engine.trimGrid(int32_t(error / 4));
      if (switchAt >= 0) {
        if (i < switchAt + int64_t(field::rate) * 8) during = std::max(during, std::llabs(error));
        else worst = std::max(worst, std::llabs(error));
      }
    }
    assert(engine.bpm() == slower);
    assert(worst < field::rate * 2 / 1000);
    std::cout << "seed " << seed << ": " << (slower + 8) << " to " << slower << " BPM, worst " << during
              << " samples in the change, " << worst << " after\n";
  }
}

int main() {
  checkTempoChange();
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
