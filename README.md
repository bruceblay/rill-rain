# Rill World

A generative environment box, a companion to [Rill](https://github.com/bruceblay/rill), for the **M5Stack StickS3**. Rill World plays real field recordings, grouped into banks (Rain, Birds, more to come), through a slow bar-synced filter sweep and a handful of self-clearing effects, so it breathes rather than sitting as a flat loop. Tap for a new recording within the current bank. Shake for a different bank entirely.

It is a sibling instrument, not a Rill feature: same hardware, same GPL-3.0-or-later license and Rill's parent project Pocket Radio, and built with the same host-testable, allocation-free approach, but its own repository, its own engine, and its own aesthetic. It needs no Wi-Fi, account, or cloud service.

## Play

| Gesture | Action |
| --- | --- |
| Front button: tap | Pick a new recording and character within the current bank, change the visual, and play |
| Front button: hold for about 0.65 seconds | Fade sound out or in; the loop continues while quiet |
| Side button: tap | Cycle volume and show the data view for four seconds |
| Shake | Cross into a different bank -- a different environment, a different visual palette and drift |

The data view shows the current bank, recording, generation, tempo, bar count, volume and battery estimate.

## What it is

An earlier version generated its own noise from scratch (filtered white noise standing in for water/rain/wind), in the spirit of a vintage sound conditioner like the Marsona 1200. That synthesis was rejected by ear as universally bad, the same conclusion reached about Rill Drums' own procedural noise voices, so this project pivoted to real recordings instead, and later broadened from a single rain-only device into multiple selectable environments:

- **Four banks so far**: **Rain** (six surfaces -- umbrella, puddle, concrete, terrace, plastic tarpaulin, metal wheelbarrow -- a steady wash through to increasingly percussive and metallic), **Birds** (forest ambience, dawn chorus, evening birds), **Insects** (nocturnal insects, cicada, crickets/frogs meadow), and **Body** (heartbeat, breathing -- a lower-confidence experiment, kept smaller than the others until it's judged on real hardware). All CC0-licensed placeholders (see [docs/SOURCES.md](docs/SOURCES.md)), expected to be replaced with original field recordings. Adding a further bank is data, not a rewrite -- see `src/Field.h`.
- **A bar-synced lowpass sweep** stands in for a vintage sound conditioner's Tone/Surf Rate knobs: narrow range, low resonance, slow enough to read as breathing rather than the main event. Its floor is kept above ~700 Hz -- the StickS3's small speaker barely reproduces anything lower (confirmed by Rill Drums' own on-device measurements), so a sweep that dips below that reads as silence, not warmth.
- **Five punch-in effects** -- Pitch Wobble, Delay Throw, Crush, Reverb, Smear -- one at a time, semi-random, self-clearing after a bar or two, each evolving across its own window rather than sitting at one flat setting. Same direction as Rill Drums' punch-in effects. Magnitude varies per bank: Birds and Insects run bigger, wobblier Smears and a wider Pitch Wobble range than Rain's original tuning (occasionally surreal, by design); Body stays conservative -- a wide pitch swing on a heartbeat reads as uncanny, not surreal.

Device-to-device ensemble sync -- so a Rill World unit could lock its sweep and bar clock to another Rill instrument's shared tempo -- follows the same unimplemented [design proposal](https://github.com/bruceblay/rill/blob/main/SYNC-DESIGN.md) as Rill and Rill Drums; nothing here talks to another device yet. The bar clock is already shaped to receive that later without restructuring.

Playback state is not saved across restarts.

## On the device

The visual is one continuous particle field per bank, not a catalog of families like Rill's or Rill Drums' -- this project's focus is the audio, and the screen only needs to read as alive. A bar boundary briefly brightens a particle near the top, a visible tell for the rhythm that's otherwise only in the audio's filter sweep. Rain's particles are plain falling dots; Birds' are tiny two-stroke chevrons that flap between wings-up and wings-down as they wander; Insects are tiny specks darting erratically; Body reuses Birds' gentle undirected wander, but as plain dots, on its own dark ground. Tap cycles the ink color within a bank; shake crosses to the other bank's ground, ink set, drift and shape entirely.

Color follows the same drawing language as Rill and Rill Drums -- flat opaque ink, a fainter particle mixed toward the ground colour rather than toward black, regardless of which one is lighter. The ground colour is fixed per bank rather than shuffling -- it's how you tell banks apart at a glance: Rain keeps a dark, moody charcoal-blue ground with pale ink (it suits rain at night); Birds inverts that, a light, fun sky-blue ground with dark ink, so its particles read as birds against open sky; Insects gets a true near-black night ground with pale firefly-ish ink; Body gets a dark, intimate maroon ground with warm pale ink.

**Rain** (falling dots, dark charcoal-blue ground)

| | | |
| --- | --- | --- |
| ![Rain visual, pale grey-blue ink](docs/images/rain-visual-1.png) | ![Rain visual, ice-blue ink](docs/images/rain-visual-2.png) | ![Rain visual, pale silver ink](docs/images/rain-visual-3.png) |

**Birds** (flapping chevrons, light sky-blue ground)

| | | |
| --- | --- | --- |
| ![Birds visual, dark olive ink](docs/images/birds-visual-1.png) | ![Birds visual, dark wine ink](docs/images/birds-visual-2.png) | ![Birds visual, dark navy ink](docs/images/birds-visual-3.png) |

**Insects** (darting specks, near-black night ground)

| | | |
| --- | --- | --- |
| ![Insects visual, firefly-yellow ink](docs/images/insects-visual-1.png) | ![Insects visual, amber ink](docs/images/insects-visual-2.png) | ![Insects visual, moonlit blue-white ink](docs/images/insects-visual-3.png) |

**Body** (gentle wander, dark maroon ground)

| | | |
| --- | --- | --- |
| ![Body visual, warm blush ink](docs/images/body-visual-1.png) | ![Body visual, muted rose ink](docs/images/body-visual-2.png) | ![Body visual, pale cream ink](docs/images/body-visual-3.png) |

These are host-rendered previews (`tools/visual_preview.cpp`), pixel-identical to what the firmware pushes to the real screen, not photos of the device -- the animation and bar-tick highlight don't show in a still frame.

## Hardware

Supported and tested: **M5Stack StickS3**, with ESP32-S3, 8 MB flash, display, IMU and built-in speaker. Other ESP32 boards and earlier M5Stick models are not supported by this configuration.

The PlatformIO board name is `esp32-s3-devkitc-1`; the project supplies the StickS3 memory settings and uses M5Unified for board peripherals. Flash is partitioned as a single ~7 MB factory app slot (`partitions_field.csv`), not the usual two-slot OTA layout, so the embedded clips fit -- this device is flashed by USB each time, not updated over the air. With all four banks the partition is now ~96% full; a further bank or longer clips will need shorter/fewer clips or growing the partition further into the ~900 KB still unallocated on the 8 MB chip.

## Build and install

Install Python 3.11 or later, then run these commands from the repository root:

```sh
python3 -m venv .venv
source .venv/bin/activate
python -m pip install -r requirements-dev.txt
pio run
```

On Windows, activate with `.venv\Scripts\activate` instead. PlatformIO downloads the pinned platform and library dependencies on the first build.

Connect the StickS3 with a USB data cable, locate its port with `pio device list`, then install:

```sh
python tools/flash.py --port YOUR_DEVICE_PORT
```

Flashing replaces the firmware currently on the device. The script builds and uploads, then applies the watchdog reset used successfully during Rill's development; a normal RTS reset can leave this board in download mode.

To observe diagnostics:

```sh
pio device monitor --port YOUR_DEVICE_PORT --baud 115200
```

Close the monitor before another upload.

## Develop without hardware

Host tools use the same C++ engine and visual code as the firmware. A C++17 compiler is required.

```sh
python tools/test.py
mkdir -p build
c++ -std=c++17 -O2 tools/render.cpp -o build/render
build/render build/rain.wav 60 42
c++ -std=c++17 -O2 tools/visual_preview.cpp -o build/visual_preview
build/visual_preview build/preview.ppm 23 0
```

The audio arguments are output path, seconds, and optional seed. The visual arguments are output path, seed, and optional regenerate-once flag. Audio output is mono 32 kHz / 16-bit WAV; visual output is PPM. For host address/undefined-behavior checks, run `python tools/test.py --sanitize` with a compatible compiler. Set `CXX` to choose a compiler.

`tools/embed_samples.py` converts mono 16-bit 32 kHz WAV loops into `src/Samples.h`; see that file's docstring and [docs/SOURCES.md](docs/SOURCES.md) before replacing or adding a clip.

Tests cover five simulated minutes of playback per seed, bounded output, a jump/discontinuity bound across crossfades and the loop point, seed reproducibility, per-bank texture coverage, that tap never crosses a bank boundary and shake always can, fade/resume, and shake gesture recognition. They do not replace listening or checking the physical screen.

## Project layout

- `src/Field.h` -- sample playback, bank ranges, bar-synced sweep and punch-in effects
- `src/Samples.h` -- generated PCM data; see `tools/embed_samples.py`
- `src/Weather.h` -- the ambient particle visual, one palette set and drift kind per bank
- `src/main.cpp` -- audio, display, buttons and motion tasks
- `src/ShakeDetector.h` -- gesture recognition, shared with Rill
- `tools/` -- portable tests, auditions, previews, sample embedding and flashing
- `tests/` -- host verification
- `docs/SOURCES.md` -- source recordings and licenses for the embedded clips

## Credits and license

In the spirit of [Rill](https://github.com/bruceblay/rill). Developed through iterative on-device listening and viewing, with Claude assisting implementation.

Rill World follows Rill's parent project Pocket Radio's **GPL-3.0-or-later** license. See [LICENSE](LICENSE). The embedded field recordings are separately licensed CC0 1.0; see [docs/SOURCES.md](docs/SOURCES.md).
