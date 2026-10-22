// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#include "stylekcm.h"

#include "beveledstyle.h"

#include <formhelpers.h>

#include <KPluginFactory>

#include <QFormLayout>
#include <QLabel>

namespace Beveled {

StyleKcm::StyleKcm(QObject *parent, const KPluginMetaData &data)
    : KCModule(parent, data)
    , m_config(new StyleConfig(this))
{
    auto layout = new QFormLayout(widget());

    addSlider(layout, QStringLiteral("Bevel shade depth:"), m_config->shadeDepthItem(), 10, QStringLiteral("%"));
    addChoice(layout, QStringLiteral("Sunken frames:"), QStringLiteral("SunkenFrame"), {QStringLiteral("Soft (classic)"), QStringLiteral("Hard")});

    addSpacer(layout);
    addSlider(layout, QStringLiteral("Button padding, sides:"), m_config->buttonPaddingXItem(), 1, QStringLiteral("px"));
    addSlider(layout, QStringLiteral("Button padding, top and bottom:"), m_config->buttonPaddingYItem(), 1, QStringLiteral("px"));
    addSlider(layout, QStringLiteral("Minimum button width:"), m_config->buttonMinWidthItem(), 10, QStringLiteral("px"));

    addSpacer(layout);
    addSlider(layout, QStringLiteral("Window and group margins:"), m_config->layoutMarginItem(), 1, QStringLiteral("px"));
    addSlider(layout, QStringLiteral("Space between controls:"), m_config->layoutSpacingItem(), 1, QStringLiteral("px"));

    addSpacer(layout);
    auto note = new QLabel(QStringLiteral("Changes apply to applications started afterwards. The bevel shade depth is shared "
                                          "with the Beveled window decoration."),
        widget());
    note->setWordWrap(true);
    layout->addRow(note);

    addConfig(m_config, widget());
}

void StyleKcm::load()
{
    m_config->load();
    KCModule::load();
}

} // namespace Beveled

K_PLUGIN_CLASS_WITH_JSON(Beveled::StyleKcm, "beveledstyleconfig.json")

#include "stylekcm.moc"
