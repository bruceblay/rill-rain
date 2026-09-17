# Rill Field

A generative texture companion to [Rill](https://github.com/bruceblay/rill), for the **M5Stack StickS3**. Rill Field is a rhythmic white-noise machine: three filtered-noise textures (water, rain, wind) breathe on a bar-synced swell instead of sitting as flat hiss. Tap for a new texture. Shake for a different visual.

It is a sibling instrument, not a Rill feature: same hardware, same GPL-3.0-or-later license and Rill's parent project Pocket Radio, and built with the same host-testable, allocation-free approach, but its own repository, its own generative engine, and its own aesthetic. It needs no Wi-Fi, account, audio files, or cloud service — the textures are synthesized filtered noise, not field recordings.

## Play

| Gesture | Action |
| --- | --- |
| Front button: tap | Generate a new texture and character, change the visual, and play |
| Front button: hold for about 0.65 seconds | Fade sound out or in; the swell continues while quiet |
| Side button: tap | Cycle volume and show the data view for four seconds |
| Shake | Immediately switch the ambient visual, without changing the texture |

The data view shows the texture, generation, tempo, bar count, volume and battery estimate.

## Design basis: vintage sound conditioners, not a sample bank

Digital "white noise machine" apps are usually a bank of looped field recordings. Rill Field instead follows the older analog machines: the Marpac/SleepMate line generates its hiss mechanically from a spinning fan with a rotating vent for tone, and the Marsona 1200/TSC-330 synthesizes Rain, Surf and Waterfall from filtered noise behind just four knobs — Volume, Tone, and a Surf Rate/Range pair whose LFO swells the filter cutoff to make the "waves." Rill Field keeps that shape:

- **Three textures**, each a state-variable-filtered noise bed rather than a sample: **Water** (a mid-band stream brightened by a fast shimmer), **Rain** (a bright hiss bed with soft droplets scheduled on a step grid), and **Wind** (a low-mid band that wanders, with sparser leaf-rustle transients).
- **A bar-synced swell** stands in for the Marsona's Surf Rate/Range: the filter cutoff (and, on Rain/Wind, how often a transient fires) breathes on a multi-bar cycle tied to the same 16-step/tempo clock Rill and Rill Drums use, rather than a free-running LFO.
- **Scheduled, not scattered, transients**: Rain's droplets and Wind's rustles are gated to a sparse Euclidean-style step pattern, so the texture reads as quietly rhythmic instead of uniformly random.

Device-to-device ensemble sync — so a Rill Field unit could lock its swell and step grid to another Rill instrument's shared tempo and bar boundary — follows the same unimplemented [design proposal](https://github.com/bruceblay/rill/blob/main/SYNC-DESIGN.md) as Rill and Rill Drums; nothing here talks to another device yet. The tempo/bar clock is already shaped to receive that later without restructuring.

Textures are not saved across restarts.

## Hardware

Supported and tested: **M5Stack StickS3**, with ESP32-S3, 8 MB flash, display, IMU and built-in speaker. Other ESP32 boards and earlier M5Stick models are not supported by this configuration.

The PlatformIO board name is `esp32-s3-devkitc-1`; the project supplies the StickS3 memory settings and uses M5Unified for board peripherals.

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

Host tools use the same C++ synthesis and visual code as the firmware. A C++17 compiler is required.

```sh
python tools/test.py
mkdir -p build
c++ -std=c++17 -O2 tools/render.cpp -o build/render
build/render build/field.wav 60 42
c++ -std=c++17 -O2 tools/visual_preview.cpp -o build/visual_preview
build/visual_preview build/preview.ppm 23 0
```

The audio arguments are output path, seconds, and optional seed. The visual arguments are output path, seed, and optional regenerate-once flag. Audio output is mono 32 kHz / 16-bit WAV; visual output is PPM. For host address/undefined-behavior checks, run `python tools/test.py --sanitize` with a compatible compiler. Set `CXX` to choose a compiler.

Tests cover five simulated minutes of texture per seed, bounded output, a jump/discontinuity bound across texture crossfades, seed reproducibility, texture coverage across generations, fade/resume, and shake gesture recognition. They do not replace listening or checking the physical screen.

## Project layout

- `src/Field.h` — noise synthesis, bar-synced swell and transient scheduling
- `src/Weather.h` — the ambient particle visual
- `src/main.cpp` — audio, display, buttons and motion tasks
- `src/ShakeDetector.h` — gesture recognition, shared with Rill
- `tools/` — portable tests, auditions, previews and flashing
- `tests/` — host verification

## Credits and license

Created by Bruce Blay, in the spirit of [Rill](https://github.com/bruceblay/rill). Developed through iterative on-device listening and viewing, with Claude assisting implementation.

Rill Field follows Rill's parent project Pocket Radio's **GPL-3.0-or-later** license. See [LICENSE](LICENSE).
