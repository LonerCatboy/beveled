// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QColor>
#include <QRect>
#include <QTransform>

class QPainter;

namespace Beveled {

// Draws in physical (device) pixels on top of an existing QPainter.
//
// While a PixelPainter is alive, the wrapped painter works in device pixels: coordinates passed
// to fillRect() are physical pixels, antialiasing is off and no pen is set. The previous painter
// state is restored on destruction.
//
// Logical rectangles are converted with toDevice(), which rounds each *edge* to the pixel grid
// (not the size), so adjacent logical rectangles always meet without gaps or overlaps.
class PixelPainter
{
public:
    explicit PixelPainter(QPainter *painter);
    ~PixelPainter();

    PixelPainter(const PixelPainter &) = delete;
    PixelPainter &operator=(const PixelPainter &) = delete;

    // Device pixels per logical pixel, as an integer >= 1. This is the thickness of one
    // "classic" pixel: 1 at 100%, 2 at 200%.
    int unit() const { return m_unit; }

    // False when the painter is rotated or sheared; drawing then happens in logical
    // coordinates and is not guaranteed to be crisp.
    bool isPixelAligned() const { return m_aligned; }

    QRect toDevice(const QRectF &logical) const;
    QPoint toDevice(const QPointF &logical) const;

    void fillRect(const QRect &deviceRect, const QColor &color);

    QPainter *painter() const { return m_painter; }

private:
    QPainter *m_painter;
    QTransform m_toDevice;
    int m_unit = 1;
    bool m_aligned = false;
};

} // namespace Beveled
