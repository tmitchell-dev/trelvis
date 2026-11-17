# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Timothy Mitchell
#
# A stand-in for a product that uses TreLVis as a git submodule, following
# the README's quick start. CI builds and runs it; it is not part of the
# TreLVis build.
#
#   cmake -S tests/consumer -B build-consumer -DLVGL_DIR=path/to/lvgl
#   cmake --build build-consumer
#   ctest --test-dir build-consumer --output-on-failure
#
# Options:
#   -DCONSUMER_CONF=include_path   trlv_conf.h next to lv_conf.h (default)
#   -DCONSUMER_CONF=conf_path      conf_path/product_trlv_conf.h via TRLV_CONF_PATH

cmake_minimum_required(VERSION 3.16)
project(trlv_consumer C)

set(LVGL_DIR "" CACHE PATH "LVGL 9.6 source tree")
set(CONSUMER_CONF "include_path" CACHE STRING "How trlv_conf.h is found: include_path or conf_path")
if(NOT EXISTS "${LVGL_DIR}/lvgl.h")
    message(FATAL_ERROR "Set LVGL_DIR to an LVGL 9.6 source tree")
endif()

# Where a product keeps the submodule (external/trelvis).
get_filename_component(TRELVIS_DIR ${CMAKE_CURRENT_LIST_DIR}/../.. ABSOLUTE)

# ---- LVGL, as the product's own target with its own name ----
# Many embedded projects compile LVGL's sources into their own target
# rather than use LVGL's CMake, so the target isn't called "lvgl".
file(GLOB_RECURSE LVGL_SOURCES ${LVGL_DIR}/src/*.c)
add_library(app_lvgl STATIC ${LVGL_SOURCES})
target_include_directories(app_lvgl PUBLIC
    ${LVGL_DIR}/include                     # <lvgl/lvgl.h>
    ${CMAKE_CURRENT_LIST_DIR}/config)       # lv_conf.h and trlv_conf.h
target_compile_definitions(app_lvgl PUBLIC LV_CONF_INCLUDE_SIMPLE)

# ---- TreLVis (README quick start, step 2) ----
set(TRLV_LVGL_TARGET app_lvgl)
add_subdirectory(${TRELVIS_DIR} trelvis)

if(CONSUMER_CONF STREQUAL "conf_path")
    target_compile_definitions(trelvis PUBLIC
        "TRLV_CONF_PATH=\"${CMAKE_CURRENT_LIST_DIR}/conf_path/product_trlv_conf.h\"")
    set(CONSUMER_EXPECT_CONF 2)
elseif(CONSUMER_CONF STREQUAL "include_path")
    set(CONSUMER_EXPECT_CONF 1)
else()
    message(FATAL_ERROR "CONSUMER_CONF must be include_path or conf_path")
endif()

# ---- The copied token template (step 3) ----
file(READ ${TRELVIS_DIR}/templates/trlv_tokens_template.c TOKENS_C)
string(REPLACE "#include \"trlv_tokens_template.h\"" "#include \"tokens.h\"" TOKENS_C "${TOKENS_C}")
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/tokens.c "${TOKENS_C}")
configure_file(${TRELVIS_DIR}/templates/trlv_tokens_template.h ${CMAKE_CURRENT_BINARY_DIR}/tokens.h COPYONLY)

# ---- The application: trlv_init and the demo, headless (steps 4 and 5) ----
file(GLOB DEMO_SOURCES ${TRELVIS_DIR}/examples/demo/*.c)
add_executable(consumer main.c ${CMAKE_CURRENT_BINARY_DIR}/tokens.c ${DEMO_SOURCES})
target_include_directories(consumer PRIVATE ${CMAKE_CURRENT_BINARY_DIR} ${TRELVIS_DIR}/examples/demo)
target_compile_definitions(consumer PRIVATE CONSUMER_EXPECT_CONF=${CONSUMER_EXPECT_CONF})
target_link_libraries(consumer PRIVATE trelvis)

enable_testing()
add_test(NAME consumer COMMAND consumer)
