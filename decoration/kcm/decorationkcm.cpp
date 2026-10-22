// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "decorationkcm.h"

#include "beveledconfig.h"

#include <KPluginFactory>

#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>

namespace Beveled {

DecorationKcm::DecorationKcm(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_config(new DecorationConfig(this))
{
    QWidget *form = widget();
    auto layout = new QFormLayout(form);

    // Combo box indexes follow the order of the choices in beveledconfig.kcfg.
    auto titleBar = new QComboBox(form);
    titleBar->setObjectName(QStringLiteral("kcfg_TitleBarStyle"));
    titleBar->addItems({QStringLiteral("Gradient"), QStringLiteral("Solid color")});
    layout->addRow(QStringLiteral("Title bar:"), titleBar);

    auto bevel = new QComboBox(form);
    bevel->setObjectName(QStringLiteral("kcfg_ButtonBevel"));
    bevel->addItems({QStringLiteral("Soft (classic)"), QStringLiteral("Hard")});
    layout->addRow(QStringLiteral("Button bevel:"), bevel);

    // The slider range comes from the <min>/<max> of the entry, so it cannot drift from the schema.
    const KConfigSkeleton::ItemInt *paddingItem = m_config->buttonPaddingItem();
    auto padding = new QSlider(Qt::Horizontal, form);
    padding->setObjectName(QStringLiteral("kcfg_ButtonPadding"));
    padding->setRange(paddingItem->minValue().toInt(), paddingItem->maxValue().toInt());
    padding->setPageStep(1);
    padding->setTickPosition(QSlider::TicksBelow);
    auto paddingValue = new QLabel(form);
    connect(padding, &QSlider::valueChanged, paddingValue, [paddingValue](int value) { paddingValue->setText(QStringLiteral("%1 px").arg(value)); });
    auto paddingRow = new QHBoxLayout;
    paddingRow->addWidget(padding);
    paddingRow->addWidget(paddingValue);
    layout->addRow(QStringLiteral("Button padding:"), paddingRow);

    addConfig(m_config, form);
    paddingValue->setText(QStringLiteral("%1 px").arg(padding->value()));
}

void DecorationKcm::load()
{
    m_config->load();
    KCModule::load();
}

} // namespace Beveled

K_PLUGIN_CLASS_WITH_JSON(Beveled::DecorationKcm, "kcm_beveleddecoration.json")

#include "decorationkcm.moc"
