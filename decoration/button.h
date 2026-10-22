// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KDecoration3/DecorationButton>

namespace Beveled {

class Decoration;

class Button : public KDecoration3::DecorationButton
{
    Q_OBJECT

public:
    // Returns nullptr for button types Beveled does not draw yet.
    static Button *create(KDecoration3::DecorationButtonType type, Decoration *decoration);

    // The window menu button is drawn as the window icon, without a bevel.
    bool isIcon() const;

    void paint(QPainter *painter, const QRectF &repaintArea) override;

private:
    Button(KDecoration3::DecorationButtonType type, Decoration *decoration);
};

} // namespace Beveled
