// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "style.h"

#include "beveledstyle.h"

#include <beveled/bevel.h>
#include <beveled/shades.h>

#include <beveled/pixelpainter.h>

#include <KConfigGroup>

#include <QApplication>
#include <QStyleFactory>
#include <QStyleOption>
#include <QTabBar>

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

    // Settings pages write with KConfig::Notify, so every running application hears about it.
    m_watcher = KConfigWatcher::create(m_config->sharedConfig());
    connect(m_watcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group) {
        if (group.name() == QLatin1String("Style") || group.name() == QLatin1String("General")) {
            reparseConfiguration();
        }
    });
}

void Style::reparseConfiguration()
{
    m_config->load();
    if (QApplication::style() != this || m_restylePending) {
        return;
    }
    // Sizes and the polished palette are computed when a style is set; setting the same object
    // again is a no-op, so a fresh instance takes over. Queued: this object is deleted by Qt
    // when it is replaced, which must not happen while one of its slots is running.
    m_restylePending = true;
    QMetaObject::invokeMethod(qApp, [] { QApplication::setStyle(QStringLiteral("Beveled")); }, Qt::QueuedConnection);
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
    // Sunken frames (lists, scroll areas, text fields) are drawn by libbeveled: on the device
    // pixel grid, and with the optional hard variant. Like the Windows style, PE_Frame is always
    // a two-pixel panel whatever lineWidth says (QFrame's default is 1, which most applications
    // keep); text fields have no frame when lineWidth is 0.
    if (element == PE_Frame || element == PE_FrameLineEdit) {
        const auto *frame = qstyleoption_cast<const QStyleOptionFrame *>(option);
        const bool hasFrame = frame && (element == PE_Frame || frame->lineWidth > 0);
        if (hasFrame && (frame->state & State_Sunken)) {
            const bool hard = m_config->sunkenFrame() == StyleConfig::EnumSunkenFrame::Hard;
            drawBevel(painter, frame->rect, hard ? BevelStyle::SunkenHard : BevelStyle::Sunken, frame->palette);
            return;
        }
    }
    QProxyStyle::drawPrimitive(element, option, painter, widget);
}

void Style::drawControl(ControlElement element, const QStyleOption *option, QPainter *painter, const QWidget *widget) const
{
    if (element == CE_TabBarTabShape && m_config->tabShape() == StyleConfig::EnumTabShape::Square) {
        if (const auto *tab = qstyleoption_cast<const QStyleOptionTab *>(option); tab && drawSquareTab(tab, painter, widget)) {
            return;
        }
    }
    QProxyStyle::drawControl(element, option, painter, widget);
}

// Same geometry as the Windows style's tabs (unselected tabs sit 2px lower and leave the edge
// shared with a selected neighbor to it; the selected tab opens into its page), with square
// corners: Light/Midlight on the leading sides, Shadow/Dark on the trailing ones.
bool Style::drawSquareTab(const QStyleOptionTab *tab, QPainter *painter, const QWidget *widget) const
{
    const bool north = tab->shape == QTabBar::RoundedNorth || tab->shape == QTabBar::TriangularNorth;
    const bool south = tab->shape == QTabBar::RoundedSouth || tab->shape == QTabBar::TriangularSouth;
    if (!north && !south) {
        return false;
    }

    const QPalette &pal = tab->palette;
    const bool selected = tab->state & State_Selected;
    const bool onlyOne = tab->position == QStyleOptionTab::OnlyOneTab;
    const bool rtl = tab->direction == Qt::RightToLeft;
    const bool first = onlyOne || tab->position == (rtl ? QStyleOptionTab::End : QStyleOptionTab::Beginning);
    const bool last = onlyOne || tab->position == (rtl ? QStyleOptionTab::Beginning : QStyleOptionTab::End);
    const bool leftSelected = tab->selectedPosition == (rtl ? QStyleOptionTab::NextIsSelected : QStyleOptionTab::PreviousIsSelected);
    const bool rightSelected = tab->selectedPosition == (rtl ? QStyleOptionTab::PreviousIsSelected : QStyleOptionTab::NextIsSelected);

    QRect r = tab->rect;
    if (!selected) {
        const int overlap = pixelMetric(PM_TabBarBaseOverlap, tab, widget);
        if (north) {
            r.setTop(r.top() + 2);
        } else {
            r.setBottom(r.bottom() - 2);
        }
        r.setLeft(r.left() + (first ? overlap / 2 : 0));
        r.setRight(r.right() - (last ? overlap / 2 : 0));
    }

    PixelPainter pp(painter);
    const QRect d = pp.toDevice(r);
    const int u = pp.unit();
    const auto hLine = [&](int x0, int x1, int y, QPalette::ColorRole role) { pp.fillRect(QRect(x0, y, x1 - x0 + 1, u), pal.color(role)); };
    const auto vLine = [&](int x, int y0, int y1, QPalette::ColorRole role) { pp.fillRect(QRect(x, y0, u, y1 - y0 + 1), pal.color(role)); };
    const int left = d.left(), right = d.right() - u + 1; // x of the outer columns
    const int top = d.top(), bottom = d.bottom() - u + 1; // y of the outer rows
    const bool drawLeft = first || selected || !leftSelected;
    const bool drawRight = last || selected || !rightSelected;

    pp.fillRect(d, pal.color(QPalette::Window));
    // Two rings, as in libbeveled: Light/Midlight on top and left, Shadow/Dark on the right and
    // bottom, the bottom/right colors owning the corners. The side facing the page stays open.
    if (north) {
        hLine(left, right - u, top, QPalette::Light);
        hLine(left + u, right - 2 * u, top + u, QPalette::Midlight);
        if (drawLeft) {
            vLine(left, top, d.bottom(), QPalette::Light);
            vLine(left + u, top + u, d.bottom(), QPalette::Midlight);
        }
        if (drawRight) {
            vLine(right, top, d.bottom(), QPalette::Shadow);
            vLine(right - u, top + u, d.bottom(), QPalette::Dark);
        }
    } else {
        hLine(left, d.right(), bottom, QPalette::Shadow);
        hLine(left + u, right - u, bottom - u, QPalette::Dark);
        if (drawLeft) {
            vLine(left, top, bottom - u, QPalette::Light);
            vLine(left + u, top, bottom - 2 * u, QPalette::Midlight);
        }
        if (drawRight) {
            vLine(right, top, bottom, QPalette::Shadow);
            vLine(right - u, top, bottom - u, QPalette::Dark);
        }
    }
    return true;
}

} // namespace Beveled
