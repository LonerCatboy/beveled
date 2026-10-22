// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "style.h"

#include "beveledstyle.h"

#include <beveled/bevel.h>
#include <beveled/shades.h>

#include <QStyleFactory>
#include <QStyleOption>

#include <algorithm>

namespace Beveled {

namespace {

// The style to build on. Never fall back to QProxyStyle's default (the desktop style): when
// Beveled itself is the desktop style, that would create Beveled again, endlessly.
QStyle *createBaseStyle()
{
    for (const QString &name : {QStringLiteral("Windows"), QStringLiteral("Fusion")}) {
        if (QStyle *style = QStyleFactory::create(name)) {
            return style;
        }
    }
    return nullptr;
}

} // namespace

Style::Style()
    : QProxyStyle(createBaseStyle())
    , m_config(std::make_unique<StyleConfig>())
{
    m_config->load(); // the generated class starts with default values
}

Style::~Style() = default;

void Style::polish(QPalette &palette)
{
    QProxyStyle::polish(palette);
    applyShades(palette, m_config->shadeDepth() / 100.0);
}

int Style::pixelMetric(PixelMetric metric, const QStyleOption *option, const QWidget *widget) const
{
    switch (metric) {
    case PM_LayoutLeftMargin:
    case PM_LayoutTopMargin:
    case PM_LayoutRightMargin:
    case PM_LayoutBottomMargin:
        return m_config->layoutMargin();
    case PM_LayoutHorizontalSpacing:
    case PM_LayoutVerticalSpacing:
        return m_config->layoutSpacing();
    default:
        return QProxyStyle::pixelMetric(metric, option, widget);
    }
}

QSize Style::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    if (type == CT_PushButton) {
        if (const auto *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            // The Windows style adds 6px of margin per side plus the frame, then forces at least
            // 75x23: generous with today's larger fonts. Here: bevel + configurable padding.
            const int bevel = bevelThickness();
            int width = contentsSize.width() + 2 * (bevel + m_config->buttonPaddingX());
            int height = contentsSize.height() + 2 * (bevel + m_config->buttonPaddingY());
            // Default buttons in dialogs get the classic extra dark ring around them.
            if (button->features & QStyleOptionButton::AutoDefaultButton) {
                const int indicator = 2 * pixelMetric(PM_ButtonDefaultIndicator, option, widget);
                width += indicator;
                height += indicator;
            }
            if (!button->text.isEmpty()) {
                width = std::max(width, m_config->buttonMinWidth());
            }
            return QSize(width, height);
        }
    }
    return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
}

void Style::drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    // Two-pixel sunken frames (lists, text fields, scroll areas) are drawn by libbeveled: on the
    // device pixel grid, and with the optional hard variant.
    if (element == PE_Frame || element == PE_FrameLineEdit) {
        const auto *frame = qstyleoption_cast<const QStyleOptionFrame *>(option);
        if (frame && (frame->state & State_Sunken) && frame->lineWidth == bevelThickness()) {
            const bool hard = m_config->sunkenFrame() == StyleConfig::EnumSunkenFrame::Hard;
            drawBevel(painter, frame->rect, hard ? BevelStyle::SunkenHard : BevelStyle::Sunken, frame->palette);
            return;
        }
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

} // namespace Beveled
