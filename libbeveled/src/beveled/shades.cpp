// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "shades.h"

#include <QPalette>

#include <algorithm>
#include <cmath>

namespace Beveled {

namespace {

// Ratios of the classic shadow grays to the gray level of the 212,208,200 face (qGray = 208.125).
constexpr qreal DarkRatio = 128.0 / 208.125;
constexpr qreal ShadowRatio = 64.0 / 208.125;

QColor grayAt(qreal level)
{
    const int v = std::clamp(int(std::lround(level)), 0, 255);
    return QColor(v, v, v);
}

} // namespace

Shades shadesForFace(const QColor &face, qreal depth)
{
    const qreal gray = (face.red() * 11 + face.green() * 16 + face.blue() * 5) / 32.0;
    const qreal darkRatio = 1.0 - (1.0 - DarkRatio) * depth;
    const qreal shadowRatio = 1.0 - (1.0 - ShadowRatio) * depth;

    // TODO: tune the highlight for dark faces when the dark scheme is designed.
    return Shades {
        .light = QColor(255, 255, 255),
        .midlight = face,
        .dark = grayAt(gray * darkRatio),
        .shadow = grayAt(gray * shadowRatio),
    };
}

void applyShades(QPalette &palette, qreal depth)
{
    for (const QPalette::ColorGroup group : {QPalette::Active, QPalette::Inactive, QPalette::Disabled}) {
        const Shades s = shadesForFace(palette.color(group, QPalette::Button), depth);
        palette.setColor(group, QPalette::Light, s.light);
        palette.setColor(group, QPalette::Midlight, s.midlight);
        palette.setColor(group, QPalette::Dark, s.dark);
        palette.setColor(group, QPalette::Mid, s.dark);
        palette.setColor(group, QPalette::Shadow, s.shadow);
    }
}

} // namespace Beveled
