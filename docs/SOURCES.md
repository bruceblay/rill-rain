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
| BirdForest | Forest | Joseph Sardin | [bigsoundbank.com/forest-s0100.html](https://bigsoundbank.com/forest-s0100.html) | CC0 1.0 |
| BirdWake | Birds Waking #3 | Joseph Sardin | [bigsoundbank.com/wake-birds-3-s0999.html](https://bigsoundbank.com/wake-birds-3-s0999.html) | CC0 1.0 |
| BirdEvening | Evening Birds | Joseph Sardin | [bigsoundbank.com/evening-birds-s1859.html](https://bigsoundbank.com/evening-birds-s1859.html) | CC0 1.0 |
| InsectNight | Nocturnal Insects #4 | Joseph Sardin | [bigsoundbank.com/nocturnal-insects-4-s1470.html](https://bigsoundbank.com/nocturnal-insects-4-s1470.html) | CC0 1.0 |
| InsectCrickets | Campaign at Night #4 | Joseph Sardin | [bigsoundbank.com/campaign-at-night-4-s1880.html](https://bigsoundbank.com/campaign-at-night-4-s1880.html) | CC0 1.0 |
| OceanWaves1 | Sea Waves | Joseph Sardin | [bigsoundbank.com/sea-waves-s0698.html](https://bigsoundbank.com/sea-waves-s0698.html) | CC0 1.0 |
| OceanWaves2 | Sea: Waves | Joseph Sardin | [bigsoundbank.com/sea-waves-s0266.html](https://bigsoundbank.com/sea-waves-s0266.html) | CC0 1.0 |
| OceanWhale | Humpback whale song | NOAA Fisheries / NOAA Pacific Islands Fisheries Science Center Passive Acoustics Group | [fisheries.noaa.gov/national/science-data/sounds-ocean-mammals](https://www.fisheries.noaa.gov/national/science-data/sounds-ocean-mammals) | Public domain (US federal government work, 17 U.S.C. §105) |

This started as three textures (Water, Rain, Wind); Water and Wind were dropped and Rain expanded to six surfaces once the project became rain-specific. Birds and Insects were added after that as further banks. A cicada clip was tried in Insects and dropped ("off-putting"). A Body bank (heartbeat, breathing) was tried and dropped entirely ("not very interesting"), replaced by Ocean. See CHANGELOG.md and NOTES.md for that history.

The whale clip is the only non-CC0 source here: it's a NOAA Fisheries recording, and works created by US federal employees as part of their official duties are public domain in the US (not copyrighted at all, so no license grant is needed) -- a stronger guarantee than CC0's voluntary dedication.

CC0 requires no attribution and permits commercial use, modification and redistribution (confirmed against BigSoundBank's own license page before downloading; this table exists anyway because "I found it on a site that says free" isn't the same as recording the actual license). Credited here as good practice, not because CC0 requires it.

## Processing applied

Each source was downloaded at its native rate (48 kHz, 16 or 24-bit, mono or stereo), then, with `ffmpeg`:

1. Trimmed to an excerpt from a point that avoided obvious handling noise or scene changes at the recording's start/end (exact offsets: rain +3s, puddle +5s, concrete +5s, terrace +30s, tarpaulin +10s, wheelbarrow +20s, forest +5s, birds waking +30s, evening birds +30s, nocturnal insects +30s, crickets/frogs +60s, sea waves +20s, sea swirls +5s, whale +8s). Rain and Birds clips are 8 seconds; Insects and Ocean are 6 seconds, trimmed shorter specifically to fit the remaining flash budget once four banks existed. The whale clip was additionally cut down from NOAA's 54-second original.
2. Downmixed to mono and resampled to 32000 Hz to match `field::rate`.
3. Loudness-normalized (`loudnorm=I=-18:TP=-2:LRA=7`) so the clips sit at a comparable level; the raw downloads varied enough in level that one would have dominated the mix.

The flash layout is a single ~7 MB factory app slot (`partitions_field.csv`, no OTA). With all four banks it now sits at ~96% of that partition (~265 KB free) -- comfortable for now, but the next bank or any lengthening of an existing clip will need either shorter clips, dropping a clip, or growing the partition further into the ~900 KB still unallocated on the 8 MB chip.
