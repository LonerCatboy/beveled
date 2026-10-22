// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KCModule>

namespace Beveled {

class StyleConfig;

// Form shown by System Settings for the style's "Configure..." button. Changes apply to
// applications started afterwards.
class StyleKcm : public KCModule
{
    Q_OBJECT

public:
    StyleKcm(QObject *parent, const KPluginMetaData &data);

    // KCModule only refreshes the widgets from the skeleton; read beveledrc first.
    void load() override;

private:
    StyleConfig *m_config;
};

} // namespace Beveled
