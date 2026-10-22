# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

# Form helpers shared by the settings modules (decoration and application style).
add_library(beveledkcmcommon STATIC formhelpers.cpp)
add_library(Beveled::KcmCommon ALIAS beveledkcmcommon)
set_target_properties(beveledkcmcommon PROPERTIES POSITION_INDEPENDENT_CODE ON)
target_include_directories(beveledkcmcommon PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(beveledkcmcommon PUBLIC KF6::ConfigGui Qt6::Widgets)
