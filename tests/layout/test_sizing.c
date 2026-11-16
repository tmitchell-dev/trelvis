/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Sizing modifiers: trlv_hug_* and axis-aware trlv_fill_*, with the token
 * template's Regular values (SM = 8).
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
    lv_obj_t * obj = trlv_box(parent);
    lv_obj_set_size(obj, w, h);
    return obj;
}

/* A box that hugs a 30 x 20 child, so its hug size is known. */
static lv_obj_t * hugging_box(lv_obj_t * parent)
{
    lv_obj_t * obj = trlv_box(parent);
    child(obj, 30, 20);
    return obj;
}

/* In a row, fill width grows into the free space and fill height is 100 %. */
static void fill_in_a_row(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_size(row, 200, 50);
    child(row, 40, 10);
    lv_obj_t * filled = hugging_box(row);
    trlv_fill_width(filled);
    trlv_fill_height(filled);

    TRLV_ASSERT_RECT(filled, 40 + 8, 0, 200 - 40 - 8, 50);
}

/* In a column, fill height grows into the free space and fill width is 100 %. */
static void fill_in_a_column(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_size(column, 100, 300);
    child(column, 10, 40);
    lv_obj_t * filled = hugging_box(column);
    trlv_fill_width(filled);
    trlv_fill_height(filled);

    TRLV_ASSERT_RECT(filled, 0, 40 + 8, 100, 300 - 40 - 8);
}

/* Without a flex parent, fill is 100 % on both axes. */
static void fill_in_a_non_flex_parent(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box(setup(screen));
    lv_obj_set_size(box, 200, 100);
    lv_obj_t * filled = hugging_box(box);
    trlv_fill_width(filled);
    trlv_fill_height(filled);

    TRLV_ASSERT_RECT(filled, 0, 0, 200, 100);
}

/* Fill is measured inside the parent's padding, as in Figma. */
static void fill_respects_parent_padding(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_size(column, 100, 100);
    trlv_pad(column, TRLV_SPACE_LG);
    lv_obj_t * filled = hugging_box(column);
    trlv_fill_width(filled);

    TRLV_ASSERT_RECT(filled, 16, 16, 100 - 2 * 16, 20);
}

/* Two filled siblings share the free space equally. */
static void filled_siblings_share_the_space(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_size(row, 208, 20);
    lv_obj_t * a = hugging_box(row);
    lv_obj_t * b = hugging_box(row);
    trlv_fill_width(a);
    trlv_fill_width(b);

    TRLV_ASSERT_RECT(a, 0, 0, 100, 20);
    TRLV_ASSERT_RECT(b, 100 + 8, 0, 100, 20);
}

/* Hug clears an earlier fill on the same axis, in both cases (grow and 100 %). */
static void hug_clears_an_earlier_fill(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_size(row, 200, 50);
    lv_obj_t * filled = hugging_box(row);
    trlv_fill_width(filled);
    trlv_fill_height(filled);
    trlv_hug_width(filled);
    trlv_hug_height(filled);

    TRLV_ASSERT_RECT(filled, 0, 0, 30, 20);
}

/* Fill clears an earlier hug. */
static void fill_clears_an_earlier_hug(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box(setup(screen));
    lv_obj_set_size(box, 200, 100);
    lv_obj_t * filled = hugging_box(box);
    trlv_hug_width(filled);
    trlv_fill_width(filled);

    TRLV_ASSERT_WIDTH(filled, 200);
}

/* Changing one axis keeps the other: hug height after fill width in a row
 * must not stop the width from growing. */
static void axes_are_independent(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_size(row, 200, 50);
    lv_obj_t * filled = hugging_box(row);
    trlv_fill_width(filled);
    trlv_fill_height(filled);
    trlv_hug_height(filled);

    TRLV_ASSERT_RECT(filled, 0, 0, 200, 20);
}

/* The choice is made at call time: after the parent changes direction,
 * calling fill again re-decides. */
static void fill_again_after_direction_change(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box(setup(screen));
    lv_obj_set_size(box, 200, 100);
    lv_obj_t * filled = hugging_box(box);
    trlv_fill_width(filled);                          /* no flex: 100 % */
    lv_obj_set_flex_flow(box, LV_FLEX_FLOW_ROW);
    child(box, 40, 10);
    trlv_fill_width(filled);                          /* row: grow */

    TRLV_ASSERT_WIDTH(filled, 200 - 40);
}

/* Fill works on an LVGL widget TreLVis did not create. */
static void fill_on_a_widget(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_width(column, 240);
    lv_obj_t * bar = lv_bar_create(column);
    trlv_fill_width(bar);

    TRLV_ASSERT_WIDTH(bar, 240);
}

int main(void)
{
    TRLV_TEST_RUN(fill_in_a_row);
    TRLV_TEST_RUN(fill_in_a_column);
    TRLV_TEST_RUN(fill_in_a_non_flex_parent);
    TRLV_TEST_RUN(fill_respects_parent_padding);
    TRLV_TEST_RUN(filled_siblings_share_the_space);
    TRLV_TEST_RUN(hug_clears_an_earlier_fill);
    TRLV_TEST_RUN(fill_clears_an_earlier_hug);
    TRLV_TEST_RUN(axes_are_independent);
    TRLV_TEST_RUN(fill_again_after_direction_change);
    TRLV_TEST_RUN(fill_on_a_widget);
    return trlv_test_finish();
}
