// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QImage>
#include <QMargins>
#include <QRect>

class QColor;

namespace Beveled {

// A window shadow as a nine-patch texture, in logical pixels:
//   image      - the texture; the area under the window is transparent
//   innerRect  - the stretched center of the nine-patch; its x/y are the corner sizes
//   padding    - how far the shadow reaches outside the window on each side
// These map one to one onto KDecoration3::DecorationShadow.
struct ShadowTexture
{
    QImage image;
    QRect innerRect;
    QMargins padding;
};

// Bounds of the distance parameter, the "how far the window floats above the desktop" option.
inline constexpr int MinShadowDistance = 1;
inline constexpr int MaxShadowDistance = 16;

// Hard-edged shadow offset by distance pixels down and right, as a flat translucent fill of
// color (alpha included). Square corners, no blur.
ShadowTexture solidShadow(int distance, const QColor &color);

// Blurred shadow in the style of KDE's default decoration: offset downwards by distance pixels,
// with a blur radius that grows with the distance. color sets the darkest point (alpha included).
ShadowTexture softShadow(int distance, const QColor &color);

} // namespace Beveled
