# Audio sources

`src/Samples.h` is generated from real field recordings by `tools/embed_samples.py`, not written by hand. The raw downloads are not kept in this repository (they're much larger than the trimmed clips actually embedded); this file is the record of where each embedded clip came from and under what license, so anyone auditing the repo doesn't have to reverse-engineer PCM data to find out.

**These are placeholders**, expected to be replaced with original field recordings. When that happens, update this file to describe the new source (self-recorded, date, location, equipment) and delete the corresponding row below.

| Texture | Title | Author | Source | License |
| --- | --- | --- | --- | --- |
| Rain | Rain Under an Umbrella | BigSoundBank | [bigsoundbank.com/rain-under-an-umbrella-s2679.html](https://bigsoundbank.com/rain-under-an-umbrella-s2679.html) | CC0 1.0 |
| RainPuddle | Rain on Puddle | Joseph Sardin | [bigsoundbank.com/rain-on-puddle-s1290.html](https://bigsoundbank.com/rain-on-puddle-s1290.html) | CC0 1.0 |
| RainConcrete | Rain on Concrete | Joseph Sardin | [bigsoundbank.com/rain-on-concrete-s1289.html](https://bigsoundbank.com/rain-on-concrete-s1289.html) | CC0 1.0 |
| RainTerrace | Summer Rain on Terrace | Joseph Sardin | [bigsoundbank.com/summer-rain-on-terrace-s1019.html](https://bigsoundbank.com/summer-rain-on-terrace-s1019.html) | CC0 1.0 |
| RainTarpaulin | Rain on Plastic Tarpaulin | Joseph Sardin | [bigsoundbank.com/rain-on-plastic-tarpaulin-s1292.html](https://bigsoundbank.com/rain-on-plastic-tarpaulin-s1292.html) | CC0 1.0 |
| RainWheelbarrow | Rain on Wheelbarrow | Joseph Sardin | [bigsoundbank.com/rain-on-wheelbarrow-s1291.html](https://bigsoundbank.com/rain-on-wheelbarrow-s1291.html) | CC0 1.0 |

This started as three textures (Water, Rain, Wind); Water and Wind were dropped and Rain expanded to five surfaces once the project became rain-specific -- see CHANGELOG.md and NOTES.md for that history.

CC0 requires no attribution and permits commercial use, modification and redistribution (confirmed against BigSoundBank's own license page before downloading; this table exists anyway because "I found it on a site that says free" isn't the same as recording the actual license). Credited here as good practice, not because CC0 requires it.

## Processing applied

Each source was downloaded at its native rate (48 kHz, 16 or 24-bit, mono or stereo), then, with `ffmpeg`:

1. Trimmed to an 8-second excerpt from a point that avoided obvious handling noise or scene changes at the recording's start/end (exact offsets: rain +3s, puddle +5s, concrete +5s, terrace +30s, tarpaulin +10s, wheelbarrow +20s).
2. Downmixed to mono and resampled to 32000 Hz to match `field::rate`.
3. Loudness-normalized (`loudnorm=I=-18:TP=-2:LRA=7`) so the clips sit at a comparable level; the raw downloads varied enough in level that one would have dominated the mix.

8 seconds per clip was chosen for a first listen. The flash layout was since repartitioned to a single ~7 MB factory app slot (`partitions_field.csv`, no OTA) specifically to leave room for more or longer clips, since any eventual replacement recordings will likely run longer than these.
