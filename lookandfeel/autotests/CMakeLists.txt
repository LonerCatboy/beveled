# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test Qml Quick)
include(ECMAddTests)

# Loads the splash screen and runs the layout script offscreen, straight from the source tree.
ecm_add_test(lookandfeeltest.cpp
    TEST_NAME lookandfeeltest
    LINK_LIBRARIES Qt6::Test Qt6::Qml Qt6::Quick
)
target_compile_definitions(lookandfeeltest PRIVATE PACKAGE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../common")
set_tests_properties(lookandfeeltest PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_QUICK_BACKEND=software")
