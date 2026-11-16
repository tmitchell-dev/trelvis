/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Spacing modifiers: trlv_gap and trlv_pad, with the token template's
 * Regular values (SM = 8, MD = 12, LG = 16).
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    return screen;
}

/* A 10 x 10 child. */
static lv_obj_t * child(lv_obj_t * parent)
{
    lv_obj_t * obj = trlv_box(parent);
    lv_obj_set_size(obj, 10, 10);
    return obj;
}

/* A box with flex flow `flow` and two 10 x 10 children. */
static lv_obj_t * flex_box(lv_obj_t * parent, lv_flex_flow_t flow, lv_obj_t ** a, lv_obj_t ** b)
{
    lv_obj_t * box = trlv_box(parent);
    lv_obj_set_flex_flow(box, flow);
    *a = child(box);
    *b = child(box);
    return box;
}

/*---------------------------------------------------------------------------
 * trlv_gap
 *-------------------------------------------------------------------------*/

/* The gap between children in a row is the token value. */
static void gap_between_row_children(lv_obj_t * screen)
{
    lv_obj_t * a;
    lv_obj_t * b;
    lv_obj_t * box = flex_box(setup(screen), LV_FLEX_FLOW_ROW, &a, &b);
    trlv_gap(box, TRLV_SPACE_MD);

    TRLV_ASSERT_X(a, 0);
    TRLV_ASSERT_X(b, 10 + 12);
    TRLV_ASSERT_WIDTH(box, 10 + 12 + 10);
}

/* The gap between children in a column is the token value. */
static void gap_between_column_children(lv_obj_t * screen)
{
    lv_obj_t * a;
    lv_obj_t * b;
    lv_obj_t * box = flex_box(setup(screen), LV_FLEX_FLOW_COLUMN, &a, &b);
    trlv_gap(box, TRLV_SPACE_LG);

    TRLV_ASSERT_Y(b, 10 + 16);
    TRLV_ASSERT_HEIGHT(box, 10 + 16 + 10);
}

/* A second call replaces the first, and back again. */
static void second_gap_replaces_first(lv_obj_t * screen)
{
    lv_obj_t * a;
    lv_obj_t * b;
    lv_obj_t * box = flex_box(setup(screen), LV_FLEX_FLOW_ROW, &a, &b);

    trlv_gap(box, TRLV_SPACE_LG);
    trlv_gap(box, TRLV_SPACE_SM);
    TRLV_ASSERT_X(b, 10 + 8);

    trlv_gap(box, TRLV_SPACE_NONE);
    TRLV_ASSERT_X(b, 10);
}

/* Works on a themed lv_obj that TreLVis did not create: the token value
 * wins over the theme's gap. */
static void gap_on_a_plain_lv_obj(lv_obj_t * screen)
{
    lv_obj_t * obj = lv_obj_create(setup(screen));
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    trlv_gap(obj, TRLV_SPACE_SM);

    TRLV_ASSERT_INT(lv_obj_get_style_pad_column(obj, LV_PART_MAIN), 8);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_row(obj, LV_PART_MAIN), 8);
}

/* An out-of-range step is ignored. */
static void gap_out_of_range_is_ignored(lv_obj_t * screen)
{
    lv_obj_t * a;
    lv_obj_t * b;
    lv_obj_t * box = flex_box(setup(screen), LV_FLEX_FLOW_ROW, &a, &b);
    trlv_gap(box, TRLV_SPACE_SM);
    trlv_gap(box, TRLV_SPACE_COUNT);

    TRLV_ASSERT_X(b, 10 + 8);
}

/*---------------------------------------------------------------------------
 * Token styles and trlv_init
 *-------------------------------------------------------------------------*/

/* trlv_init with other layout tokens updates objects already using them. */
static void init_again_updates_existing_objects(lv_obj_t * screen)
{
    static trlv_layout_tokens_t wider;
    lv_obj_t * a;
    lv_obj_t * b;
    lv_obj_t * box = flex_box(setup(screen), LV_FLEX_FLOW_ROW, &a, &b);
    trlv_gap(box, TRLV_SPACE_MD);
    TRLV_ASSERT_X(b, 10 + 12);

    wider = tokens_regular;
    wider.space[TRLV_SPACE_MD] = 20;
    trlv_init(lv_obj_get_display(screen), &tokens_light, &wider);
    TRLV_ASSERT_X(b, 10 + 20);
}

/* A token style applied before trlv_init is empty, and takes its value when
 * trlv_init runs. (Each test runs on a fresh LVGL, so this also checks that
 * the styles work again after lv_deinit and lv_init.) */
static void modifier_before_init_takes_value_at_init(lv_obj_t * screen)
{
    lv_obj_t * a;
    lv_obj_t * b;
    lv_obj_t * box = flex_box(screen, LV_FLEX_FLOW_ROW, &a, &b);
    trlv_gap(box, TRLV_SPACE_MD);

    setup(screen);
    TRLV_ASSERT_X(b, 10 + 12);
}

int main(void)
{
    TRLV_TEST_RUN(gap_between_row_children);
    TRLV_TEST_RUN(gap_between_column_children);
    TRLV_TEST_RUN(second_gap_replaces_first);
    TRLV_TEST_RUN(gap_on_a_plain_lv_obj);
    TRLV_TEST_RUN(gap_out_of_range_is_ignored);
    TRLV_TEST_RUN(init_again_updates_existing_objects);
    TRLV_TEST_RUN(modifier_before_init_takes_value_at_init);
    return trlv_test_finish();
}
