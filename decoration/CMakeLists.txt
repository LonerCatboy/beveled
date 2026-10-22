# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# KWin loads window decorations from <qt plugins>/org.kde.kdecoration3/; the file name is the
# decoration id ("beveled").
kcoreaddons_add_plugin(beveled
    SOURCES
        plugin.cpp
        decoration.cpp
        button.cpp
    INSTALL_NAMESPACE "${KDECORATION_PLUGIN_DIR}"
)
kconfig_add_kcfg_files(beveled beveledconfig.kcfgc)

target_link_libraries(beveled PRIVATE
    Beveled::Core
    KDecoration3::KDecoration
    KF6::ConfigGui
    KF6::CoreAddons
    Qt6::Gui
)

add_subdirectory(kcm)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
