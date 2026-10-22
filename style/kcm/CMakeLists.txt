# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Settings module behind the "Configure..." button of the style in System Settings, linked from
# beveled.themerc ("ConfigPage=kstyle_config/beveledstyleconfig").
kcoreaddons_add_plugin(beveledstyleconfig
    SOURCES
        stylekcm.cpp
    INSTALL_NAMESPACE "kstyle_config"
)
kconfig_add_kcfg_files(beveledstyleconfig ../beveledstyle.kcfgc)

target_link_libraries(beveledstyleconfig PRIVATE
    Beveled::KcmCommon
    KF6::ConfigGui
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::KCMUtils
    Qt6::Widgets
)
