/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_fill_* warns when the parent hugs on the same axis, and only then.
 * The test build turns LVGL logging on; warnings are caught with a log
 * callback.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#include <string.h>

static int fill_warnings;
static char last_warning[256];

static void log_cb(lv_log_level_t level, const char * buf)
{
    if(level == LV_LOG_LEVEL_WARN && strstr(buf, "trlv_fill_") != NULL) {
        fill_warnings++;
        lv_strlcpy(last_warning, buf, sizeof(last_warning));
    }
}

/* lv_deinit clears the log callback, so it is registered in every test. */
static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    fill_warnings = 0;
    last_warning[0] = '\0';
    lv_log_register_print_cb(log_cb);
    return screen;
}

static lv_obj_t * child(lv_obj_t * parent, int32_t w, int32_t h)
{
    lv_obj_t * obj = trlv_box(parent);
    lv_obj_set_size(obj, w, h);
    return obj;
}

/* Fill width across a column that hugs its width: warns once, naming the
 * function, the object and the axis. */
static void warns_for_fill_across_a_hugging_column(lv_obj_t * screen)
{
    char obj_text[32];
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_t * filled = child(column, 10, 10);
    trlv_fill_width(filled);

    TRLV_ASSERT_INT(fill_warnings, 1);
    lv_snprintf(obj_text, sizeof(obj_text), "trlv_fill_width(%p)", (void *)filled);
    TRLV_ASSERT_INT(strstr(last_warning, obj_text) != NULL, true);
    TRLV_ASSERT_INT(strstr(last_warning, "hugs its width") != NULL, true);
}

/* Fill width along a row that hugs its width (grow): warns. */
static void warns_for_fill_along_a_hugging_row(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row(setup(screen), TRLV_SPACE_SM);
    trlv_fill_width(child(row, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 1);
}

/* Fill height in a column that hugs its height: warns, naming the height. */
static void warns_for_fill_height(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_width(column, 100);
    trlv_fill_height(child(column, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 1);
    TRLV_ASSERT_INT(strstr(last_warning, "hugs its height") != NULL, true);
}

/* No warning in a sized parent, flex or not. */
static void quiet_in_a_sized_parent(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_size(column, 100, 100);
    trlv_fill_width(child(column, 10, 10));
    trlv_fill_height(child(column, 10, 10));

    lv_obj_t * box = trlv_box(screen);
    lv_obj_set_size(box, 100, 100);
    trlv_fill_width(child(box, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 0);
}

/* No warning when the parent hugs only the other axis: a column with a
 * width that hugs its height, filled across. */
static void quiet_when_parent_hugs_the_other_axis(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_width(column, 100);
    trlv_fill_width(child(column, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 0);
}

/* No warning when the parent fills its own parent. */
static void quiet_when_parent_fills(lv_obj_t * screen)
{
    lv_obj_t * outer = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_width(outer, 200);
    lv_obj_t * inner = trlv_column(outer, TRLV_SPACE_SM);
    trlv_fill_width(inner);
    trlv_fill_width(child(inner, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 0);
}

/* What LVGL does with fill inside hug, for the record (ROADMAP "To confirm
 * in the slice"). A 100 % child doesn't count towards its parent's content
 * size, so the column hugs its other children and the filled child takes
 * that width: as wide as the widest sibling. */
static void lvgl_result_for_fill_in_hug(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    child(column, 80, 10);
    lv_obj_t * filled = trlv_box(column);     /* hugs a 10 x 10 child */
    child(filled, 10, 10);
    trlv_fill_width(filled);

    TRLV_ASSERT_WIDTH(column, 80);
    TRLV_ASSERT_WIDTH(filled, 80);
}

/* The same along the main axis is worse: a growing child doesn't count
 * towards a hugging row's width either, so there is no free space and it
 * collapses to zero width, its content hidden. Only the gap remains. */
static void lvgl_result_for_grow_in_hug(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row(setup(screen), TRLV_SPACE_SM);
    child(row, 80, 10);
    lv_obj_t * filled = trlv_box(row);        /* hugs a 10 x 10 child */
    child(filled, 10, 10);
    trlv_fill_width(filled);

    TRLV_ASSERT_WIDTH(row, 80 + 8);
    TRLV_ASSERT_WIDTH(filled, 0);
}

int main(void)
{
    TRLV_TEST_RUN(warns_for_fill_across_a_hugging_column);
    TRLV_TEST_RUN(warns_for_fill_along_a_hugging_row);
    TRLV_TEST_RUN(warns_for_fill_height);
    TRLV_TEST_RUN(quiet_in_a_sized_parent);
    TRLV_TEST_RUN(quiet_when_parent_hugs_the_other_axis);
    TRLV_TEST_RUN(quiet_when_parent_fills);
    TRLV_TEST_RUN(lvgl_result_for_fill_in_hug);
    TRLV_TEST_RUN(lvgl_result_for_grow_in_hug);
    return trlv_test_finish();
}
