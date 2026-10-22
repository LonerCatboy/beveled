# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Static library: linked into both the style and the decoration plugins, never installed.
add_library(beveledcore STATIC
    src/beveled/bevel.cpp
    src/beveled/glyphs.cpp
    src/beveled/pixelpainter.cpp
    src/beveled/shades.cpp
    src/beveled/shadow.cpp
)
add_library(Beveled::Core ALIAS beveledcore)

set_target_properties(beveledcore PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(beveledcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(beveledcore PUBLIC Qt6::Gui)

if(BUILD_TESTING)
    add_subdirectory(autotests)
endif()
