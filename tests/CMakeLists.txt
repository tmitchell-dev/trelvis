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

# ---- Compile-only: the public headers must work from C++17 ----
enable_language(CXX)
add_library(trlv_cpp_include_test OBJECT cpp_include_test.cpp)
target_link_libraries(trlv_cpp_include_test PRIVATE trelvis)
target_compile_options(trlv_cpp_include_test PRIVATE ${TRLV_STRICT_WARNINGS})
set_target_properties(trlv_cpp_include_test PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF)
