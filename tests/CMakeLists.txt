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
#   -DTRLV_TEST_CONFIG=minimal                    build against tests/config/minimal
#                                                 (a small product's lv_conf.h and
#                                                 trlv_conf.h) and run the tests
#                                                 that need no widgets

# ---- LVGL ----
set(TRLV_LVGL_GIT_TAG "v9.6.0" CACHE STRING "LVGL tag or branch for the development build")
set(TRLV_TEST_CONFIG "default" CACHE STRING
    "LVGL configuration: default (LVGL's defaults, every test) or minimal (tests/config/minimal)")
set_property(CACHE TRLV_TEST_CONFIG PROPERTY STRINGS default minimal)

if(TRLV_TEST_CONFIG STREQUAL "minimal")
    set(TRLV_TEST_FULL OFF)
    set(LV_CONF_SKIP OFF CACHE BOOL "" FORCE)
    set(LV_BUILD_CONF_PATH ${CMAKE_CURRENT_LIST_DIR}/config/minimal/lv_conf.h CACHE PATH "" FORCE)
elseif(TRLV_TEST_CONFIG STREQUAL "default")
    set(TRLV_TEST_FULL ON)
    set(LV_CONF_SKIP ON CACHE BOOL "" FORCE)
else()
    message(FATAL_ERROR "TRLV_TEST_CONFIG must be default or minimal, not '${TRLV_TEST_CONFIG}'")
endif()
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

if(TRLV_TEST_FULL)
    # LVGL logging on, so tests can check TreLVis warnings through a log
    # callback. Nothing is printed (LV_LOG_PRINTF stays 0). CI's no-CMake
    # step and the minimal configuration compile src/ with logging off.
    target_compile_definitions(lvgl PUBLIC LV_USE_LOG=1)

    # A few Montserrat sizes besides LVGL's default 14 px, so the token
    # template's type scale has real hierarchy and the text tests can check
    # the line-height conversion with a font that isn't LV_FONT_DEFAULT.
    target_compile_definitions(lvgl PUBLIC
        LV_FONT_MONTSERRAT_12=1
        LV_FONT_MONTSERRAT_16=1
        LV_FONT_MONTSERRAT_24=1)
else()
    # The minimal trlv_conf.h, found on the include path as a product's is.
    target_include_directories(trelvis PUBLIC ${CMAKE_CURRENT_LIST_DIR}/config/minimal)
endif()

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

# The example HMI colour scheme next to the template, built the same way.
add_library(trlv_tokens_hmi OBJECT ../templates/trlv_tokens_hmi.c)
target_include_directories(trlv_tokens_hmi PUBLIC ${PROJECT_SOURCE_DIR}/templates)
target_link_libraries(trlv_tokens_hmi PUBLIC trelvis)
target_compile_options(trlv_tokens_hmi PRIVATE ${TRLV_STRICT_WARNINGS})
set_target_properties(trlv_tokens_hmi PROPERTIES
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

# ---- The demo (examples/demo), built with the same strict flags ----
# It shows LVGL's widgets, so the minimal configuration leaves it out.
if(TRLV_TEST_FULL)
    add_library(trlv_demo STATIC
        ../examples/demo/trlv_demo.c
        ../examples/demo/trlv_demo_readouts.c
        ../examples/demo/trlv_demo_surfaces.c
        ../examples/demo/trlv_demo_cards.c
        ../examples/demo/trlv_demo_controls.c
        ../examples/demo/trlv_demo_list.c)
    target_include_directories(trlv_demo PUBLIC ${PROJECT_SOURCE_DIR}/examples/demo)
    target_link_libraries(trlv_demo PUBLIC trelvis)
    target_compile_options(trlv_demo PRIVATE ${TRLV_STRICT_WARNINGS})
    set_target_properties(trlv_demo PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
endif()

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

# Tests that need no widget besides the label, no LVGL log and no font but
# Montserrat 14: these also run in the minimal configuration.
trlv_add_test(layout_smoke layout/test_smoke.c)
trlv_add_test(layout_box layout/test_box.c)
trlv_add_test(layout_spacing layout/test_spacing.c)
target_link_libraries(layout_spacing PRIVATE trlv_tokens_template)
trlv_add_test(layout_row_column layout/test_row_column.c)
target_link_libraries(layout_row_column PRIVATE trlv_tokens_template)
trlv_add_test(layout_flex_place layout/test_flex_place.c)
target_link_libraries(layout_flex_place PRIVATE trlv_tokens_template)
trlv_add_test(layout_debug_outline_off layout/test_debug_outline.c)
target_link_libraries(layout_debug_outline_off PRIVATE trlv_tokens_template)
trlv_add_test(layout_divider layout/test_divider.c)
target_link_libraries(layout_divider PRIVATE trlv_tokens_template)
trlv_add_test(theme_init theme/test_init.c)
target_link_libraries(theme_init PRIVATE trlv_tokens_template)
trlv_add_test(theme_surface theme/test_surface.c)
target_link_libraries(theme_surface PRIVATE trlv_tokens_template)

if(TRLV_TEST_FULL)
    trlv_add_test(layout_sizing layout/test_sizing.c)
    target_link_libraries(layout_sizing PRIVATE trlv_tokens_template)
    trlv_add_test(layout_fill_warning layout/test_fill_warning.c)
    target_link_libraries(layout_fill_warning PRIVATE trlv_tokens_template)
    trlv_add_test(layout_text layout/test_text.c)
    target_link_libraries(layout_text PRIVATE trlv_tokens_template)
    trlv_add_test(layout_spacer layout/test_spacer.c)
    target_link_libraries(layout_spacer PRIVATE trlv_tokens_template)
    trlv_add_test(layout_text_role layout/test_text_role.c)
    target_link_libraries(layout_text_role PRIVATE trlv_tokens_template)
    trlv_add_test(layout_replace_not_stack layout/test_replace_not_stack.c)
    target_link_libraries(layout_replace_not_stack PRIVATE trlv_tokens_template)
    trlv_add_test(layout_demo layout/test_demo.c)
    target_link_libraries(layout_demo PRIVATE trlv_tokens_template trlv_tokens_hmi trlv_demo)
    trlv_add_test(layout_readme_card layout/test_readme_card.c)
    target_link_libraries(layout_readme_card PRIVATE trlv_tokens_template)

    trlv_add_test(theme_theme theme/test_theme.c)
    target_link_libraries(theme_theme PRIVATE trlv_tokens_template)
    trlv_add_test(theme_states theme/test_states.c)
    target_link_libraries(theme_states PRIVATE trlv_tokens_template)
    trlv_add_test(theme_color_mode theme/test_color_mode.c)
    target_link_libraries(theme_color_mode PRIVATE trlv_tokens_template)
    trlv_add_test(theme_widgets theme/test_widgets.c)
    target_link_libraries(theme_widgets PRIVATE trlv_tokens_template)
    trlv_add_test(theme_surface_custom theme/test_surface_custom.c)
    target_link_libraries(theme_surface_custom PRIVATE trlv_tokens_template)
    trlv_add_test(theme_button_type theme/test_button_type.c)
    target_link_libraries(theme_button_type PRIVATE trlv_tokens_template)
    trlv_add_test(theme_apply_warnings theme/test_apply_warnings.c)
    target_link_libraries(theme_apply_warnings PRIVATE trlv_tokens_template)
endif()

# Config options: a copy of the library built with one option set, and a
# test linked against it alone (its own harness and template objects).
get_target_property(TRLV_SOURCES trelvis SOURCES)
list(TRANSFORM TRLV_SOURCES PREPEND ${PROJECT_SOURCE_DIR}/)
get_target_property(TRLV_INCLUDES trelvis INCLUDE_DIRECTORIES)
function(trlv_add_option_test name source library definition)
    if(NOT TARGET ${library})
        add_library(${library} STATIC ${TRLV_SOURCES})
        target_include_directories(${library} PUBLIC ${TRLV_INCLUDES})
        target_link_libraries(${library} PUBLIC ${TRLV_LVGL_TARGET})
        target_compile_definitions(${library} PUBLIC ${definition})
        target_compile_options(${library} PRIVATE ${TRLV_STRICT_WARNINGS})
        set_target_properties(${library} PROPERTIES
            C_STANDARD 99
            C_STANDARD_REQUIRED ON
            C_EXTENSIONS OFF)
    endif()
    add_executable(${name} ${source} harness/trlv_test.c ../templates/trlv_tokens_template.c)
    target_include_directories(${name} PRIVATE harness ${PROJECT_SOURCE_DIR}/templates)
    target_link_libraries(${name} PRIVATE ${library})
    target_compile_options(${name} PRIVATE ${TRLV_STRICT_WARNINGS})
    set_target_properties(${name} PROPERTIES
        C_STANDARD 99
        C_STANDARD_REQUIRED ON
        C_EXTENSIONS OFF)
    add_test(NAME ${name} COMMAND ${name})
endfunction()

trlv_add_option_test(layout_debug_outline_on layout/test_debug_outline.c
                     trelvis_debug_outline TRLV_USE_DEBUG_OUTLINE=1)
if(TRLV_TEST_FULL)
    trlv_add_option_test(theme_no_theme theme/test_no_theme.c trelvis_no_theme TRLV_USE_THEME=0)
    trlv_add_option_test(theme_surface_custom_max0 theme/test_surface_custom.c
                         trelvis_custom_color_max0 TRLV_CUSTOM_COLOR_MAX=0)
endif()

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
