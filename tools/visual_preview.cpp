// Copyright (c) 2026 Bruce Blay
// SPDX-License-Identifier: GPL-3.0-or-later
#include "../src/Field.h"
#include "../src/Weather.h"
#include <fstream>
#include <memory>
#include <cstdlib>
int main(int argc, char** argv) {
  if (argc < 2) return 1;
  uint32_t seed = argc > 2 ? std::strtoul(argv[2], nullptr, 10) : 23;
  auto engine = std::unique_ptr<field::Engine>(new field::Engine(seed));
  auto scene = std::unique_ptr<weather::Scene>(new weather::Scene(seed));
  if (argc > 3 && std::strtoul(argv[3], nullptr, 10) % 2 == 1) scene->regenerate();
  // Advance real audio so the visual has bar ticks to react to.
  for (unsigned i = 0; i < field::rate * 6; ++i) {
    engine->sample();
    if ((i % (field::rate / 60)) == 0) scene->render(1.0f / 60, engine->drainBarTick());
  }
  std::ofstream out(argv[1], std::ios::binary);
  out << "P6\n240 135\n255\n";
  for (unsigned i = 0; i < 240 * 135; ++i) {
    uint16_t c = scene->pixels()[i];
    out.put(char(((c >> 11) & 31) * 255 / 31));
    out.put(char(((c >> 5) & 63) * 255 / 63));
    out.put(char((c & 31) * 255 / 31));
  }
  return out ? 0 : 2;
}
