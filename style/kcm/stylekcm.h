// SPDX-FileCopyrightText: 2026 LonerCatboy
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

#include <KCModule>

#include <QWidget>

class KConfigDialogManager;

namespace Beveled {

class StyleConfig;

// Builds the options form into form; widgets are named "kcfg_<Entry>" for KConfigXT.
void buildStyleForm(QWidget *form, StyleConfig *config);

// The page System Settings shows for the style's "Configure..." button. The Application Style
// page loads it through the C function allocate_kstyle_config() and talks to it with these
// exact signal and slot signatures (connected by name).
class StyleConfigWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StyleConfigWidget(QWidget *parent = nullptr);

public Q_SLOTS:
    void save();
    void defaults();

Q_SIGNALS:
    void changed(bool changed);

private:
    StyleConfig *m_config;
    KConfigDialogManager *m_manager;
};

// The same form as a KCModule, which also makes the library a regular Qt plugin.
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
