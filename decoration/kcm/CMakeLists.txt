# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Settings module behind the "Configure..." button of the decoration. The decoration finds it
# through "X-KDE-ConfigModule" in beveled.json.
kcoreaddons_add_plugin(kcm_beveleddecoration
    SOURCES
        decorationkcm.cpp
    INSTALL_NAMESPACE "${KDECORATION_KCM_PLUGIN_DIR}"
)
kconfig_add_kcfg_files(kcm_beveleddecoration ../beveledconfig.kcfgc)

target_link_libraries(kcm_beveleddecoration PRIVATE
    KF6::ConfigGui
    KF6::ConfigWidgets
    KF6::CoreAddons
    KF6::KCMUtils
    Qt6::Widgets
)
