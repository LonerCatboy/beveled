// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QRect>

class QPainter;
class QPalette;

namespace Beveled {

enum class Glyph {
    Minimize,
    Maximize,
    Restore,
    Close,
    Help,
    KeepAbove,
    KeepBelow,
    OnAllDesktops,
    Shade,
};

// How far the glyph moves from its resting position.
enum class GlyphOffset {
    None,
    Pressed, // one unit down and right, as on a pressed classic button
};

// Size of the caption button the glyphs are designed for, in logical pixels.
inline constexpr QSize CaptionButtonSize(16, 14);

// Draws a caption glyph inside logicalButtonRect, at the same position as on a classic 16x14
// caption button. Larger or smaller buttons keep the glyph centered by the same offset.
// Disabled glyphs are drawn embossed (Light shifted one pixel, then Dark on top).
void drawCaptionGlyph(
    QPainter *painter, const QRect &logicalButtonRect, Glyph glyph, const QPalette &palette, bool enabled = true, GlyphOffset offset = GlyphOffset::None);

} // namespace Beveled
