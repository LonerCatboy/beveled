// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "button.h"

#include "beveledconfig.h"
#include "decoration.h"

#include <beveled/bevel.h>
#include <beveled/glyphs.h>
#include <beveled/shades.h>

#include <KDecoration3/DecoratedWindow>

#include <QIcon>
#include <QPainter>

namespace Beveled {

using KDecoration3::DecorationButtonType;

namespace {

constexpr qreal HoverLightening = 0.5; // share of the Light color in a highlighted button face

} // namespace

Button::Button(DecorationButtonType type, Decoration *decoration)
    : KDecoration3::DecorationButton(type, decoration, decoration)
{
    connect(this, &KDecoration3::DecorationButton::hoveredChanged, this, &Button::updateWithNudge);
    connect(this, &KDecoration3::DecorationButton::pressedChanged, this, &Button::updateWithNudge);
}

Button *Button::create(DecorationButtonType type, Decoration *decoration)
{
    switch (type) {
    case DecorationButtonType::Menu:
    case DecorationButtonType::Minimize:
    case DecorationButtonType::Maximize:
    case DecorationButtonType::Close:
    case DecorationButtonType::ContextHelp:
    case DecorationButtonType::KeepAbove:
    case DecorationButtonType::KeepBelow:
    case DecorationButtonType::OnAllDesktops:
    case DecorationButtonType::Shade:
    case DecorationButtonType::Spacer:
        return new Button(type, decoration);
    default:
        return nullptr;
    }
}

void Button::updateWithNudge()
{
    update(geometry().adjusted(0, 0, 0, 1));
}

bool Button::isIcon() const
{
    return type() == DecorationButtonType::Menu;
}

void Button::paint(QPainter *painter, const QRectF &repaintArea)
{
    Q_UNUSED(repaintArea)
    if (!isVisible() || type() == DecorationButtonType::Spacer) {
        return;
    }

    const auto deco = static_cast<Decoration *>(decoration());
    const QRect r = geometry().toAlignedRect();

    if (isIcon()) {
        deco->window()->icon().paint(painter, r);
        return;
    }

    const QPalette pal = deco->palette();
    // Toggle buttons (keep above, on all desktops, ...) stay pushed while their state is on.
    // Maximize is checkable too, but shows its state with the restore glyph instead.
    const bool toggled = isCheckable() && isChecked() && type() != DecorationButtonType::Maximize;
    const bool down = isPressed() || toggled;
    const auto hover = deco->config().buttonHover();
    const bool hovered = isHovered() && !down;

    // "Slightly pressed" hover: the whole button sinks by one device pixel and stays there while
    // pressed. There is always room below: the title bar is followed by a separator row.
    const bool nudged = hover == DecorationConfig::EnumButtonHover::Nudge && (isHovered() || down);
    painter->save();
    if (nudged) {
        painter->translate(0, 1.0 / painter->deviceTransform().m22());
    }

    QColor face = pal.color(QPalette::Button);
    if (hovered && hover == DecorationConfig::EnumButtonHover::Highlight) {
        // A lighter face, halfway to the bevel highlight; no hue change.
        face = mix(face, pal.color(QPalette::Light), HoverLightening);
    }
    fillRect(painter, r, face);
    const bool hard = deco->config().buttonBevel() == DecorationConfig::EnumButtonBevel::Hard;
    if (hard) {
        drawBevel(painter, r, down ? BevelStyle::PushedHard : BevelStyle::RaisedHard, pal);
    } else {
        drawBevel(painter, r, down ? BevelStyle::Pushed : BevelStyle::Raised, pal);
    }

    Glyph glyph = Glyph::Close;
    switch (type()) {
    case DecorationButtonType::Minimize:
        glyph = Glyph::Minimize;
        break;
    case DecorationButtonType::Maximize:
        glyph = deco->window()->isMaximized() ? Glyph::Restore : Glyph::Maximize;
        break;
    case DecorationButtonType::ContextHelp:
        glyph = Glyph::Help;
        break;
    case DecorationButtonType::KeepAbove:
        glyph = Glyph::KeepAbove;
        break;
    case DecorationButtonType::KeepBelow:
        glyph = Glyph::KeepBelow;
        break;
    case DecorationButtonType::OnAllDesktops:
        glyph = Glyph::OnAllDesktops;
        break;
    case DecorationButtonType::Shade:
        glyph = Glyph::Shade;
        break;
    default:
        break;
    }
    drawCaptionGlyph(painter, r, glyph, pal, isEnabled(), down ? GlyphOffset::Pressed : GlyphOffset::None);
    painter->restore();
}

} // namespace Beveled
