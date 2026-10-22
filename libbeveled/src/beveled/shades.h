// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QColor>

class QPalette;

namespace Beveled {

// The four classic 3D colors around a button face.
struct Shades
{
    QColor light; // ButtonHighlight
    QColor midlight; // 3DLight
    QColor dark; // ButtonShadow
    QColor shadow; // 3DDkShadow
};

// Derives the 3D colors from the button face. KDE computes QPalette::Light..Shadow with its own
// contrast formula, which cannot reproduce the classic values, so Beveled computes them itself.
//
// With depth = 1.0 a face of 212,208,200 gives exactly 255,255,255 / 212,208,200 /
// 128,128,128 / 64,64,64. depth scales how far the shadows move away from the face
// (0 = flat, higher = deeper); it is meant to be exposed as a bounded user option.
Shades shadesForFace(const QColor &face, qreal depth = 1.0);

// Replaces Light, Midlight, Mid, Dark and Shadow in every color group with shadesForFace()
// of that group's Button color. Mid is set equal to Dark: the classic look has no separate Mid.
void applyShades(QPalette &palette, qreal depth = 1.0);

} // namespace Beveled
