# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Global theme: applies colors, decoration, application and Plasma styles in one go, and brings
# the splash screen and an optional classic panel layout.
install(DIRECTORY beveled.desktop DESTINATION ${KDE_INSTALL_DATADIR}/plasma/look-and-feel)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
