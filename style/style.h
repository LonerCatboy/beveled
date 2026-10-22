// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QProxyStyle>

#include <memory>

namespace Beveled {

class StyleConfig;

// Qt's built-in Windows style, with Beveled's changes on top. QProxyStyle is the public Qt API
// for this: everything not overridden here is drawn and measured by the Windows style, which Qt
// maintains. If Qt ever stops shipping it, Fusion is used instead.
//
// Options come from ~/.config/beveledrc and are read when the style is created, i.e. when an
// application starts.
class Style : public QProxyStyle
{
    Q_OBJECT

public:
    Style();
    ~Style() override;

    // Replaces KDE's computed 3D colors with Beveled's own, as the window decoration does.
    void polish(QPalette &palette) override;
    using QProxyStyle::polish;

    int pixelMetric(PixelMetric metric, const QStyleOption *option = nullptr, const QWidget *widget = nullptr) const override;
    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const override;
    void drawPrimitive(PrimitiveElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

private:
    std::unique_ptr<StyleConfig> m_config;
};

} // namespace Beveled
