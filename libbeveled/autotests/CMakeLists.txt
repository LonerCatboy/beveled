# SPDX-FileCopyrightText: 2026 LonerCatboy
# SPDX-License-Identifier: GPL-2.0-or-later

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS Test)
include(ECMAddTests)

set(tests pixelpaintertest beveltest glyphtest shadestest shadowtest)
foreach(test ${tests})
    ecm_add_test(${test}.cpp LINK_LIBRARIES Beveled::Core Qt6::Test TEST_NAME ${test})
    # Render into images only: no display server needed (and the desktop is never touched).
    set_tests_properties(${test} PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endforeach()
