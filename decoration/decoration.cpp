// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "decoration.h"

#include "beveledconfig.h"
#include "button.h"

#include <beveled/bevel.h>
#include <beveled/pixelpainter.h>
#include <beveled/shades.h>
#include <beveled/shadow.h>

#include <KConfigGroup>
#include <KDecoration3/DecoratedWindow>
#include <KDecoration3/DecorationSettings>
#include <KDecoration3/DecorationShadow>
#include <KSharedConfig>

#include <QLinearGradient>
#include <QPainter>

#include <algorithm>
#include <cmath>
#include <memory>

namespace Beveled {

namespace {

// Classic metrics at 100%, in logical pixels.
constexpr int BevelWidth = bevelThickness(); // the two rings of the window border
constexpr int MinTitleHeight = 18;
constexpr int TitleSeparator = 1; // face-colored row between the title bar and the client
constexpr int ButtonMargin = 2; // around caption buttons inside the title bar
constexpr int CloseGap = 2; // extra space on the left of the close button
constexpr int IconMargin = 2;
constexpr int TextGap = 4; // between the icon (or left buttons) and the caption text
constexpr int ResizeOnlyBorder = 4; // invisible resize area when there is no frame

// Face-colored padding inside the bevel, following KDE's standard "Border size" setting.
int padding(KDecoration3::BorderSize size)
{
    switch (size) {
    case KDecoration3::BorderSize::None:
    case KDecoration3::BorderSize::NoSides:
        return 0;
    case KDecoration3::BorderSize::Tiny:
        return 1;
    case KDecoration3::BorderSize::Normal:
        return 2;
    case KDecoration3::BorderSize::Large:
        return 4;
    case KDecoration3::BorderSize::VeryLarge:
        return 6;
    case KDecoration3::BorderSize::Huge:
        return 8;
    case KDecoration3::BorderSize::VeryHuge:
        return 12;
    case KDecoration3::BorderSize::Oversized:
        return 16;
    }
    return 2;
}

// Shadow opacity (0-255), applied to the palette's Shadow color.
constexpr int SoftShadowAlpha = 140;
constexpr int SolidShadowAlpha = 110;

// All windows share one shadow texture; it is rebuilt only when its parameters change and freed
// when no decoration uses it any more.
std::shared_ptr<KDecoration3::DecorationShadow> sharedShadow(DecorationConfig::EnumShadowStyle::type style, int distance, const QColor &color)
{
    struct Key
    {
        DecorationConfig::EnumShadowStyle::type style;
        int distance;
        QRgb color;
        bool operator==(const Key &) const = default;
    };
    static Key cachedKey {};
    static std::weak_ptr<KDecoration3::DecorationShadow> cached;

    const Key key {style, distance, color.rgba()};
    if (auto shadow = cached.lock(); shadow && cachedKey == key) {
        return shadow;
    }

    const ShadowTexture texture = style == DecorationConfig::EnumShadowStyle::Solid ? solidShadow(distance, color) : softShadow(distance, color);
    auto shadow = std::make_shared<KDecoration3::DecorationShadow>();
    shadow->setShadow(texture.image);
    shadow->setInnerShadowRect(texture.innerRect);
    shadow->setPadding(texture.padding);
    cached = shadow;
    cachedKey = key;
    return shadow;
}

} // namespace

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
    , m_config(std::make_unique<DecorationConfig>())
{
}

Decoration::~Decoration() = default;

bool Decoration::init()
{
    KDecoration3::DecoratedWindow *w = window();
    const auto s = settings();

    m_config->load(); // the generated class starts with default values
    m_configWatcher = KConfigWatcher::create(m_config->sharedConfig());
    connect(m_configWatcher.data(), &KConfigWatcher::configChanged, this, [this] {
        m_config->load();
        updateShadow();
        updateLayout();
    });

    readTitleBlend();
    m_globalsWatcher = KConfigWatcher::create(KSharedConfig::openConfig(QStringLiteral("kdeglobals")));
    connect(m_globalsWatcher.data(), &KConfigWatcher::configChanged, this, [this](const KConfigGroup &group) {
        if (group.name() == QLatin1String("WM")) {
            readTitleBlend();
            update();
        }
    });

    connect(s.get(), &KDecoration3::DecorationSettings::borderSizeChanged, this, &Decoration::updateLayout);
    connect(s.get(), &KDecoration3::DecorationSettings::fontChanged, this, &Decoration::updateLayout);
    connect(s.get(), &KDecoration3::DecorationSettings::reconfigured, this, &Decoration::updateLayout);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsLeftChanged, this, &Decoration::recreateButtons);
    connect(s.get(), &KDecoration3::DecorationSettings::decorationButtonsRightChanged, this, &Decoration::recreateButtons);

    connect(w, &KDecoration3::DecoratedWindow::maximizedChanged, this, &Decoration::updateLayout);
    connect(w, &KDecoration3::DecoratedWindow::sizeChanged, this, &Decoration::updateLayout);
    connect(w, &KDecoration3::DecoratedWindow::activeChanged, this, [this] { update(); });
    connect(w, &KDecoration3::DecoratedWindow::captionChanged, this, [this] { update(); });
    connect(w, &KDecoration3::DecoratedWindow::paletteChanged, this, [this] {
        updateShadow();
        update();
    });
    connect(w, &KDecoration3::DecoratedWindow::iconChanged, this, [this] { update(); });

    setOpaque(true);
    updateShadow();
    recreateButtons();
    return true;
}

QPalette Decoration::palette() const
{
    QPalette p = window()->palette();
    applyShades(p, m_config->shadeDepth() / 100.0);
    return p;
}

const DecorationConfig &Decoration::config() const
{
    return *m_config;
}

bool Decoration::hasFrame() const
{
    const KDecoration3::BorderSize size = settings()->borderSize();
    return !window()->isMaximized() && size != KDecoration3::BorderSize::None && size != KDecoration3::BorderSize::NoSides;
}

int Decoration::frameWidth() const
{
    return hasFrame() ? BevelWidth + padding(settings()->borderSize()) : 0;
}

int Decoration::titleHeight() const
{
    // Button padding grows the caption buttons, and the title bar with them.
    return std::max(MinTitleHeight, int(std::ceil(settings()->fontMetrics().height())) + 2) + 2 * m_config->buttonPadding();
}

QRect Decoration::titleRect() const
{
    const int f = frameWidth();
    return QRect(f, f, int(size().width()) - 2 * f, titleHeight());
}

void Decoration::updateLayout()
{
    const int f = frameWidth();
    setBorders(QMarginsF(f, f + titleHeight() + TitleSeparator, f, f));

    if (hasFrame() || window()->isMaximized()) {
        setResizeOnlyBorders(QMarginsF());
    } else {
        setResizeOnlyBorders(QMarginsF(ResizeOnlyBorder, 0, ResizeOnlyBorder, ResizeOnlyBorder));
    }

    setTitleBar(titleRect());
    layoutButtons();
    update();
}

void Decoration::recreateButtons()
{
    qDeleteAll(m_leftButtons);
    qDeleteAll(m_rightButtons);
    m_leftButtons.clear();
    m_rightButtons.clear();

    const auto create = [this](const QList<KDecoration3::DecorationButtonType> &types, QList<Button *> &list) {
        for (const KDecoration3::DecorationButtonType type : types) {
            if (Button *button = Button::create(type, this)) {
                list.append(button);
            }
        }
    };
    create(settings()->decorationButtonsLeft(), m_leftButtons);
    create(settings()->decorationButtonsRight(), m_rightButtons);

    updateLayout();
}

// Places the buttons as in the classic layout: caption buttons are adjacent, except for a small
// gap on the left of the close button; the window icon sits at the left edge of the title bar.
void Decoration::layoutButtons()
{
    const QRect title = titleRect();
    const int h = title.height() - 2 * ButtonMargin;
    const QSize captionSize(h + 2, h);
    const int iconSize = title.height() - 2;

    int x = title.left() + IconMargin;
    for (int i = 0; i < m_leftButtons.size(); ++i) {
        Button *button = m_leftButtons.at(i);
        if (button->isIcon()) {
            button->setGeometry(QRectF(x, title.top() + 1, iconSize, iconSize));
            x += iconSize + IconMargin;
        } else {
            if (i > 0 && button->type() == KDecoration3::DecorationButtonType::Close) {
                x += CloseGap;
            }
            button->setGeometry(QRectF(QPointF(x, title.top() + ButtonMargin), captionSize));
            x += captionSize.width();
        }
    }

    x = title.right() + 1 - ButtonMargin;
    for (int i = m_rightButtons.size() - 1; i >= 0; --i) {
        Button *button = m_rightButtons.at(i);
        const bool beforeClose = i + 1 < m_rightButtons.size() && m_rightButtons.at(i + 1)->type() == KDecoration3::DecorationButtonType::Close;
        if (beforeClose) {
            x -= CloseGap;
        }
        if (button->isIcon()) {
            x -= iconSize;
            button->setGeometry(QRectF(x, title.top() + 1, iconSize, iconSize));
            x -= IconMargin;
        } else {
            x -= captionSize.width();
            button->setGeometry(QRectF(QPointF(x, title.top() + ButtonMargin), captionSize));
        }
    }
}

void Decoration::readTitleBlend()
{
    const KConfigGroup wm = KSharedConfig::openConfig(QStringLiteral("kdeglobals"))->group(QStringLiteral("WM"));
    m_activeBlend = wm.readEntry("activeBlend", QColor());
    m_inactiveBlend = wm.readEntry("inactiveBlend", QColor());
}

void Decoration::updateShadow()
{
    const auto style = DecorationConfig::EnumShadowStyle::type(m_config->shadowStyle());
    if (style == DecorationConfig::EnumShadowStyle::None) {
        setShadow({});
        return;
    }
    QColor color = palette().color(QPalette::Shadow);
    color.setAlpha(style == DecorationConfig::EnumShadowStyle::Solid ? SolidShadowAlpha : SoftShadowAlpha);
    setShadow(sharedShadow(style, m_config->shadowDistance(), color));
}

void Decoration::paint(QPainter *painter, const QRectF &repaintArea)
{
    KDecoration3::DecoratedWindow *w = window();
    const QPalette pal = palette();
    const QRect full = rect().toAlignedRect();
    const QRect client = full.marginsRemoved(borders().toMargins());

    // Frame, title separator and everything else outside the client area.
    const QColor face = pal.color(QPalette::Button);
    fillRect(painter, QRect(full.left(), full.top(), full.width(), client.top() - full.top()), face);
    fillRect(painter, QRect(full.left(), client.top(), client.left() - full.left(), client.height()), face);
    fillRect(painter, QRect(client.right() + 1, client.top(), full.right() - client.right(), client.height()), face);
    fillRect(painter, QRect(full.left(), client.bottom() + 1, full.width(), full.bottom() - client.bottom()), face);
    if (hasFrame()) {
        const bool hard = m_config->frameBevel() == DecorationConfig::EnumFrameBevel::Hard;
        drawBevel(painter, full, hard ? BevelStyle::RaisedHard : BevelStyle::Window, pal);
    }

    // Title bar: solid, or a horizontal gradient from the title color to the blend color.
    const QRect title = titleRect();
    const KDecoration3::ColorGroup group = w->isActive() ? KDecoration3::ColorGroup::Active : KDecoration3::ColorGroup::Inactive;
    const QColor background = w->color(group, KDecoration3::ColorRole::TitleBar);
    const QColor blend = w->isActive() ? m_activeBlend : m_inactiveBlend;
    if (m_config->titleBarStyle() == DecorationConfig::EnumTitleBarStyle::Solid || !blend.isValid()) {
        fillRect(painter, title, background);
    } else {
        PixelPainter pp(painter);
        const QRect device = pp.toDevice(title);
        QLinearGradient gradient(device.topLeft(), device.topRight());
        gradient.setColorAt(0, background);
        gradient.setColorAt(1, blend);
        painter->fillRect(device, gradient);
    }

    // Caption text between the left and right buttons.
    int textLeft = title.left() + TextGap;
    for (const Button *button : std::as_const(m_leftButtons)) {
        textLeft = std::max(textLeft, int(button->geometry().right()) + TextGap);
    }
    int textRight = title.right() - ButtonMargin;
    for (const Button *button : std::as_const(m_rightButtons)) {
        textRight = std::min(textRight, int(button->geometry().left()) - ButtonMargin);
    }
    if (textRight > textLeft) {
        const QRect textRect(textLeft, title.top(), textRight - textLeft, title.height());
        painter->save();
        painter->setFont(settings()->font());
        painter->setPen(w->color(group, KDecoration3::ColorRole::Foreground));
        const QString caption = painter->fontMetrics().elidedText(w->caption(), Qt::ElideRight, textRect.width());
        painter->drawText(textRect, Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine, caption);
        painter->restore();
    }

    for (Button *button : std::as_const(m_leftButtons)) {
        button->paint(painter, repaintArea);
    }
    for (Button *button : std::as_const(m_rightButtons)) {
        button->paint(painter, repaintArea);
    }
}

} // namespace Beveled
