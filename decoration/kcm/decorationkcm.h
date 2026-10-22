// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KCModule>

class QComboBox;

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
    void defaults() override;

private:
    // Shows the preset matching the current options, or "Custom".
    void updatePreset();

    DecorationConfig *m_config;
    QComboBox *m_preset;
};

} // namespace Beveled
