# Rill World visual reference

The visual is one continuous particle field per bank, not a catalog of families like Rill's or Rill Drums' -- this project's focus is the audio, and the screen only needs to read as alive. A bar boundary briefly brightens a particle near the top, a visible tell for the rhythm that's otherwise only in the audio's filter sweep. Rain forms tiny expanding pond ripples that fade and reappear at new impact points; Cave keeps falling dots. Birds are tiny two-stroke chevrons that flap as they wander; Insects have little bodies and four fluttering wings, independently alternating between brief hovers, sudden darts, and longer curving flights with fresh directions and durations; Ocean has hollow bubbles drifting slowly upward. Tap cycles the ink color within a bank; shake crosses to the other bank's ground, ink set, drift and shape entirely.

Color follows the same drawing language as Rill and Rill Drums -- flat opaque ink, a fainter particle mixed toward the ground colour rather than toward black, regardless of which one is lighter. The ground colour is fixed per bank rather than shuffling -- it's how you tell banks apart at a glance: Rain is the only dark ground (moody charcoal-blue with pale ink, since it suits rain at night); Birds, Insects, Ocean and Cave all invert that with light grounds and dark ink -- sky-blue for Birds, green for Insects, a lighter turquoise for Ocean, a light purple for Cave. Cave's ground started dark like Rain's and was changed after feedback that it needed a more distinctive look.

**Rain** (expanding pond ripples, dark charcoal-blue ground)

| | | |
| --- | --- | --- |
| ![Rain visual, pale grey-blue ink](docs/images/rain-visual-1.png) | ![Rain visual, ice-blue ink](docs/images/rain-visual-2.png) | ![Rain visual, pale silver ink](docs/images/rain-visual-3.png) |

**Birds** (flapping chevrons, light sky-blue ground)

| | | |
| --- | --- | --- |
| ![Birds visual, dark olive ink](docs/images/birds-visual-1.png) | ![Birds visual, dark wine ink](docs/images/birds-visual-2.png) | ![Birds visual, dark navy ink](docs/images/birds-visual-3.png) |

**Insects** (fluttering insects, light green ground)

| | | |
| --- | --- | --- |
| ![Insects visual, dark forest-green ink](docs/images/insects-visual-1.png) | ![Insects visual, dark amber-brown ink](docs/images/insects-visual-2.png) | ![Insects visual, dark navy ink](docs/images/insects-visual-3.png) |

**Ocean** (hollow bubbles drifting upward, lighter turquoise ground)

| | | |
| --- | --- | --- |
| ![Ocean visual, deep teal ink](docs/images/ocean-visual-1.png) | ![Ocean visual, dark navy ink](docs/images/ocean-visual-2.png) | ![Ocean visual, deep sandy-brown ink](docs/images/ocean-visual-3.png) |

**Cave** (falling dots, light purple ground)

| | | |
| --- | --- | --- |
| ![Cave visual, deep violet ink](docs/images/cave-visual-1.png) | ![Cave visual, dark charcoal-purple ink](docs/images/cave-visual-2.png) | ![Cave visual, dark indigo ink](docs/images/cave-visual-3.png) |

These are host-rendered previews (`tools/visual_preview.cpp`), pixel-identical to what the firmware pushes to the real screen, not photos of the device -- the animation and bar-tick highlight don't show in a still frame.
