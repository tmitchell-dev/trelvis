/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_set_*_fill warns when it would grow along the main axis of a parent that
 * hugs that axis (the child always collapses), and only then. Filling
 * across a hugging parent (100 %) works, so it doesn't warn.
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
    if(level == LV_LOG_LEVEL_WARN && strstr(buf, "no space to grow") != NULL) {
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
    lv_obj_t * obj = trlv_box_create(parent);
    lv_obj_set_size(obj, w, h);
    return obj;
}

/* Fill width along a row that hugs its width (grow): warns once, naming the
 * function, the object and the axis. */
static void warns_for_fill_along_a_hugging_row(lv_obj_t * screen)
{
    char obj_text[32];
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    lv_obj_t * filled = child(row, 10, 10);
    trlv_set_width_fill(filled);

    TRLV_ASSERT_INT(fill_warnings, 1);
    lv_snprintf(obj_text, sizeof(obj_text), "trlv_set_width_fill(%p)", (void *)filled);
    TRLV_ASSERT_INT(strstr(last_warning, obj_text) != NULL, true);
    TRLV_ASSERT_INT(strstr(last_warning, "hugs its width") != NULL, true);
}

/* Fill width across a column that hugs its width (100 %): no warning. The
 * child stretches to the widest sibling (see lvgl_result_for_fill_in_hug). */
static void quiet_for_fill_across_a_hugging_column(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_SM);
    trlv_set_width_fill(child(column, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 0);
}

/* The same in a hugging box without flex layout (100 %): no warning. */
static void quiet_for_fill_in_a_hugging_box(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    trlv_set_width_fill(child(box, 10, 10));
    trlv_set_height_fill(child(box, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 0);
}

/* Fill height in a column that hugs its height (grow): warns, naming the height. */
static void warns_for_fill_height(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_SM);
    lv_obj_set_width(column, 100);
    trlv_set_height_fill(child(column, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 1);
    TRLV_ASSERT_INT(strstr(last_warning, "hugs its height") != NULL, true);
}

/* No warning in a sized parent, flex or not. */
static void quiet_in_a_sized_parent(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_SM);
    lv_obj_set_size(column, 100, 100);
    trlv_set_width_fill(child(column, 10, 10));
    trlv_set_height_fill(child(column, 10, 10));

    lv_obj_t * box = trlv_box_create(screen);
    lv_obj_set_size(box, 100, 100);
    trlv_set_width_fill(child(box, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 0);
}

/* No warning when the parent hugs only the other axis: a column with a
 * width that hugs its height, filled across. */
static void quiet_when_parent_hugs_the_other_axis(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_SM);
    lv_obj_set_width(column, 100);
    trlv_set_width_fill(child(column, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 0);
}

/* No warning when the parent fills its own parent. */
static void quiet_when_parent_fills(lv_obj_t * screen)
{
    lv_obj_t * outer = trlv_column_create(setup(screen));
    trlv_set_gap(outer, TRLV_SPACE_SM);
    lv_obj_set_width(outer, 200);
    lv_obj_t * inner = trlv_column_create(outer);
    trlv_set_gap(inner, TRLV_SPACE_SM);
    trlv_set_width_fill(inner);
    trlv_set_width_fill(child(inner, 10, 10));

    TRLV_ASSERT_INT(fill_warnings, 0);
}

/* What LVGL does with fill across a hugging parent: a 100 % child doesn't
 * count towards its parent's content size, so the column hugs its other
 * children and the filled child takes that width: as wide as the widest
 * sibling (CSS align-self: stretch). With no sized sibling it would be 0. */
static void lvgl_result_for_fill_in_hug(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_SM);
    child(column, 80, 10);
    lv_obj_t * filled = trlv_box_create(column);     /* hugs a 10 x 10 child */
    child(filled, 10, 10);
    trlv_set_width_fill(filled);

    TRLV_ASSERT_WIDTH(column, 80);
    TRLV_ASSERT_WIDTH(filled, 80);
}

/* The same along the main axis is worse: a growing child doesn't count
 * towards a hugging row's width either, so there is no free space and it
 * collapses to zero width, its content hidden. Only the gap remains. */
static void lvgl_result_for_grow_in_hug(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    child(row, 80, 10);
    lv_obj_t * filled = trlv_box_create(row);        /* hugs a 10 x 10 child */
    child(filled, 10, 10);
    trlv_set_width_fill(filled);

    TRLV_ASSERT_WIDTH(row, 80 + 8);
    TRLV_ASSERT_WIDTH(filled, 0);
}

int main(void)
{
    TRLV_TEST_RUN(warns_for_fill_along_a_hugging_row);
    TRLV_TEST_RUN(quiet_for_fill_across_a_hugging_column);
    TRLV_TEST_RUN(quiet_for_fill_in_a_hugging_box);
    TRLV_TEST_RUN(warns_for_fill_height);
    TRLV_TEST_RUN(quiet_in_a_sized_parent);
    TRLV_TEST_RUN(quiet_when_parent_hugs_the_other_axis);
    TRLV_TEST_RUN(quiet_when_parent_fills);
    TRLV_TEST_RUN(lvgl_result_for_fill_in_hug);
    TRLV_TEST_RUN(lvgl_result_for_grow_in_hug);
    return trlv_test_finish();
}
