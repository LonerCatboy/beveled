// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "shadow.h"

#include <QColor>
#include <QPainter>

#include <algorithm>
#include <cmath>
#include <vector>

namespace Beveled {

namespace {

// Builds a nine-patch around a square stand-in for the window. The corners (size c) must hold
// every non-uniform part of the shadow, so that stretching the 1px edges gives the same shadow
// along any window side.
struct Layout
{
    QMargins padding;
    int corner;
    int window; // side of the stand-in window: two corners and the 1px stretched center

    Layout(const QMargins &padding, int corner)
        : padding(padding)
        , corner(corner)
        , window(2 * corner + 1)
    {
    }

    QSize imageSize() const { return QSize(padding.left() + window + padding.right(), padding.top() + window + padding.bottom()); }

    QRect windowRect() const { return QRect(padding.left(), padding.top(), window, window); }

    ShadowTexture finish(QImage image) const
    {
        // Nothing under the window: translucent windows must not show their own shadow.
        QPainter painter(&image);
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.fillRect(windowRect(), Qt::transparent);
        painter.end();

        const QRect inner(padding.left() + corner, padding.top() + corner, 1, 1);
        return {std::move(image), inner, padding};
    }
};

// Coverage of the interval [a, b) by a Gaussian blur of the given sigma, sampled at x.
qreal blurredSpan(qreal x, qreal a, qreal b, qreal sigma)
{
    const qreal k = 1.0 / (std::sqrt(2.0) * sigma);
    return 0.5 * (std::erf((x - a) * k) - std::erf((x - b) * k));
}

} // namespace

ShadowTexture solidShadow(int distance, const QColor &color)
{
    const int d = std::clamp(distance, MinShadowDistance, MaxShadowDistance);
    // Only the right and bottom sides are visible; the corners must reach past the offset.
    const Layout layout(QMargins(0, 0, d, d), d + 1);

    QImage image(layout.imageSize(), QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    QPainter painter(&image);
    painter.fillRect(layout.windowRect().translated(d, d), color);
    painter.end();
    return layout.finish(std::move(image));
}

ShadowTexture softShadow(int distance, const QColor &color)
{
    const int d = std::clamp(distance, MinShadowDistance, MaxShadowDistance);
    const int radius = 8 + 4 * d; // visible extent of the blur
    const qreal sigma = radius / 3.0;
    const Layout layout(QMargins(radius, std::max(0, radius - d), radius, radius + d), radius + d);

    // A Gaussian-blurred rectangle is separable: coverage(x, y) = span(x) * span(y).
    const QRect shadowRect = layout.windowRect().translated(0, d);
    const QSize size = layout.imageSize();
    std::vector<qreal> spanX(size.width());
    std::vector<qreal> spanY(size.height());
    for (int x = 0; x < size.width(); ++x) {
        spanX[x] = blurredSpan(x + 0.5, shadowRect.left(), shadowRect.left() + shadowRect.width(), sigma);
    }
    for (int y = 0; y < size.height(); ++y) {
        spanY[y] = blurredSpan(y + 0.5, shadowRect.top(), shadowRect.top() + shadowRect.height(), sigma);
    }

    QImage image(size, QImage::Format_ARGB32_Premultiplied);
    for (int y = 0; y < size.height(); ++y) {
        auto line = reinterpret_cast<QRgb *>(image.scanLine(y));
        for (int x = 0; x < size.width(); ++x) {
            const int alpha = std::clamp(int(std::lround(color.alpha() * spanX[x] * spanY[y])), 0, 255);
            line[x] = qPremultiply(qRgba(color.red(), color.green(), color.blue(), alpha));
        }
    }
    return layout.finish(std::move(image));
}

} // namespace Beveled
