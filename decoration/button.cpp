// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "button.h"

#include "beveledconfig.h"
#include "decoration.h"

#include <beveled/bevel.h>
#include <beveled/glyphs.h>

#include <KDecoration3/DecoratedWindow>

#include <QIcon>
#include <QPainter>

namespace Beveled {

using KDecoration3::DecorationButtonType;

namespace {

constexpr qreal HoverTint = 0.2; // share of the selection color in a highlighted button face

QColor mix(const QColor &a, const QColor &b, qreal t)
{
    return QColor::fromRgbF(a.redF() + (b.redF() - a.redF()) * t, a.greenF() + (b.greenF() - a.greenF()) * t, a.blueF() + (b.blueF() - a.blueF()) * t);
}

} // namespace

Button::Button(DecorationButtonType type, Decoration *decoration)
    : KDecoration3::DecorationButton(type, decoration, decoration)
{
    connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, [this] { update(); });
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
    const auto hover = deco->config().buttonHover();
    const bool hovered = isHovered() && !down;

    QColor face = pal.color(QPalette::Button);
    if (hovered && hover == DecorationConfig::EnumButtonHover::Highlight) {
        // A light tint of the selection color, like the hover of KDE's default buttons.
        face = mix(face, pal.color(QPalette::Highlight), HoverTint);
    }
    fillRect(painter, r, face);
    const bool hard = deco->config().buttonBevel() == DecorationConfig::EnumButtonBevel::Hard;
    if (hard) {
        drawBevel(painter, r, down ? BevelStyle::PushedHard : BevelStyle::RaisedHard, pal);
    } else {
        drawBevel(painter, r, down ? BevelStyle::Pushed : BevelStyle::Raised, pal);
    }

    Glyph glyph = Glyph::Close;
    if (type() == DecorationButtonType::Minimize) {
        glyph = Glyph::Minimize;
    } else if (type() == DecorationButtonType::Maximize) {
        glyph = deco->window()->isMaximized() ? Glyph::Restore : Glyph::Maximize;
    }
    GlyphOffset offset = GlyphOffset::None;
    if (down) {
        offset = GlyphOffset::Pressed;
    } else if (hovered && hover == DecorationConfig::EnumButtonHover::Nudge) {
        offset = GlyphOffset::Nudged;
    }
    drawCaptionGlyph(painter, r, glyph, pal, isEnabled(), offset);
}

} // namespace Beveled
