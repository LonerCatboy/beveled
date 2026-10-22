// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "glyphs.h"

#include "pixelpainter.h"

#include <QPalette>

#include <array>
#include <span>
#include <string_view>

namespace Beveled {

namespace {

// A glyph as a 1x bitmap ('#' = ink) and its position inside a 16x14 caption button.
struct GlyphBitmap
{
    QPoint offset;
    std::span<const std::string_view> rows;
};

constexpr std::array<std::string_view, 2> minimizeRows = {
    "######",
    "######",
};

constexpr std::array<std::string_view, 9> maximizeRows = {
    "#########",
    "#########",
    "#.......#",
    "#.......#",
    "#.......#",
    "#.......#",
    "#.......#",
    "#.......#",
    "#########",
};

constexpr std::array<std::string_view, 9> restoreRows = {
    "..######",
    "..######",
    "..#....#",
    "######.#",
    "######.#",
    "#....###",
    "#....#..",
    "#....#..",
    "######..",
};

constexpr std::array<std::string_view, 7> closeRows = {
    "##....##",
    ".##..##.",
    "..####..",
    "...##...",
    "..####..",
    ".##..##.",
    "##....##",
};

GlyphBitmap bitmap(Glyph glyph)
{
    switch (glyph) {
    case Glyph::Minimize:
        return {QPoint(4, 9), minimizeRows};
    case Glyph::Maximize:
        return {QPoint(3, 2), maximizeRows};
    case Glyph::Restore:
        return {QPoint(3, 2), restoreRows};
    case Glyph::Close:
        return {QPoint(4, 3), closeRows};
    }
    return {};
}

void drawBitmap(PixelPainter &pp, const GlyphBitmap &glyph, const QPoint &origin, const QColor &color)
{
    const int u = pp.unit();
    for (int y = 0; y < int(glyph.rows.size()); ++y) {
        const std::string_view row = glyph.rows[y];
        // Merge horizontal runs of ink into a single rectangle.
        for (int x = 0; x < int(row.size());) {
            if (row[x] != '#') {
                ++x;
                continue;
            }
            const int start = x;
            while (x < int(row.size()) && row[x] == '#') {
                ++x;
            }
            pp.fillRect(QRect(origin.x() + start * u, origin.y() + y * u, (x - start) * u, u), color);
        }
    }
}

} // namespace

void drawCaptionGlyph(QPainter *painter, const QRect &logicalButtonRect, Glyph glyph, const QPalette &palette, bool enabled, bool pressed)
{
    PixelPainter pp(painter);
    const int u = pp.unit();
    const QRect button = pp.toDevice(logicalButtonRect);
    const GlyphBitmap bmp = bitmap(glyph);

    const QPoint centering((button.width() - CaptionButtonSize.width() * u) / 2, (button.height() - CaptionButtonSize.height() * u) / 2);
    QPoint origin = button.topLeft() + centering + bmp.offset * u;
    if (pressed) {
        origin += QPoint(u, u);
    }

    if (enabled) {
        drawBitmap(pp, bmp, origin, palette.color(QPalette::Active, QPalette::ButtonText));
    } else {
        drawBitmap(pp, bmp, origin + QPoint(u, u), palette.color(QPalette::Light));
        drawBitmap(pp, bmp, origin, palette.color(QPalette::Dark));
    }
}

} // namespace Beveled
