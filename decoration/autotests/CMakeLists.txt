# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)
include(ECMAddTests)

# Loads the settings module from the build tree and checks its widgets against beveledrc. The
# config file lives in the build directory, never in the user's ~/.config.
ecm_add_test(kcmtest.cpp
    TEST_NAME kcmtest
    LINK_LIBRARIES KF6::ConfigGui KF6::CoreAddons KF6::KCMUtils Qt6::Test Qt6::Widgets
)
target_compile_definitions(kcmtest PRIVATE KCM_PLUGIN_PATH="$<TARGET_FILE:kcm_beveleddecoration>")
add_dependencies(kcmtest kcm_beveleddecoration)
set_tests_properties(kcmtest PROPERTIES ENVIRONMENT
    "QT_QPA_PLATFORM=offscreen;XDG_CONFIG_HOME=${CMAKE_CURRENT_BINARY_DIR}/config")
