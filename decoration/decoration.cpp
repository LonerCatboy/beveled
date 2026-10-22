// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "decoration.h"

#include "beveledconfig.h"
#include "button.h"

#include <beveled/bevel.h>
#include <beveled/glyphs.h>
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
#include <map>
#include <memory>

namespace Beveled {

namespace {

// Classic metrics at 100%, in logical pixels.
constexpr int BevelWidth = bevelThickness(); // the two rings of the window border
constexpr int TitleSeparator = 1; // face-colored row between the title bar and the client
constexpr int TextMargin = 2; // between the caption text and the right buttons or edge
constexpr int ButtonGap = 2; // between caption buttons, except minimize and maximize
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

QFont::Weight titleWeight(int option)
{
    switch (option) {
    case DecorationConfig::EnumTitleWeight::Medium:
        return QFont::Medium;
    case DecorationConfig::EnumTitleWeight::SemiBold:
        return QFont::DemiBold;
    case DecorationConfig::EnumTitleWeight::Bold:
        return QFont::Bold;
    default:
        return QFont::Normal;
    }
}

// With the accent color option, the gradient fades from the accent towards the Light color.
constexpr qreal AccentBlendAmount = 0.6;
// With the inactive tint option, share of the active title colors mixed into inactive title bars.
constexpr qreal InactiveTintAmount = 0.2;

// Shadow opacity (0-255), applied to the palette's Shadow color.
constexpr int SoftShadowAlpha = 140;
constexpr int SolidShadowAlpha = 110;
// Inactive windows get a lighter shadow, so the active one stands out.
constexpr qreal InactiveShadowOpacity = 0.6;

// Windows with the same shadow parameters share one texture (typically one for active and one for
// inactive windows). Textures are built on first use and freed when no decoration uses them.
struct ShadowKey
{
    DecorationConfig::EnumShadowStyle::type style;
    int distance;
    QRgb color;
    auto operator<=>(const ShadowKey &) const = default;
};
std::map<ShadowKey, std::weak_ptr<KDecoration3::DecorationShadow>> s_shadowCache;
int s_decorationCount = 0;

std::shared_ptr<KDecoration3::DecorationShadow> sharedShadow(DecorationConfig::EnumShadowStyle::type style, int distance, const QColor &color)
{
    auto &cache = s_shadowCache;

    const ShadowKey key {style, distance, color.rgba()};
    if (auto shadow = cache[key].lock()) {
        return shadow;
    }
    std::erase_if(cache, [](const auto &entry) { return entry.second.expired(); });

    const ShadowTexture texture = style == DecorationConfig::EnumShadowStyle::Solid ? solidShadow(distance, color) : softShadow(distance, color);
    auto shadow = std::make_shared<KDecoration3::DecorationShadow>();
    shadow->setShadow(texture.image);
    shadow->setInnerShadowRect(texture.innerRect);
    shadow->setPadding(texture.padding);
    cache[key] = shadow;
    return shadow;
}

} // namespace

Decoration::Decoration(QObject *parent, const QVariantList &args)
    : KDecoration3::Decoration(parent, args)
    , m_config(std::make_unique<DecorationConfig>())
{
    ++s_decorationCount;
}

Decoration::~Decoration()
{
    // Our members (m_config, the buttons list, ...) are destroyed before the KDecoration base
    // class, but signal connections to this object only go away in ~QObject, after it. Cut them
    // now, so that nothing emitted while KDecoration tears the window down (typically at logout,
    // for every window at once) reaches a slot that uses destroyed members.
    if (window()) {
        disconnect(window(), nullptr, this, nullptr);
    }
    if (const auto s = settings()) {
        disconnect(s.get(), nullptr, this, nullptr);
    }
    if (m_configWatcher) {
        disconnect(m_configWatcher.data(), nullptr, this, nullptr);
    }
    if (m_globalsWatcher) {
        disconnect(m_globalsWatcher.data(), nullptr, this, nullptr);
    }
    for (Button *button : std::as_const(m_leftButtons) + std::as_const(m_rightButtons)) {
        disconnect(button, nullptr, this, nullptr);
    }

    // With the last window gone, drop the shadow cache entirely (as KDecoration may unload the
    // plugin afterwards).
    if (--s_decorationCount == 0) {
        s_shadowCache.clear();
    }
}

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
    connect(w, &KDecoration3::DecoratedWindow::activeChanged, this, [this] {
        updateShadow();
        update();
    });
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

int Decoration::fontHeight() const
{
    return int(std::ceil(settings()->fontMetrics().height()));
}

// Caption buttons are 14px tall as in the classic layout; only fonts much taller than the classic
// 8pt title font make them grow. The button padding option adds to that.
int Decoration::buttonHeight() const
{
    return std::max(CaptionButtonSize.height(), fontHeight() - 4) + 2 * m_config->buttonPadding();
}

// The title bar wraps the buttons with the button margin option (2px in the classic layout), but
// is never shorter than the caption text; buttons are then centered vertically.
int Decoration::titleHeight() const
{
    return std::max(buttonHeight() + 2 * m_config->buttonMargin(), fontHeight());
}

QRect Decoration::titleRect() const
{
    const int f = frameWidth();
    return QRect(f, f, std::max(0, int(size().width()) - 2 * f), titleHeight());
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
                // Some buttons show or hide with the window state (help, on all desktops).
                connect(button, &KDecoration3::DecorationButton::visibilityChanged, this, &Decoration::updateLayout);
                list.append(button);
            }
        }
    };
    create(settings()->decorationButtonsLeft(), m_leftButtons);
    create(settings()->decorationButtonsRight(), m_rightButtons);

    updateLayout();
}

// Places the buttons as in the classic layout: minimize and maximize touch, every other pair of
// neighbors keeps a small gap; the window icon keeps its own margin. Hidden buttons (e.g. help on
// windows without context help) take no space.
void Decoration::layoutButtons()
{
    const QRect title = titleRect();
    const int margin = m_config->buttonMargin();
    const int h = buttonHeight();
    const QSize captionSize(h + 2, h);
    const int top = title.top() + (title.height() - h) / 2;
    const int iconSize = title.height() - 2;

    const auto gap = [](const Button *left, const Button *right) {
        if (left->isIcon() || right->isIcon()) {
            return IconMargin;
        }
        if (left->type() == KDecoration3::DecorationButtonType::Minimize && right->type() == KDecoration3::DecorationButtonType::Maximize) {
            return 0;
        }
        return ButtonGap;
    };
    const auto width = [&](const Button *button) { return button->isIcon() ? iconSize : captionSize.width(); };
    const auto place = [&](Button *button, int x) {
        if (button->isIcon()) {
            button->setGeometry(QRectF(x, title.top() + 1, iconSize, iconSize));
        } else {
            button->setGeometry(QRectF(QPointF(x, top), captionSize));
        }
    };
    const auto edgeMargin = [&](const Button *button) { return button->isIcon() ? IconMargin : margin; };

    const QList<Button *> left = visibleButtons(m_leftButtons);
    if (!left.isEmpty()) {
        int x = title.left() + edgeMargin(left.first());
        for (int i = 0; i < left.size(); ++i) {
            if (i > 0) {
                x += gap(left.at(i - 1), left.at(i));
            }
            place(left.at(i), x);
            x += width(left.at(i));
        }
    }

    const QList<Button *> right = visibleButtons(m_rightButtons);
    if (!right.isEmpty()) {
        int x = title.right() + 1 - edgeMargin(right.last());
        for (int i = right.size() - 1; i >= 0; --i) {
            if (i < right.size() - 1) {
                x -= gap(right.at(i), right.at(i + 1));
            }
            x -= width(right.at(i));
            place(right.at(i), x);
        }
    }
}

QList<Button *> Decoration::visibleButtons(const QList<Button *> &buttons)
{
    QList<Button *> visible;
    for (Button *button : buttons) {
        if (button->isVisible()) {
            visible.append(button);
        }
    }
    return visible;
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
    const int alpha = style == DecorationConfig::EnumShadowStyle::Solid ? SolidShadowAlpha : SoftShadowAlpha;
    color.setAlpha(window()->isActive() ? alpha : int(alpha * InactiveShadowOpacity));
    setShadow(sharedShadow(style, m_config->shadowDistance(), color));
}

Decoration::TitleColors Decoration::titleColors(bool active) const
{
    KDecoration3::DecoratedWindow *w = window();
    const KDecoration3::ColorGroup group = active ? KDecoration3::ColorGroup::Active : KDecoration3::ColorGroup::Inactive;
    TitleColors colors {
        w->color(group, KDecoration3::ColorRole::TitleBar),
        active ? m_activeBlend : m_inactiveBlend,
        w->color(group, KDecoration3::ColorRole::Foreground),
    };
    if (!colors.blend.isValid()) {
        colors.blend = colors.background;
    }

    if (active && m_config->accentTitleBar()) {
        // KDE writes the accent color into the selection colors, so Highlight is the accent.
        const QPalette pal = palette();
        colors.background = pal.color(QPalette::Active, QPalette::Highlight);
        colors.blend = mix(colors.background, pal.color(QPalette::Active, QPalette::Light), AccentBlendAmount);
        colors.foreground = pal.color(QPalette::Active, QPalette::HighlightedText);
    } else if (!active && m_config->tintInactiveTitle()) {
        // A hint of the active title colors (accent or scheme), keeping the inactive text color.
        const TitleColors activeColors = titleColors(true);
        colors.background = mix(colors.background, activeColors.background, InactiveTintAmount);
        colors.blend = mix(colors.blend, activeColors.blend, InactiveTintAmount);
    }
    return colors;
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
    const TitleColors colors = titleColors(w->isActive());
    if (m_config->titleBarStyle() == DecorationConfig::EnumTitleBarStyle::Solid) {
        fillRect(painter, title, colors.background);
    } else {
        PixelPainter pp(painter);
        const QRect device = pp.toDevice(title);
        if (device.isValid()) {
            QLinearGradient gradient(device.topLeft(), device.topRight());
            gradient.setColorAt(0, colors.background);
            gradient.setColorAt(1, colors.blend);
            painter->fillRect(device, gradient);
        }
    }

    // Caption text between the left and right buttons.
    int textLeft = title.left() + TextGap;
    for (const Button *button : visibleButtons(m_leftButtons)) {
        textLeft = std::max(textLeft, int(button->geometry().right()) + TextGap);
    }
    int textRight = title.right() - TextMargin;
    for (const Button *button : visibleButtons(m_rightButtons)) {
        textRight = std::min(textRight, int(button->geometry().left()) - TextMargin);
    }
    if (textRight > textLeft) {
        const QRect textRect(textLeft, title.top(), textRight - textLeft, title.height());
        painter->save();
        QFont font = settings()->font();
        font.setWeight(titleWeight(m_config->titleWeight()));
        painter->setFont(font);
        painter->setPen(colors.foreground);
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
