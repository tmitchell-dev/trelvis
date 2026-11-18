/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_row_create and trlv_column_create: Figma auto layout → and ↓, with the
 * token template's Regular values (SM = 8, MD = 12, LG = 16).
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    return screen;
}

static lv_obj_t * child(lv_obj_t * parent, int32_t w, int32_t h)
{
    lv_obj_t * obj = trlv_box_create(parent);
    lv_obj_set_size(obj, w, h);
    return obj;
}

/* Children go left to right with the gap; the row hugs them, as tall as
 * the tallest child. */
static void row_places_children_left_to_right(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    lv_obj_t * a = child(row, 40, 20);
    lv_obj_t * b = child(row, 30, 10);
    lv_obj_t * c = child(row, 20, 30);

    TRLV_ASSERT_RECT(a, 0, 0, 40, 20);
    TRLV_ASSERT_RECT(b, 40 + 8, 0, 30, 10);
    TRLV_ASSERT_RECT(c, 40 + 8 + 30 + 8, 0, 20, 30);
    TRLV_ASSERT_RECT(row, 0, 0, 40 + 8 + 30 + 8 + 20, 30);
}

/* Children go top to bottom with the gap; the column hugs them, as wide as
 * the widest child. */
static void column_places_children_top_to_bottom(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_MD);
    lv_obj_t * a = child(column, 40, 20);
    lv_obj_t * b = child(column, 60, 10);

    TRLV_ASSERT_RECT(a, 0, 0, 40, 20);
    TRLV_ASSERT_RECT(b, 0, 20 + 12, 60, 10);
    TRLV_ASSERT_RECT(column, 0, 0, 60, 20 + 12 + 10);
}

/* Rows nest in columns with padding, as in a Figma auto-layout frame. */
static void rows_nest_in_a_padded_column(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_SM);
    trlv_set_pad(column, TRLV_SPACE_LG);
    lv_obj_t * row1 = trlv_row_create(column);
    trlv_set_gap(row1, TRLV_SPACE_SM);
    child(row1, 10, 10);
    child(row1, 10, 10);
    lv_obj_t * row2 = trlv_row_create(column);
    trlv_set_gap(row2, TRLV_SPACE_SM);
    child(row2, 50, 20);

    TRLV_ASSERT_RECT(row1, 16, 16, 10 + 8 + 10, 10);
    TRLV_ASSERT_RECT(row2, 16, 16 + 10 + 8, 50, 20);
    TRLV_ASSERT_RECT(column, 0, 0, 16 + 50 + 16, 16 + 10 + 8 + 20 + 16);
}

/* Rows and columns are boxes: not scrollable or clickable, no theme background. */
static void row_and_column_are_boxes(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    lv_obj_t * column = trlv_column_create(screen);
    trlv_set_gap(column, TRLV_SPACE_SM);

    TRLV_ASSERT_INT(lv_obj_is_scrollable(row), false);
    TRLV_ASSERT_INT(lv_obj_is_clickable(row), false);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(row, LV_PART_MAIN), LV_OPA_TRANSP);
    TRLV_ASSERT_INT(lv_obj_is_scrollable(column), false);
    TRLV_ASSERT_INT(lv_obj_is_clickable(column), false);
}

/* A new row or column starts like a fresh Figma frame: no gap and no
 * padding, even inside a themed parent. */
static void row_and_column_start_with_no_gap_or_padding(lv_obj_t * screen)
{
    lv_obj_t * parent = lv_obj_create(setup(screen));
    lv_obj_t * row = trlv_row_create(parent);
    lv_obj_t * row_a = child(row, 10, 10);
    lv_obj_t * row_b = child(row, 10, 10);
    lv_obj_t * column = trlv_column_create(parent);
    child(column, 10, 10);
    lv_obj_t * column_b = child(column, 10, 10);

    TRLV_ASSERT_X(row_a, 0);
    TRLV_ASSERT_X(row_b, 10);
    TRLV_ASSERT_Y(column_b, 10);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_column(row, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_row(column, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(column, LV_PART_MAIN), 0);
}

/* trlv_set_gap on a row replaces the gap it had. */
static void gap_changes_after_creation(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    child(row, 10, 10);
    lv_obj_t * b = child(row, 10, 10);
    trlv_set_gap(row, TRLV_SPACE_LG);

    TRLV_ASSERT_X(b, 10 + 16);
}

/* Raw LVGL wrap works on a row, as the mapping table says. */
static void row_wraps_with_lvgl(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    lv_obj_set_width(row, 50);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
    child(row, 20, 10);
    child(row, 20, 10);
    lv_obj_t * c = child(row, 20, 10);

    TRLV_ASSERT_RECT(c, 0, 10 + 8, 20, 10);
    TRLV_ASSERT_HEIGHT(row, 10 + 8 + 10);
}

int main(void)
{
    TRLV_TEST_RUN(row_places_children_left_to_right);
    TRLV_TEST_RUN(column_places_children_top_to_bottom);
    TRLV_TEST_RUN(rows_nest_in_a_padded_column);
    TRLV_TEST_RUN(row_and_column_are_boxes);
    TRLV_TEST_RUN(row_and_column_start_with_no_gap_or_padding);
    TRLV_TEST_RUN(gap_changes_after_creation);
    TRLV_TEST_RUN(row_wraps_with_lvgl);
    return trlv_test_finish();
}
