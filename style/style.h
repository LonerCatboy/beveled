// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <QProxyStyle>

namespace Beveled {

// Bounded user options read from ~/.config/beveledrc, in logical pixels or percent.
struct StyleOptions
{
    int shadeDepth = 100; // [General] ShadeDepth, 50-150, shared with the decoration
    int buttonPaddingX = 6; // [Style] ButtonPaddingX, 0-12: between a button's bevel and its label
    int buttonPaddingY = 1; // [Style] ButtonPaddingY, 0-6
    int buttonMinWidth = 64; // [Style] ButtonMinWidth, 0-100: for buttons with text

    static StyleOptions load();
};

// Qt's built-in Windows style, with Beveled's changes on top. QProxyStyle is the public Qt API
// for this: everything not overridden here is drawn and measured by the Windows style, which Qt
// maintains. If Qt ever stops shipping it, Fusion is used instead.
class Style : public QProxyStyle
{
    Q_OBJECT

public:
    Style();

    // Replaces KDE's computed 3D colors with Beveled's own, as the window decoration does.
    void polish(QPalette &palette) override;
    using QProxyStyle::polish;

    QSize sizeFromContents(ContentsType type, const QStyleOption *option, const QSize &contentsSize, const QWidget *widget) const override;

private:
    StyleOptions m_options;
};

} // namespace Beveled
