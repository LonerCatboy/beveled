// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "bevel.h"

#include "pixelpainter.h"

#include <QPalette>

namespace Beveled {

namespace {

// Draws one ring of thickness t inside deviceRect. Top/left stop short of the bottom/right
// edges, which run the full length and therefore own the corners.
void drawRing(PixelPainter &pp, const QRect &r, const Ring &ring, int t)
{
    pp.fillRect(QRect(r.left(), r.top(), r.width() - t, t), ring.topLeft);
    pp.fillRect(QRect(r.left(), r.top(), t, r.height() - t), ring.topLeft);
    pp.fillRect(QRect(r.left(), r.bottom() - t + 1, r.width(), t), ring.bottomRight);
    pp.fillRect(QRect(r.right() - t + 1, r.top(), t, r.height()), ring.bottomRight);
}

} // namespace

BevelColors bevelColors(BevelStyle style, const QPalette &p)
{
    switch (style) {
    case BevelStyle::Window:
        return {{p.color(QPalette::Midlight), p.color(QPalette::Shadow)}, {p.color(QPalette::Light), p.color(QPalette::Dark)}};
    case BevelStyle::Raised:
        return {{p.color(QPalette::Light), p.color(QPalette::Shadow)}, {p.color(QPalette::Midlight), p.color(QPalette::Dark)}};
    case BevelStyle::Pressed:
        return {{p.color(QPalette::Shadow), p.color(QPalette::Shadow)}, {p.color(QPalette::Dark), p.color(QPalette::Dark)}};
    case BevelStyle::Sunken:
        return {{p.color(QPalette::Dark), p.color(QPalette::Light)}, {p.color(QPalette::Shadow), p.color(QPalette::Midlight)}};
    case BevelStyle::Etched:
        return {{p.color(QPalette::Dark), p.color(QPalette::Light)}, {p.color(QPalette::Light), p.color(QPalette::Dark)}};
    }
    return {};
}

void drawBevel(QPainter *painter, const QRect &logicalRect, const BevelColors &colors, int ringWidth)
{
    PixelPainter pp(painter);
    const int t = ringWidth * pp.unit();
    const QRect outer = pp.toDevice(logicalRect);
    drawRing(pp, outer, colors.outer, t);
    drawRing(pp, outer.adjusted(t, t, -t, -t), colors.inner, t);
}

void drawBevel(QPainter *painter, const QRect &logicalRect, BevelStyle style, const QPalette &palette, int ringWidth)
{
    drawBevel(painter, logicalRect, bevelColors(style, palette), ringWidth);
}

void drawEtchedLine(QPainter *painter, const QRect &logicalRect, Qt::Orientation orientation, const QPalette &palette, int ringWidth)
{
    PixelPainter pp(painter);
    const int t = ringWidth * pp.unit();
    const QRect r = pp.toDevice(logicalRect);
    if (orientation == Qt::Horizontal) {
        const int y = r.top() + (r.height() - 2 * t) / 2;
        pp.fillRect(QRect(r.left(), y, r.width(), t), palette.color(QPalette::Dark));
        pp.fillRect(QRect(r.left(), y + t, r.width(), t), palette.color(QPalette::Light));
    } else {
        const int x = r.left() + (r.width() - 2 * t) / 2;
        pp.fillRect(QRect(x, r.top(), t, r.height()), palette.color(QPalette::Dark));
        pp.fillRect(QRect(x + t, r.top(), t, r.height()), palette.color(QPalette::Light));
    }
}

} // namespace Beveled
