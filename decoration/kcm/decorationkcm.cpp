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

    addChoice(layout, QStringLiteral("Title bar:"), QStringLiteral("TitleBarStyle"), {QStringLiteral("Gradient"), QStringLiteral("Solid color")});
    auto accent = new QCheckBox(QStringLiteral("Use the accent color for the active title bar"), widget());
    accent->setObjectName(QStringLiteral("kcfg_AccentTitleBar"));
    layout->addRow(QString(), accent);
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
}

void DecorationKcm::load()
{
    m_config->load();
    KCModule::load();
}

} // namespace Beveled

K_PLUGIN_CLASS_WITH_JSON(Beveled::DecorationKcm, "kcm_beveleddecoration.json")

#include "decorationkcm.moc"
