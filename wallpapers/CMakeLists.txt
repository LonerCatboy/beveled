# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Wallpaper packages, set by the global themes (light: Synevyr, dark: Skäralid) and listed in
# the wallpaper picker. The photos come from Wikimedia Commons, CC BY-SA 4.0 (see REUSE.toml).
foreach(package beveled-synevyr beveled-skaralid)
    install(DIRECTORY ${package} DESTINATION ${KDE_INSTALL_WALLPAPERDIR})
endforeach()
