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
| Window decoration (KDecoration3) | `decoration/` | in progress |
| Light and dark color schemes | `colors/` | light only |
| Plasma style, cursors, icons, splash, global theme | — | later |

## Building

```bash
cmake --preset dev
cmake --build --preset dev
ctest --preset dev
```

## Installing on a test machine

Beveled is not packaged yet. On a disposable test machine (e.g. a Fedora KDE VM) it can be
installed into `/usr`, where System Settings finds it.

Build dependencies (Fedora):

```bash
sudo dnf install git cmake ninja-build gcc-c++ extra-cmake-modules qt6-qtbase-devel \
  kf6-kconfig-devel kf6-kcoreaddons-devel kf6-kcolorscheme-devel kf6-kconfigwidgets-devel \
  kf6-kcmutils-devel kdecoration-devel
```

Build, test and install:

```bash
cmake --preset vm
cmake --build --preset vm
ctest --preset vm
sudo cmake --install build-vm
```

Then choose **Beveled** in System Settings → Colors & Themes → Window Decorations, and
**Beveled Classic** in Colors.

The decoration options (title bar style, button bevel, button padding) are behind the edit
button of Beveled in Window Decorations, and are stored in `~/.config/beveledrc`.

To update, there is no need to uninstall first: the new install overwrites the old files.

```bash
git pull
cmake --build --preset vm
sudo cmake --install build-vm
```

Then log out and back in: KWin keeps the previously loaded plugin in memory.

Uninstall:

```bash
sudo xargs -a build-vm/install_manifest.txt rm -v
```

If the decoration crashes KWin and the session becomes unusable, switch to a text console
(Ctrl+Alt+F3), log in, run the uninstall command above and reboot: KWin falls back to the
default decoration.

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
