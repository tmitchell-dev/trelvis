/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_test.h
 * Headless layout test harness: build an object tree, let LVGL lay it out,
 * assert positions and sizes. Plain C, no test framework.
 *
 * Each test is a function that builds onto the screen it is given and then
 * asserts. Every test runs on a fresh LVGL (lv_init / lv_deinit) with a
 * test-only display that renders nothing:
 *
 *     static void row_places_children(lv_obj_t * screen)
 *     {
 *         lv_obj_t * row = ...;
 *         TRLV_ASSERT_RECT(child, 16, 16, 40, 20);
 *     }
 *
 *     int main(void)
 *     {
 *         TRLV_TEST_RUN(row_places_children);
 *         return trlv_test_finish();
 *     }
 *
 * Positions are measured from the parent's outer top-left corner, the way
 * Figma's inspector shows them, so a parent's padding is visible in the
 * child's x and y. (lv_obj_get_x() measures from inside the padding.) The
 * assertions update the layout first, so tests never need to.
 *
 * A failed assertion prints file, line, test, expected and actual values,
 * and the test carries on so that every mismatch is reported.
 */

#ifndef TRLV_TEST_H
#define TRLV_TEST_H

#include <trelvis/trelvis.h>

#include <stdbool.h>

/** A test: builds onto `screen`, then asserts. */
typedef void (*trlv_test_fn_t)(lv_obj_t * screen);

/** Size of the test display for the tests that follow. Default 800 x 480. */
void trlv_test_set_display_size(int32_t width, int32_t height);

/** Run one test on a fresh LVGL. Use TRLV_TEST_RUN(fn) to name it after the function. */
void trlv_test_run(const char * name, trlv_test_fn_t fn);

/** Print a summary. Returns the exit code for main(): 0 if every test passed. */
int trlv_test_finish(void);

#define TRLV_TEST_RUN(fn) trlv_test_run(#fn, fn)

/*---------------------------------------------------------------------------
 * Measurements (the layout is updated first)
 *-------------------------------------------------------------------------*/

/** x from the parent's outer left edge (from the display's left edge for a screen). */
int32_t trlv_test_x(lv_obj_t * obj);
/** y from the parent's outer top edge (from the display's top edge for a screen). */
int32_t trlv_test_y(lv_obj_t * obj);
int32_t trlv_test_width(lv_obj_t * obj);
int32_t trlv_test_height(lv_obj_t * obj);

/*---------------------------------------------------------------------------
 * Assertions
 *-------------------------------------------------------------------------*/

/** Record an integer comparison; prints a message and fails the test on mismatch. */
bool trlv_test_check_int(const char * file, int line, const char * what,
                         int32_t expected, int32_t actual);

#define TRLV_ASSERT_INT(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (expected), (actual))

#define TRLV_ASSERT_X(obj, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #obj ".x", (expected), trlv_test_x(obj))
#define TRLV_ASSERT_Y(obj, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #obj ".y", (expected), trlv_test_y(obj))
#define TRLV_ASSERT_WIDTH(obj, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #obj ".width", (expected), trlv_test_width(obj))
#define TRLV_ASSERT_HEIGHT(obj, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #obj ".height", (expected), trlv_test_height(obj))

/** Position (from the parent's outer edge) and size in one go. */
#define TRLV_ASSERT_RECT(obj, x, y, width, height) \
    do {                                           \
        TRLV_ASSERT_X(obj, x);                     \
        TRLV_ASSERT_Y(obj, y);                     \
        TRLV_ASSERT_WIDTH(obj, width);             \
        TRLV_ASSERT_HEIGHT(obj, height);           \
    } while(0)

#endif /* TRLV_TEST_H */
