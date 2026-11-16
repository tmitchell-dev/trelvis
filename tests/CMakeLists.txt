# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Timothy Mitchell
#
# Development build, used only when TreLVis is the top-level project:
#
#   cmake -B build                       # fetches LVGL (TRLV_LVGL_GIT_TAG)
#   cmake --build build
#
# Options:
#   -DTRLV_LVGL_GIT_TAG=master                    build against another LVGL ref
#   -DFETCHCONTENT_SOURCE_DIR_LVGL=path/to/lvgl   use a local LVGL tree instead

# ---- LVGL, default configuration ----
set(TRLV_LVGL_GIT_TAG "v9.6.0" CACHE STRING "LVGL tag or branch for the development build")

set(LV_CONF_SKIP ON CACHE BOOL "" FORCE)
set(LV_BUILD_INSTALL OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG OFF CACHE BOOL "" FORCE)

include(FetchContent)
FetchContent_Declare(lvgl
    GIT_REPOSITORY https://github.com/lvgl/lvgl.git
    GIT_TAG        ${TRLV_LVGL_GIT_TAG}
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(lvgl)

# ---- Strict warnings for TreLVis code (not for LVGL) ----
set(TRLV_STRICT_WARNINGS -Wall -Wextra -Wpedantic -Werror)
target_compile_options(trelvis PRIVATE ${TRLV_STRICT_WARNINGS})

# ---- Compile-only: the token template must build unedited ----
add_library(trlv_tokens_template OBJECT ../templates/trlv_tokens_template.c)
target_include_directories(trlv_tokens_template PUBLIC ${PROJECT_SOURCE_DIR}/templates)
target_link_libraries(trlv_tokens_template PUBLIC trelvis)
target_compile_options(trlv_tokens_template PRIVATE ${TRLV_STRICT_WARNINGS})
set_target_properties(trlv_tokens_template PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF)

# ---- Compile-only: the public headers must work from C++17 ----
enable_language(CXX)
add_library(trlv_cpp_include_test OBJECT cpp_include_test.cpp)
target_link_libraries(trlv_cpp_include_test PRIVATE trelvis trlv_tokens_template)
target_compile_options(trlv_cpp_include_test PRIVATE ${TRLV_STRICT_WARNINGS})
set_target_properties(trlv_cpp_include_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)

# ---- Headless layout tests (run with: ctest --test-dir build) ----
add_library(trlv_test_harness STATIC harness/trlv_test.c)
target_include_directories(trlv_test_harness PUBLIC harness)
target_link_libraries(trlv_test_harness PUBLIC trelvis)
target_compile_options(trlv_test_harness PRIVATE ${TRLV_STRICT_WARNINGS})
set_target_properties(trlv_test_harness PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF)

# One executable per test file; each file's main() runs its tests.
function(trlv_add_test name source)
    add_executable(${name} ${source})
    target_link_libraries(${name} PRIVATE trlv_test_harness)
    target_compile_options(${name} PRIVATE ${TRLV_STRICT_WARNINGS})
    set_target_properties(${name} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

trlv_add_test(layout_smoke layout/test_smoke.c)

# Tests of internal helpers also see src/.
trlv_add_test(internal_style_group internal/test_style_group.c)
target_include_directories(internal_style_group PRIVATE ${PROJECT_SOURCE_DIR}/src)

# Harness self-test: a deliberately wrong assertion must fail the program...
trlv_add_test(harness_failure_exit harness/selftest_failure.c)
set_tests_properties(harness_failure_exit PROPERTIES WILL_FAIL TRUE)
# ...and print a readable message (same program, checked on its output).
add_test(NAME harness_failure_message COMMAND harness_failure_exit)
set_tests_properties(harness_failure_message PROPERTIES
    PASS_REGULAR_EXPRESSION "selftest_failure\\.c:[0-9]+: in wrong_expectation: box\\.x expected 99, got 10")
