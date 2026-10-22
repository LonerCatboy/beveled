# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Plasma styles (desktoptheme), generated from generate.py at build time so the repository has a
# single source for the artwork.
find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(theme_dir ${CMAKE_CURRENT_BINARY_DIR}/desktoptheme)
set(stamp ${CMAKE_CURRENT_BINARY_DIR}/desktoptheme.stamp)
add_custom_command(
    OUTPUT ${stamp}
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${theme_dir}
    COMMAND Python3::Interpreter -I ${CMAKE_CURRENT_SOURCE_DIR}/generate.py ${theme_dir}
    COMMAND ${CMAKE_COMMAND} -E touch ${stamp}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/generate.py
    COMMENT "Generating the Beveled Plasma styles"
)
add_custom_target(plasma-styles ALL DEPENDS ${stamp})

install(DIRECTORY ${theme_dir}/ DESTINATION ${KDE_INSTALL_DATADIR}/plasma/desktoptheme)

if(BUILD_TESTING)
    foreach(theme beveled beveled-dark)
        add_test(NAME plasmastyle-${theme} COMMAND Python3::Interpreter -I ${CMAKE_CURRENT_SOURCE_DIR}/generate.py --check ${theme_dir}/${theme})
    endforeach()
endif()
