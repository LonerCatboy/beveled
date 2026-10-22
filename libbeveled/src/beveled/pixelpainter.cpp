// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "pixelpainter.h"

#include <QPainter>

#include <cmath>

namespace Beveled {

PixelPainter::PixelPainter(QPainter *painter)
    : m_painter(painter)
{
    m_painter->save();

    // deviceTransform() = worldTransform() * D, where D is the part the paint device adds
    // (device pixel ratio, redirection offset). Setting the world transform to D^-1 makes the
    // whole chain an identity: coordinates become device pixels.
    // Mirrored transforms (negative scale) are fine: some surfaces, such as OpenGL framebuffers
    // used by previews, are painted upside down.
    const QTransform device = m_painter->deviceTransform();
    m_aligned = device.type() <= QTransform::TxScale && device.m11() != 0 && device.m22() != 0;

    if (m_aligned) {
        const QTransform devicePart = m_painter->worldTransform().inverted() * device;
        m_toDevice = device;
        m_painter->setWorldTransform(devicePart.inverted());
        // Fractional ratios round down for now: 1.25-1.75 draw 1px lines (see CLAUDE.md).
        m_unit = std::max(1, int(std::floor(std::min(std::abs(device.m11()), std::abs(device.m22())) + 1e-6)));
    }

    m_painter->setRenderHint(QPainter::Antialiasing, false);
    m_painter->setPen(Qt::NoPen);
}

PixelPainter::~PixelPainter()
{
    m_painter->restore();
}

QRect PixelPainter::toDevice(const QRectF &logical) const
{
    const QPointF topLeft = m_toDevice.map(logical.topLeft());
    const QPointF bottomRight = m_toDevice.map(logical.bottomRight());
    const int x0 = int(std::lround(topLeft.x()));
    const int y0 = int(std::lround(topLeft.y()));
    const int x1 = int(std::lround(bottomRight.x()));
    const int y1 = int(std::lround(bottomRight.y()));
    // Inverted logical rects stay invalid; a mirrored transform only swaps the edges.
    const bool flipX = m_toDevice.m11() < 0;
    const bool flipY = m_toDevice.m22() < 0;
    const int left = flipX ? x1 : x0;
    const int right = flipX ? x0 : x1;
    const int top = flipY ? y1 : y0;
    const int bottom = flipY ? y0 : y1;
    return QRect(left, top, right - left, bottom - top);
}

QPoint PixelPainter::toDevice(const QPointF &logical) const
{
    const QPointF p = m_toDevice.map(logical);
    return QPoint(int(std::lround(p.x())), int(std::lround(p.y())));
}

void PixelPainter::fillRect(const QRect &deviceRect, const QColor &color)
{
    if (deviceRect.isValid()) {
        m_painter->fillRect(deviceRect, color);
    }
}

} // namespace Beveled
