// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QColor>
#include <QRect>

class QPainter;
class QPalette;

namespace Beveled {

// One ring of a bevel: the top/left color and the bottom/right color. As in the classic
// look, the bottom/right color owns the corners.
struct Ring
{
    QColor topLeft;
    QColor bottomRight;
};

// A classic two-ring bevel, outer ring first.
struct BevelColors
{
    Ring outer;
    Ring inner;
};

// The palette roles map to the classic 3D colors:
//   Light = ButtonHighlight, Midlight = 3DLight, Dark = ButtonShadow, Shadow = 3DDkShadow.
enum class BevelStyle {
    Window, // window border: Midlight/Shadow, then Light/Dark
    Raised, // push and caption buttons: Light/Shadow, then Midlight/Dark
    Pressed, // pressed push button: flat Shadow ring, then flat Dark ring
    Pushed, // pressed caption/tool button: Shadow/Light, then Dark/Midlight
    RaisedHard, // high-contrast variant of Raised: both rings Light/Shadow
    PushedHard, // high-contrast variant of Pushed: both rings Shadow/Light
    Sunken, // text fields, list views: Dark/Light, then Shadow/Midlight
    SunkenHard, // thicker light edge: Dark/Light, then Shadow/Light (2px of Light at bottom/right)
    Etched, // group boxes, toolbar bands: Dark/Light, then Light/Dark
};

BevelColors bevelColors(BevelStyle style, const QPalette &palette);

// Draws a two-ring bevel along the edges of logicalRect. Each ring is ringWidth logical pixels
// thick (rounded to whole device pixels). The inside is not filled.
void drawBevel(QPainter *painter, const QRect &logicalRect, const BevelColors &colors, int ringWidth = 1);
void drawBevel(QPainter *painter, const QRect &logicalRect, BevelStyle style, const QPalette &palette, int ringWidth = 1);

// Logical thickness of a two-ring bevel, for layout and style metrics.
constexpr int bevelThickness(int ringWidth = 1)
{
    return 2 * ringWidth;
}

// Fills logicalRect with edges rounded to the device pixel grid, so fills always meet bevels.
void fillRect(QPainter *painter, const QRect &logicalRect, const QColor &color);

// Glass-like gloss over logicalRect (e.g. a title bar), blended over what is already there: a
// bright reflection fading down to the middle, a sharp edge, then a faint glow at the bottom.
// highlight is the reflection color (the palette's Light); only its hue is used.
void drawGloss(QPainter *painter, const QRect &logicalRect, const QColor &highlight);

// Horizontal etched separator (Dark line above a Light line) at the vertical center of
// logicalRect, and its vertical counterpart (Dark left, Light right).
void drawEtchedLine(QPainter *painter, const QRect &logicalRect, Qt::Orientation orientation, const QPalette &palette, int ringWidth = 1);

} // namespace Beveled
