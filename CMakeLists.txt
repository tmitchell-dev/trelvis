# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Timothy Mitchell
#
# TreLVis library target. In your project (which provides LVGL):
#
#   add_subdirectory(external/lvgl)
#   add_subdirectory(external/trelvis)
#   target_link_libraries(my_app PRIVATE trelvis)
#
# Built on its own (cmake -B build), this also fetches LVGL and builds the
# development checks in tests/.

cmake_minimum_required(VERSION 3.16)

# The version is defined once, in trlv_version.h.
file(STRINGS "${CMAKE_CURRENT_LIST_DIR}/include/trelvis/trlv_version.h" _trlv_version
     REGEX "^#define TRLV_VERSION_(MAJOR|MINOR|PATCH) ")
string(REGEX REPLACE ".*MAJOR +([0-9]+).*MINOR +([0-9]+).*PATCH +([0-9]+).*" "\\1.\\2.\\3"
       _trlv_version "${_trlv_version}")

project(trelvis
    VERSION ${_trlv_version}
    DESCRIPTION "A light and versatile layout layer for LVGL"
    LANGUAGES C)

set(TRLV_LVGL_TARGET "lvgl" CACHE STRING "Name of the project's LVGL CMake target")

add_library(trelvis STATIC
    src/trlv_layout.c
    src/trlv_styles.c
    src/trlv_surface.c
    src/trlv_text.c
    src/trlv_theme.c
)
add_library(trelvis::trelvis ALIAS trelvis)

target_include_directories(trelvis PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(trelvis PUBLIC ${TRLV_LVGL_TARGET})
set_target_properties(trelvis PROPERTIES
    C_STANDARD 99
    C_STANDARD_REQUIRED ON
    C_EXTENSIONS OFF)

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    add_subdirectory(tests)
endif()
