# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Application style: a proxy around Qt's built-in Windows style that keeps its drawing and only
# changes what Beveled needs (button padding, 3D shades). Applications load it from
# <qt plugins>/styles/ by its key, "Beveled".
add_library(beveledstyle MODULE
    plugin.cpp
    style.cpp
)
kconfig_add_kcfg_files(beveledstyle beveledstyle.kcfgc)
set_target_properties(beveledstyle PROPERTIES
    OUTPUT_NAME beveled
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/styles
)
target_link_libraries(beveledstyle PRIVATE
    Beveled::Core
    KF6::ConfigGui
    Qt6::Widgets
)

install(TARGETS beveledstyle DESTINATION ${KDE_INSTALL_QTPLUGINDIR}/styles)
install(FILES beveled.themerc DESTINATION ${KDE_INSTALL_DATADIR}/kstyle/themes)

add_subdirectory(kcm)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
