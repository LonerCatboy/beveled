# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Developer tool, not installed.
add_executable(palette-dump main.cpp)
target_link_libraries(palette-dump Qt6::Gui KF6::ColorScheme KF6::ConfigCore)
