/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

#include "trlv_test.h"

#include <stdio.h>

/* Draw buffer for the test display. Nothing is rendered (layout tests never
 * run lv_timer_handler), but a display needs buffers. Partial mode, so this
 * only has to hold a few lines at the largest colour format. uint32_t keeps
 * it aligned for LV_DRAW_BUF_ALIGN's default of 4. */
#define TEST_BUF_LINES 10
#define TEST_BUF_MAX_WIDTH 1024
static uint32_t test_buf[TEST_BUF_MAX_WIDTH * TEST_BUF_LINES];   /* 4 bytes per pixel */

static int32_t disp_width = 800;
static int32_t disp_height = 480;

static const char * current_test;
static bool current_failed;
static int tests_run;
static int tests_failed;

static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    (void)area;
    (void)px_map;
    lv_display_flush_ready(disp);
}

void trlv_test_set_display_size(int32_t width, int32_t height)
{
    if(width > TEST_BUF_MAX_WIDTH) {
        printf("trlv_test: display width %d is over the harness maximum %d\n",
               (int)width, TEST_BUF_MAX_WIDTH);
        width = TEST_BUF_MAX_WIDTH;
    }
    disp_width = width;
    disp_height = height;
}

void trlv_test_run(const char * name, trlv_test_fn_t fn)
{
    current_test = name;
    current_failed = false;
    tests_run++;

    lv_init();
    lv_display_t * disp = lv_display_create(disp_width, disp_height);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, test_buf, NULL, sizeof(test_buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    fn(lv_display_get_screen_active(disp));

    lv_deinit();

    if(current_failed) tests_failed++;
    printf("%s %s\n", current_failed ? "FAIL" : "ok  ", name);
    current_test = NULL;
}

int trlv_test_finish(void)
{
    if(tests_failed) printf("FAILED: %d of %d tests\n", tests_failed, tests_run);
    else printf("passed: %d tests\n", tests_run);
    return tests_failed ? 1 : 0;
}

/*---------------------------------------------------------------------------
 * Measurements
 *-------------------------------------------------------------------------*/

/* Distance of obj's outer edge from its parent's outer edge, ignoring the
 * parent's scroll position: the layout position, as Figma shows it. */
static void outer_offset(lv_obj_t * obj, int32_t * x, int32_t * y)
{
    lv_area_t coords;

    lv_obj_update_layout(obj);
    lv_obj_get_coords(obj, &coords);

    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(parent == NULL) {
        *x = coords.x1;
        *y = coords.y1;
        return;
    }

    lv_area_t parent_coords;
    lv_obj_get_coords(parent, &parent_coords);
    *x = coords.x1 - parent_coords.x1 + lv_obj_get_scroll_x(parent);
    *y = coords.y1 - parent_coords.y1 + lv_obj_get_scroll_y(parent);
}

int32_t trlv_test_x(lv_obj_t * obj)
{
    int32_t x;
    int32_t y;
    outer_offset(obj, &x, &y);
    return x;
}

int32_t trlv_test_y(lv_obj_t * obj)
{
    int32_t x;
    int32_t y;
    outer_offset(obj, &x, &y);
    return y;
}

int32_t trlv_test_width(lv_obj_t * obj)
{
    lv_obj_update_layout(obj);
    return lv_obj_get_width(obj);
}

int32_t trlv_test_height(lv_obj_t * obj)
{
    lv_obj_update_layout(obj);
    return lv_obj_get_height(obj);
}

/*---------------------------------------------------------------------------
 * Assertions
 *-------------------------------------------------------------------------*/

bool trlv_test_check_int(const char * file, int line, const char * what,
                         int32_t expected, int32_t actual)
{
    if(expected == actual) return true;

    printf("%s:%d: in %s: %s expected %d, got %d\n", file, line,
           current_test ? current_test : "(no test)", what, (int)expected, (int)actual);
    current_failed = true;
    return false;
}
