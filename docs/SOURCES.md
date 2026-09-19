# Audio sources

`src/Samples.h` is generated from real field recordings by `tools/embed_samples.py`, not written by hand. The raw downloads are not kept in this repository (they're much larger than the trimmed clips actually embedded); this file is the record of where each embedded clip came from and under what license, so anyone auditing the repo doesn't have to reverse-engineer PCM data to find out.

**These are placeholders.** Bruce intends to replace them with his own recordings; when that happens, update this file to describe the new source (self-recorded, date, location, equipment) and delete the corresponding row below.

| Texture | Title | Author | Source | License |
| --- | --- | --- | --- | --- |
| Water | Mountain Stream #1 | Pierre Sibanarco | [bigsoundbank.com/mountain-stream-1-s2754.html](https://bigsoundbank.com/mountain-stream-1-s2754.html) | CC0 1.0 |
| Rain | Rain Under an Umbrella | BigSoundBank | [bigsoundbank.com/rain-under-an-umbrella-s2679.html](https://bigsoundbank.com/rain-under-an-umbrella-s2679.html) | CC0 1.0 |
| RainPuddle | Rain on Puddle | Joseph Sardin | [bigsoundbank.com/rain-on-puddle-s1290.html](https://bigsoundbank.com/rain-on-puddle-s1290.html) | CC0 1.0 |
| RainConcrete | Rain on Concrete | Joseph Sardin | [bigsoundbank.com/rain-on-concrete-s1289.html](https://bigsoundbank.com/rain-on-concrete-s1289.html) | CC0 1.0 |
| Wind | Wind in the Trees (forest canopy) | BigSoundBank | [bigsoundbank.com/forest-wind-in-the-trees-s0904.html](https://bigsoundbank.com/forest-wind-in-the-trees-s0904.html) | CC0 1.0 |

Rain got two extra flavors (Puddle, Concrete) because the umbrella recording read as the best of the original three by ear; RainPuddle and RainConcrete are more percussive (bigger individual drop impacts) rather than a steady wash, for contrast.

CC0 requires no attribution and permits commercial use, modification and redistribution (confirmed against BigSoundBank's own license page before downloading; this table exists anyway because "I found it on a site that says free" isn't the same as recording the actual license). Credited here as good practice, not because CC0 requires it.

## Processing applied

Each source was downloaded at its native rate (48 kHz, 16 or 24-bit, stereo), then, with `ffmpeg`:

1. Trimmed to an 8-second excerpt from a point that avoided obvious handling noise or scene changes at the recording's start/end (exact offsets: water +90s, rain +3s, rain-puddle +5s, rain-concrete +5s, wind +60s).
2. Downmixed to mono and resampled to 32000 Hz to match `field::rate`.
3. Loudness-normalized (`loudnorm=I=-18:TP=-2:LRA=7`) so the three clips sit at a comparable level; the raw downloads varied enough in level that one would have dominated the mix.

8 seconds was chosen to fit comfortably in the existing flash partition without repartitioning, for a first listen; Bruce's own recordings will likely run longer once the partition question is revisited (see NOTES.md).
