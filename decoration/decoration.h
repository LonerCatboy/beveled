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
    int fontHeight() const;
    int buttonHeight() const;
    int titleHeight() const;
    QRect frameRect() const;
    QRect titleRect() const;

    void updateLayout();
    bool translucent() const;
    void updateTranslucency();
    void recreateButtons();
    void layoutButtons();
    static QList<Button *> visibleButtons(const QList<Button *> &buttons);
    struct TitleColors
    {
        QColor background;
        QColor blend; // gradient end
        QColor foreground;
    };
    TitleColors titleColors(bool active) const;

    void readTitleBlend();
    void updateShadow();

    QList<Button *> m_leftButtons;
    QList<Button *> m_rightButtons;
    std::unique_ptr<DecorationConfig> m_config;
    KConfigWatcher::Ptr m_configWatcher;
    KConfigWatcher::Ptr m_globalsWatcher;
    QColor m_activeBlend;
    QColor m_inactiveBlend;
};

} // namespace Beveled
