// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KConfigWatcher>
#include <KDecoration3/Decoration>

#include <QColor>
#include <QList>
#include <QPalette>

#include <memory>

namespace Beveled {

class Button;
class DecorationConfig;

// Classic window decoration: beveled frame, gradient title bar, caption buttons.
// All sizes are logical pixels; drawing goes through libbeveled so it lands on device pixels.
class Decoration : public KDecoration3::Decoration
{
    Q_OBJECT

public:
    explicit Decoration(QObject *parent = nullptr, const QVariantList &args = QVariantList());
    ~Decoration() override;

    bool init() override;
    void paint(QPainter *painter, const QRectF &repaintArea) override;

    // The window palette with Beveled's 3D shades applied.
    QPalette palette() const;

    // User options from beveledrc, kept up to date while the decoration runs.
    const DecorationConfig &config() const;

private:
    bool hasFrame() const;
    int frameWidth() const;
    int titleHeight() const;
    QRect titleRect() const;

    void updateLayout();
    void recreateButtons();
    void layoutButtons();
    void readTitleBlend();

    QList<Button *> m_leftButtons;
    QList<Button *> m_rightButtons;
    std::unique_ptr<DecorationConfig> m_config;
    KConfigWatcher::Ptr m_configWatcher;
    KConfigWatcher::Ptr m_globalsWatcher;
    QColor m_activeBlend;
    QColor m_inactiveBlend;
};

} // namespace Beveled
