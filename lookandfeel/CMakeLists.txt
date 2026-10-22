# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Global themes: each applies colors, decoration, application and Plasma styles in one go. The
# light and dark variants share the splash screen and the optional classic panel layout, kept
# once in common/ and installed into both packages.
foreach(package beveled.desktop beveled-dark.desktop)
    set(destination ${KDE_INSTALL_DATADIR}/plasma/look-and-feel/${package})
    install(DIRECTORY ${package}/ DESTINATION ${destination})
    install(DIRECTORY common/contents/ DESTINATION ${destination}/contents)
endforeach()

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
