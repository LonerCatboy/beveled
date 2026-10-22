// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "style.h"

#include <beveled/bevel.h>
#include <beveled/shades.h>

#include <KConfigGroup>
#include <KSharedConfig>

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

int readBounded(const KConfigGroup &group, const char *key, int fallback, int min, int max)
{
    return std::clamp(group.readEntry(key, fallback), min, max);
}

} // namespace

StyleOptions StyleOptions::load()
{
    const KSharedConfig::Ptr config = KSharedConfig::openConfig(QStringLiteral("beveledrc"));
    const KConfigGroup general = config->group(QStringLiteral("General"));
    const KConfigGroup style = config->group(QStringLiteral("Style"));
    StyleOptions options;
    options.shadeDepth = readBounded(general, "ShadeDepth", options.shadeDepth, 50, 150);
    options.buttonPaddingX = readBounded(style, "ButtonPaddingX", options.buttonPaddingX, 0, 12);
    options.buttonPaddingY = readBounded(style, "ButtonPaddingY", options.buttonPaddingY, 0, 6);
    options.buttonMinWidth = readBounded(style, "ButtonMinWidth", options.buttonMinWidth, 0, 100);
    return options;
}

Style::Style()
    : QProxyStyle(createBaseStyle())
    , m_options(StyleOptions::load())
{
}

void Style::polish(QPalette &palette)
{
    QProxyStyle::polish(palette);
    applyShades(palette, m_options.shadeDepth / 100.0);
}

QSize Style::sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const
{
    if (type == CT_PushButton) {
        if (const auto *button = qstyleoption_cast<const QStyleOptionButton *>(option)) {
            // The Windows style adds 6px of margin per side plus the frame, then forces at least
            // 75x23: generous with today's larger fonts. Here: bevel + configurable padding.
            const int bevel = bevelThickness();
            int width = contentsSize.width() + 2 * (bevel + m_options.buttonPaddingX);
            int height = contentsSize.height() + 2 * (bevel + m_options.buttonPaddingY);
            // Default buttons in dialogs get the classic extra dark ring around them.
            if (button->features & QStyleOptionButton::AutoDefaultButton) {
                const int indicator = 2 * pixelMetric(PM_ButtonDefaultIndicator, option, widget);
                width += indicator;
                height += indicator;
            }
            if (!button->text.isEmpty()) {
                width = std::max(width, m_options.buttonMinWidth);
            }
            return QSize(width, height);
        }
    }
    return QProxyStyle::sizeFromContents(type, option, contentsSize, widget);
}

} // namespace Beveled
