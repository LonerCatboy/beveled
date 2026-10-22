// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KCModule>

namespace Beveled {

class DecorationConfig;

// Form shown by System Settings for the decoration's "Configure..." button. Widgets named
// "kcfg_<Entry>" are loaded, saved and reset to defaults by KCModule through KConfigXT.
class DecorationKcm : public KCModule
{
    Q_OBJECT

public:
    DecorationKcm(QObject *parent, const KPluginMetaData &data);

    // KCModule only refreshes the widgets from the skeleton; read beveledrc first.
    void load() override;

private:
    DecorationConfig *m_config;
};

} // namespace Beveled
