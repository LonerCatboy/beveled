// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "decorationkcm.h"

#include "beveledconfig.h"

#include <KPluginFactory>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

#include <algorithm>

namespace Beveled {

namespace {

// Combo box indexes follow the order of the choices in beveledconfig.kcfg.
QComboBox *addChoice(QFormLayout *layout, const QString &label, const QString &entry, const QStringList &choices)
{
    auto combo = new QComboBox(layout->parentWidget());
    combo->setObjectName(QStringLiteral("kcfg_") + entry);
    combo->addItems(choices);
    layout->addRow(label, combo);
    return combo;
}

// The slider range comes from the <min>/<max> of the entry, so it cannot drift from the schema.
QSlider *addSlider(QFormLayout *layout, const QString &label, const KConfigSkeleton::ItemInt *item, int pageStep, const QString &unit)
{
    QWidget *parent = layout->parentWidget();
    auto slider = new QSlider(Qt::Horizontal, parent);
    slider->setObjectName(QStringLiteral("kcfg_") + item->name());
    slider->setRange(item->minValue().toInt(), item->maxValue().toInt());
    slider->setPageStep(pageStep);
    slider->setTickInterval(pageStep);
    slider->setTickPosition(QSlider::TicksBelow);

    auto value = new QLabel(parent);
    const auto showValue = [value, unit](int v) { value->setText(QStringLiteral("%1 %2").arg(v).arg(unit)); };
    QObject::connect(slider, &QSlider::valueChanged, value, showValue);
    showValue(slider->value());

    auto row = new QHBoxLayout;
    row->addWidget(slider);
    row->addWidget(value);
    layout->addRow(label, row);
    return slider;
}

QCheckBox *addCheck(QFormLayout *layout, const QString &text, const QString &entry)
{
    auto check = new QCheckBox(text, layout->parentWidget());
    check->setObjectName(QStringLiteral("kcfg_") + entry);
    layout->addRow(QString(), check);
    return check;
}

// A preset sets several options at once, by entry name; combo boxes take the choice index.
// Every preset lists the same entries, so that the preset menu can tell which one is in use.
struct Preset
{
    QString name;
    QList<std::pair<QString, int>> values;
};

QList<Preset> presets()
{
    const auto values = [](int titleBarStyle, bool modern) {
        return QList<std::pair<QString, int>> {
            {QStringLiteral("TitleBarStyle"), titleBarStyle},
            {QStringLiteral("AccentTitleBar"), modern},
            {QStringLiteral("TintInactiveTitle"), modern},
            {QStringLiteral("TitleWeight"), DecorationConfig::EnumTitleWeight::Normal},
            {QStringLiteral("FrameBevel"), modern ? DecorationConfig::EnumFrameBevel::Hard : DecorationConfig::EnumFrameBevel::Soft},
            {QStringLiteral("ShadeDepth"), modern ? 80 : 100},
            {QStringLiteral("ButtonBevel"), modern ? DecorationConfig::EnumButtonBevel::Hard : DecorationConfig::EnumButtonBevel::Soft},
            {QStringLiteral("ButtonHover"), modern ? DecorationConfig::EnumButtonHover::Highlight : DecorationConfig::EnumButtonHover::None},
            {QStringLiteral("ButtonMargin"), 2},
            {QStringLiteral("ButtonPadding"), 0},
            {QStringLiteral("ShadowStyle"), DecorationConfig::EnumShadowStyle::Soft},
            {QStringLiteral("ShadowDistance"), 4},
        };
    };
    return {
        {QStringLiteral("Classic, gradient title bar"), values(DecorationConfig::EnumTitleBarStyle::Gradient, false)},
        {QStringLiteral("Classic, flat title bar"), values(DecorationConfig::EnumTitleBarStyle::Solid, false)},
        {QStringLiteral("Modern"), values(DecorationConfig::EnumTitleBarStyle::Gradient, true)},
    };
}

int widgetValue(const QWidget *widget)
{
    if (auto combo = qobject_cast<const QComboBox *>(widget)) {
        return combo->currentIndex();
    }
    if (auto check = qobject_cast<const QCheckBox *>(widget)) {
        return check->isChecked();
    }
    if (auto slider = qobject_cast<const QSlider *>(widget)) {
        return slider->value();
    }
    return -1;
}

void setWidgetValue(QWidget *widget, int value)
{
    if (auto combo = qobject_cast<QComboBox *>(widget)) {
        combo->setCurrentIndex(value);
    } else if (auto check = qobject_cast<QCheckBox *>(widget)) {
        check->setChecked(value != 0);
    } else if (auto slider = qobject_cast<QSlider *>(widget)) {
        slider->setValue(value);
    }
}

QWidget *optionWidget(QWidget *form, const QString &entry)
{
    return form->findChild<QWidget *>(QStringLiteral("kcfg_") + entry);
}

void addSpacer(QFormLayout *layout)
{
    layout->addItem(new QSpacerItem(0, layout->parentWidget()->fontMetrics().height() / 2));
}

} // namespace

DecorationKcm::DecorationKcm(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_config(new DecorationConfig(this))
{
    auto layout = new QFormLayout(widget());

    // Not a stored option: the menu shows the preset matching the current options, or "Custom".
    // Picking a preset fills in the widgets below, which KCModule sees like any user edit.
    m_preset = new QComboBox(widget());
    m_preset->setObjectName(QStringLiteral("preset"));
    for (const Preset &p : presets()) {
        m_preset->addItem(p.name);
    }
    m_preset->addItem(QStringLiteral("Custom"));
    connect(m_preset, &QComboBox::activated, this, [this](int index) {
        const QList<Preset> all = presets(); // keep alive: a range-for over presets().at(i) would dangle
        if (index < all.size()) {
            for (const auto &[entry, value] : all.at(index).values) {
                setWidgetValue(optionWidget(widget(), entry), value);
            }
        }
        updatePreset();
    });
    layout->addRow(QStringLiteral("Preset:"), m_preset);
    addSpacer(layout);

    addChoice(layout, QStringLiteral("Title bar:"), QStringLiteral("TitleBarStyle"), {QStringLiteral("Gradient"), QStringLiteral("Solid color")});
    addCheck(layout, QStringLiteral("Use the accent color for the active title bar"), QStringLiteral("AccentTitleBar"));
    addCheck(layout, QStringLiteral("Tint inactive title bars with the active title color"), QStringLiteral("TintInactiveTitle"));
    addChoice(layout, QStringLiteral("Title text weight:"), QStringLiteral("TitleWeight"),
        {QStringLiteral("Normal"), QStringLiteral("Medium"), QStringLiteral("Semi-bold"), QStringLiteral("Bold")});
    addChoice(layout, QStringLiteral("Window frame:"), QStringLiteral("FrameBevel"), {QStringLiteral("Soft (classic)"), QStringLiteral("Hard")});
    addSlider(layout, QStringLiteral("Bevel shade depth:"), m_config->shadeDepthItem(), 10, QStringLiteral("%"));

    addSpacer(layout);
    addChoice(layout, QStringLiteral("Button bevel:"), QStringLiteral("ButtonBevel"), {QStringLiteral("Soft (classic)"), QStringLiteral("Hard")});
    addChoice(layout, QStringLiteral("Button hover:"), QStringLiteral("ButtonHover"),
        {QStringLiteral("No effect (classic)"), QStringLiteral("Highlight"), QStringLiteral("Slightly pressed")});
    addSlider(layout, QStringLiteral("Space around buttons:"), m_config->buttonMarginItem(), 1, QStringLiteral("px"));
    addSlider(layout, QStringLiteral("Space around symbols:"), m_config->buttonPaddingItem(), 1, QStringLiteral("px"));

    addSpacer(layout);
    auto shadow = addChoice(
        layout, QStringLiteral("Shadow:"), QStringLiteral("ShadowStyle"), {QStringLiteral("None"), QStringLiteral("Soft"), QStringLiteral("Solid")});
    auto distance = addSlider(layout, QStringLiteral("Shadow distance:"), m_config->shadowDistanceItem(), 1, QStringLiteral("px"));
    const auto updateDistance = [shadow, distance] { distance->setEnabled(shadow->currentIndex() != DecorationConfig::EnumShadowStyle::None); };
    connect(shadow, &QComboBox::currentIndexChanged, distance, updateDistance);

    addConfig(m_config, widget());
    updateDistance();

    // Any option change may turn a preset into "Custom" or match another preset.
    for (QWidget *option : widget()->findChildren<QWidget *>()) {
        if (!option->objectName().startsWith(QLatin1String("kcfg_"))) {
            continue;
        }
        if (auto combo = qobject_cast<QComboBox *>(option)) {
            connect(combo, &QComboBox::currentIndexChanged, this, &DecorationKcm::updatePreset);
        } else if (auto check = qobject_cast<QCheckBox *>(option)) {
            connect(check, &QCheckBox::toggled, this, &DecorationKcm::updatePreset);
        } else if (auto slider = qobject_cast<QSlider *>(option)) {
            connect(slider, &QSlider::valueChanged, this, &DecorationKcm::updatePreset);
        }
    }
    updatePreset();
}

void DecorationKcm::load()
{
    m_config->load();
    KCModule::load();
    updatePreset();
}

void DecorationKcm::defaults()
{
    KCModule::defaults();
    updatePreset();
}

void DecorationKcm::updatePreset()
{
    const QList<Preset> all = presets();
    int match = all.size(); // "Custom"
    for (int i = 0; i < all.size(); ++i) {
        const bool matches = std::all_of(all.at(i).values.cbegin(), all.at(i).values.cend(),
            [this](const auto &entry) { return widgetValue(optionWidget(widget(), entry.first)) == entry.second; });
        if (matches) {
            match = i;
            break;
        }
    }
    m_preset->setCurrentIndex(match);
}

} // namespace Beveled

K_PLUGIN_CLASS_WITH_JSON(Beveled::DecorationKcm, "kcm_beveleddecoration.json")

#include "decorationkcm.moc"
