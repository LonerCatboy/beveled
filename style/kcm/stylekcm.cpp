// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "stylekcm.h"

#include "beveledstyle.h"

#include <formhelpers.h>

#include <KConfigDialogManager>
#include <KPluginFactory>

#include <QFormLayout>
#include <QLabel>

namespace Beveled {

void buildStyleForm(QWidget *form, StyleConfig *config)
{
    auto layout = new QFormLayout(form);

    addSlider(layout, QStringLiteral("Bevel shade depth:"), config->shadeDepthItem(), 10, QStringLiteral("%"));
    addChoice(layout, QStringLiteral("Sunken frames:"), QStringLiteral("SunkenFrame"), {QStringLiteral("Soft (classic)"), QStringLiteral("Hard")});
    addChoice(layout, QStringLiteral("Tabs:"), QStringLiteral("TabShape"), {QStringLiteral("Square"), QStringLiteral("Classic (clipped corners)")});

    addSpacer(layout);
    addSlider(layout, QStringLiteral("Button padding, sides:"), config->buttonPaddingXItem(), 1, QStringLiteral("px"));
    addSlider(layout, QStringLiteral("Button padding, top and bottom:"), config->buttonPaddingYItem(), 1, QStringLiteral("px"));
    addSlider(layout, QStringLiteral("Minimum button width:"), config->buttonMinWidthItem(), 10, QStringLiteral("px"));

    addSpacer(layout);
    addSlider(layout, QStringLiteral("Window and group margins:"), config->layoutMarginItem(), 1, QStringLiteral("px"));
    addSlider(layout, QStringLiteral("Space between controls:"), config->layoutSpacingItem(), 1, QStringLiteral("px"));

    addSpacer(layout);
    auto note = new QLabel(QStringLiteral("The bevel shade depth is shared with the Beveled window decoration."), form);
    note->setWordWrap(true);
    layout->addRow(note);
}

StyleConfigWidget::StyleConfigWidget(QWidget *parent)
    : QWidget(parent)
    , m_config(new StyleConfig(this))
{
    m_config->load();
    buildStyleForm(this, m_config);
    m_manager = new KConfigDialogManager(this, m_config);
    connect(m_manager, &KConfigDialogManager::widgetModified, this, [this] { Q_EMIT changed(m_manager->hasChanged()); });
}

void StyleConfigWidget::save()
{
    m_manager->updateSettings();
    Q_EMIT changed(false);
}

void StyleConfigWidget::defaults()
{
    m_manager->updateWidgetsDefault();
    Q_EMIT changed(m_manager->hasChanged());
}

StyleKcm::StyleKcm(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_config(new StyleConfig(this))
{
    buildStyleForm(widget(), m_config);
    addConfig(m_config, widget());
}

void StyleKcm::load()
{
    m_config->load();
    KCModule::load();
}

} // namespace Beveled

extern "C" {
Q_DECL_EXPORT QWidget *allocate_kstyle_config(QWidget *parent)
{
    return new Beveled::StyleConfigWidget(parent);
}
}

K_PLUGIN_CLASS_WITH_JSON(Beveled::StyleKcm, "beveledstyleconfig.json")

#include "stylekcm.moc"
