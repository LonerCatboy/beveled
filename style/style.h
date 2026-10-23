// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KConfigWatcher>

#include <QProxyStyle>

#include <memory>

class QStyleOptionTab;

namespace Beveled {

class StyleConfig;

// Qt's built-in Windows style, with Beveled's changes on top. QProxyStyle is the public Qt API
// for this: everything not overridden here is drawn and measured by the Windows style, which Qt
// maintains. If Qt ever stops shipping it, Fusion is used instead.
//
// Options come from ~/.config/beveledrc. When they change, the application's style is set again,
// so running applications pick them up like a style change from System Settings.
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
    void drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget = nullptr) const override;

public Q_SLOTS:
    // Reads the options again; if this is the application's style, replaces it with a new
    // instance (after returning to the event loop) so that every widget is polished again.
    void reparseConfiguration();

private:
    // Square tab shapes for tabs on top or bottom; returns false for other shapes.
    bool drawSquareTab(const QStyleOptionTab *tab, QPainter *painter, const QWidget *widget) const;
    void drawHotTrack(QPainter *painter, const QRect &rect, const QPalette &palette) const;

    std::unique_ptr<StyleConfig> m_config;
    KConfigWatcher::Ptr m_watcher;
    bool m_restylePending = false;
};

} // namespace Beveled
