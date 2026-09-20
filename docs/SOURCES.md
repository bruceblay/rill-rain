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
| InsectFieldCricket | Field cricket | Joseph Sardin | [bigsoundbank.com/field-cricket-s1020.html](https://bigsoundbank.com/field-cricket-s1020.html) | CC0 1.0 |
| InsectGrasshopper | Insect song #2 | Joseph Sardin | [bigsoundbank.com/insect-song-2-s3185.html](https://bigsoundbank.com/insect-song-2-s3185.html) | CC0 1.0 |
| OceanWaves1 | Sea Waves | Joseph Sardin | [bigsoundbank.com/sea-waves-s0698.html](https://bigsoundbank.com/sea-waves-s0698.html) | CC0 1.0 |
| OceanWaves2 | Sea: Waves | Joseph Sardin | [bigsoundbank.com/sea-waves-s0266.html](https://bigsoundbank.com/sea-waves-s0266.html) | CC0 1.0 |
| OceanUnderwater | Underwater (hydrophone recording of a waterfall) | Joseph Sardin & Axeline T. | [bigsoundbank.com/underwater-s3430.html](https://bigsoundbank.com/underwater-s3430.html) | CC0 1.0 |

This started as three textures (Water, Rain, Wind); Water and Wind were dropped and Rain expanded to six surfaces once the project became rain-specific. Birds and Insects were added after that as further banks. A cicada clip was tried in Insects and dropped ("off-putting"); a field cricket and a lone night grasshopper were added later for variety (two clips per bank "feels very repetitive"). A Body bank (heartbeat, breathing) was tried and dropped entirely ("not very interesting"), replaced by Ocean. Ocean's third clip went through several tries: a humpback whale song read as too weak/low on this speaker; a bottlenose dolphin's sharp repeated clicks were unpleasant on a loop ("a bad sound to hear repetitively"); a Weddell seal trill was downloaded and tuned but never embedded once its license turned out to be unverified (see below); a tern-calls wave recording was avoided from the start, to keep Ocean from being mistaken for the Birds bank. The underwater hydrophone recording -- an ocean sound rather than an ocean animal -- avoided all of these problems and is cleanly CC0. See CHANGELOG.md and NOTES.md for that history.

Two NOAA Fisheries marine-mammal clips were tried earlier (humpback whale, bottlenose dolphin, both since removed) and were genuinely public domain: both were credited to NOAA's own Passive Acoustics Group, and works produced by US federal employees in their official capacity carry no copyright at all under 17 U.S.C. §105 -- a stronger guarantee than CC0's voluntary dedication. The Weddell seal clip considered after them was credited to an external university researcher (Terhune & Lewe, University of New Brunswick) merely hosted on a NOAA gallery page with no license statement covering that hosting, so it was dropped before ever reaching this file or the device -- credit to a non-federal source breaks the public-domain reasoning that applied to the other two.

CC0 requires no attribution and permits commercial use, modification and redistribution (confirmed against BigSoundBank's own license page before downloading; this table exists anyway because "I found it on a site that says free" isn't the same as recording the actual license). Credited here as good practice, not because CC0 requires it.

## Processing applied

Each source was downloaded at its native rate (48 kHz, 16 or 24-bit, mono or stereo), then, with `ffmpeg`:

1. Trimmed to an excerpt from a point that avoided obvious handling noise or scene changes at the recording's start/end (exact offsets: rain +3s, puddle +5s, concrete +5s, terrace +30s, tarpaulin +10s, wheelbarrow +20s, forest +5s, birds waking +30s, evening birds +30s, nocturnal insects +30s, crickets/frogs +60s, field cricket +5s, grasshopper +0s, sea waves +20s, sea swirls +5s, underwater +10s). Rain and Birds clips are 8 seconds; Insects and Ocean are 6 seconds.
2. Downmixed to mono and resampled to 32000 Hz to match `field::rate`.
3. Loudness-normalized (`loudnorm=I=-18:TP=-2:LRA=7`) so the clips sit at a comparable level; the raw downloads varied enough in level that one would have dominated the mix.

The flash layout is a single factory app slot (`partitions_field.csv`, no OTA), grown once already (from ~7 MB to ~7.6 MB) to make room for these clips. It now sits at ~97% of that partition (~212 KB free) -- the next bank or any lengthening of an existing clip will need shorter/fewer clips or another partition grow into the remaining unallocated space on the 8 MB chip.
