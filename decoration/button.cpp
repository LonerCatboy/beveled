// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "button.h"

#include "decoration.h"

#include <beveled/bevel.h>
#include <beveled/glyphs.h>

#include <KDecoration3/DecoratedWindow>

#include <QIcon>
#include <QPainter>

namespace Beveled {

using KDecoration3::DecorationButtonType;

Button::Button(DecorationButtonType type, Decoration *decoration)
    : KDecoration3::DecorationButton(type, decoration, decoration)
{
}

Button *Button::create(DecorationButtonType type, Decoration *decoration)
{
    switch (type) {
    case DecorationButtonType::Menu:
    case DecorationButtonType::Minimize:
    case DecorationButtonType::Maximize:
    case DecorationButtonType::Close:
        return new Button(type, decoration);
    default:
        return nullptr;
    }
}

bool Button::isIcon() const
{
    return type() == DecorationButtonType::Menu;
}

void Button::paint(QPainter *painter, const QRectF &repaintArea)
{
    Q_UNUSED(repaintArea)
    if (!isVisible()) {
        return;
    }

    const auto deco = static_cast<Decoration *>(decoration());
    const QRect r = geometry().toAlignedRect();

    if (isIcon()) {
        deco->window()->icon().paint(painter, r);
        return;
    }

    const QPalette pal = deco->palette();
    const bool down = isPressed();
    fillRect(painter, r, pal.color(QPalette::Button));
    drawBevel(painter, r, down ? BevelStyle::Pushed : BevelStyle::Raised, pal);

    Glyph glyph = Glyph::Close;
    if (type() == DecorationButtonType::Minimize) {
        glyph = Glyph::Minimize;
    } else if (type() == DecorationButtonType::Maximize) {
        glyph = deco->window()->isMaximized() ? Glyph::Restore : Glyph::Maximize;
    }
    drawCaptionGlyph(painter, r, glyph, pal, isEnabled(), down);
}

} // namespace Beveled
