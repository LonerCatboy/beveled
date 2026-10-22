# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)
include(ECMAddTests)

# Loads the style plugin from the build tree by name, as applications do. The config directory
# points into the build tree so the user's beveledrc is never read.
ecm_add_test(styletest.cpp
    TEST_NAME styletest
    LINK_LIBRARIES Beveled::Core KF6::ConfigCore KF6::CoreAddons KF6::KCMUtils Qt6::Test Qt6::Widgets
)
add_dependencies(styletest beveledstyle beveledstyleconfig)
target_compile_definitions(styletest PRIVATE STYLE_KCM_PATH="$<TARGET_FILE:beveledstyleconfig>")
set_tests_properties(styletest PROPERTIES ENVIRONMENT
    "QT_QPA_PLATFORM=offscreen;QT_PLUGIN_PATH=${CMAKE_BINARY_DIR}/bin;XDG_CONFIG_HOME=${CMAKE_CURRENT_BINARY_DIR}/config")
