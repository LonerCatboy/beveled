<!--
SPDX-FileCopyrightText: 2026 LonerCatboy
SPDX-License-Identifier: CC0-1.0
-->

# Beveled

A classic 3D-bevel theme (late-'90s style) for KDE Plasma 6, drawn in code and crisp at every
scale.

> **Status:** early development, not usable yet.

## Goal

Existing retro themes are built on scaled SVGs (Kvantum, Aurorae) and show blurry lines and
antialiasing artifacts, especially at fractional scales. Beveled draws bevels, frames and glyphs
in code, in physical pixels: integer line widths, coordinates aligned to the device pixel grid,
no antialiasing on geometric primitives.

## Components

| Component | Directory | Status |
|---|---|---|
| Drawing primitives library | `libbeveled/` | in progress |
| Application style (QStyle, also QML/Kirigami) | `style/` | planned |
| Window decoration (KDecoration3) | `decoration/` | planned |
| Light and dark color schemes | `colors/` | planned |
| Plasma style, cursors, icons, splash, global theme | — | later |

## Building

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

## Trying the style on a single application

On a test machine, without installing anything or changing global settings:

```bash
QT_PLUGIN_PATH=$PWD/build/bin kwrite -style beveled
```

## Licensing

The project follows the [REUSE](https://reuse.software/) specification:

- code: GPL-2.0-or-later
- artwork: CC-BY-SA-4.0
- configuration files and documentation: CC0-1.0

Full license texts are in `LICENSES/`.

## Disclaimer

Independent project, not affiliated with or endorsed by Microsoft. It contains no Microsoft
assets, fonts or trademarks.
