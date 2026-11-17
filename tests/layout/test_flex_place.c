/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Flex placement modifiers: trlv_justify (main axis) and trlv_align_items
 * (cross axis), with the token template's Regular values (SM = 8).
 *
 * Most tests use a 180 px row with three 20 px children and no gap, so the
 * free space is 120 px and every mode lands on whole pixels.
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

/* A 180 x 20 row with no gap and three 20 x 10 children. */
static lv_obj_t * three_in_a_row(lv_obj_t * screen, lv_obj_t * out[3])
{
    lv_obj_t * row = trlv_row(setup(screen), TRLV_SPACE_NONE);
    lv_obj_set_size(row, 180, 20);
    for(int i = 0; i < 3; i++) out[i] = child(row, 20, 10);
    return row;
}

static void assert_x(lv_obj_t * items[3], int32_t x0, int32_t x1, int32_t x2)
{
    TRLV_ASSERT_X(items[0], x0);
    TRLV_ASSERT_X(items[1], x1);
    TRLV_ASSERT_X(items[2], x2);
}

/*---------------------------------------------------------------------------
 * trlv_justify
 *-------------------------------------------------------------------------*/

static void justify_start(lv_obj_t * screen)
{
    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_justify(row, TRLV_JUSTIFY_START);
    assert_x(items, 0, 20, 40);
}

static void justify_center(lv_obj_t * screen)
{
    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_justify(row, TRLV_JUSTIFY_CENTER);
    assert_x(items, 60, 80, 100);
}

static void justify_end(lv_obj_t * screen)
{
    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_justify(row, TRLV_JUSTIFY_END);
    assert_x(items, 120, 140, 160);
}

static void justify_space_between(lv_obj_t * screen)
{
    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_justify(row, TRLV_JUSTIFY_SPACE_BETWEEN);
    assert_x(items, 0, 80, 160);
}

/* 120 / 3 = 40 around each child: 20 before the first, 40 between. */
static void justify_space_around(lv_obj_t * screen)
{
    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_justify(row, TRLV_JUSTIFY_SPACE_AROUND);
    assert_x(items, 20, 80, 140);
}

/* 120 / 4 = 30 at each edge and between. */
static void justify_space_evenly(lv_obj_t * screen)
{
    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_justify(row, TRLV_JUSTIFY_SPACE_EVENLY);
    assert_x(items, 30, 80, 130);
}

/* SPACE_* spacing comes on top of the gap: free space 180 - 60 - 2 * 8 = 104,
 * 104 / 4 = 26. */
static void justify_spacing_adds_to_the_gap(lv_obj_t * screen)
{
    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_gap(row, TRLV_SPACE_SM);
    trlv_justify(row, TRLV_JUSTIFY_SPACE_EVENLY);
    assert_x(items, 26, 26 + 20 + 8 + 26, 180 - 26 - 20);
}

/* In a column, justify works vertically. */
static void justify_in_a_column(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_NONE);
    lv_obj_set_size(column, 20, 100);
    lv_obj_t * a = child(column, 10, 20);
    lv_obj_t * b = child(column, 10, 20);
    trlv_justify(column, TRLV_JUSTIFY_END);

    TRLV_ASSERT_Y(a, 60);
    TRLV_ASSERT_Y(b, 80);
}

/* A second call replaces the first in place: a project style added after
 * the modifier still wins when the modifier is called again. */
static void justify_replaces_its_previous_style(lv_obj_t * screen)
{
    static const lv_style_const_prop_t project_props[] = {
        LV_STYLE_CONST_FLEX_MAIN_PLACE(LV_FLEX_ALIGN_CENTER),
        LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(project_style, project_props);

    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_justify(row, TRLV_JUSTIFY_END);
    lv_obj_add_style(row, &project_style, LV_PART_MAIN);
    trlv_justify(row, TRLV_JUSTIFY_START);
    assert_x(items, 60, 80, 100);

    lv_obj_remove_style(row, &project_style, LV_PART_MAIN);
    assert_x(items, 0, 20, 40);
}

/* Works on a flex lv_obj TreLVis did not create. */
static void justify_on_a_plain_object(lv_obj_t * screen)
{
    lv_obj_t * obj = lv_obj_create(setup(screen));
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 180, 20);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_t * items[3];
    for(int i = 0; i < 3; i++) items[i] = child(obj, 20, 10);
    trlv_justify(obj, TRLV_JUSTIFY_SPACE_BETWEEN);

    assert_x(items, 0, 80, 160);
}

/* A value outside the enum changes nothing. */
static void justify_ignores_an_invalid_value(lv_obj_t * screen)
{
    lv_obj_t * items[3];
    lv_obj_t * row = three_in_a_row(screen, items);
    trlv_justify(row, TRLV_JUSTIFY_END);
    trlv_justify(row, (trlv_justify_t)99);
    assert_x(items, 120, 140, 160);
}

/*---------------------------------------------------------------------------
 * trlv_align_items
 *-------------------------------------------------------------------------*/

/* A 100 x 50 row with a 20 x 10 and a 20 x 30 child. */
static lv_obj_t * tall_row(lv_obj_t * screen, lv_obj_t ** small, lv_obj_t ** big)
{
    lv_obj_t * row = trlv_row(setup(screen), TRLV_SPACE_NONE);
    lv_obj_set_size(row, 100, 50);
    *small = child(row, 20, 10);
    *big = child(row, 20, 30);
    return row;
}

/* A 100 x 50 column with a 20 x 10 and a 40 x 10 child. */
static lv_obj_t * wide_column(lv_obj_t * screen, lv_obj_t ** small, lv_obj_t ** big)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_NONE);
    lv_obj_set_size(column, 100, 50);
    *small = child(column, 20, 10);
    *big = child(column, 40, 10);
    return column;
}

static void align_items_in_a_row(lv_obj_t * screen)
{
    lv_obj_t * small;
    lv_obj_t * big;
    lv_obj_t * row = tall_row(screen, &small, &big);

    trlv_align_items(row, TRLV_ALIGN_ITEMS_START);
    TRLV_ASSERT_Y(small, 0);
    TRLV_ASSERT_Y(big, 0);

    trlv_align_items(row, TRLV_ALIGN_ITEMS_CENTER);
    TRLV_ASSERT_Y(small, 20);
    TRLV_ASSERT_Y(big, 10);

    trlv_align_items(row, TRLV_ALIGN_ITEMS_END);
    TRLV_ASSERT_Y(small, 40);
    TRLV_ASSERT_Y(big, 20);
}

static void align_items_in_a_column(lv_obj_t * screen)
{
    lv_obj_t * small;
    lv_obj_t * big;
    lv_obj_t * column = wide_column(screen, &small, &big);

    trlv_align_items(column, TRLV_ALIGN_ITEMS_START);
    TRLV_ASSERT_X(small, 0);
    TRLV_ASSERT_X(big, 0);

    trlv_align_items(column, TRLV_ALIGN_ITEMS_CENTER);
    TRLV_ASSERT_X(small, 40);
    TRLV_ASSERT_X(big, 30);

    trlv_align_items(column, TRLV_ALIGN_ITEMS_END);
    TRLV_ASSERT_X(small, 80);
    TRLV_ASSERT_X(big, 60);
}

/* Replaces in place, like trlv_justify. The project style sets both
 * properties, as trlv_align_items does. */
static void align_items_replaces_its_previous_style(lv_obj_t * screen)
{
    static const lv_style_const_prop_t project_props[] = {
        LV_STYLE_CONST_FLEX_CROSS_PLACE(LV_FLEX_ALIGN_CENTER),
        LV_STYLE_CONST_FLEX_TRACK_PLACE(LV_FLEX_ALIGN_CENTER),
        LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(project_style, project_props);

    lv_obj_t * small;
    lv_obj_t * big;
    lv_obj_t * row = tall_row(screen, &small, &big);
    trlv_align_items(row, TRLV_ALIGN_ITEMS_END);
    lv_obj_add_style(row, &project_style, LV_PART_MAIN);
    trlv_align_items(row, TRLV_ALIGN_ITEMS_START);
    TRLV_ASSERT_Y(small, 20);

    lv_obj_remove_style(row, &project_style, LV_PART_MAIN);
    TRLV_ASSERT_Y(small, 0);
}

/* A local flex_track_place (CSS align-content) takes precedence over the
 * line placement trlv_align_items sets: items centre within the line only. */
static void align_items_line_placement_can_be_overridden(lv_obj_t * screen)
{
    lv_obj_t * small;
    lv_obj_t * big;
    lv_obj_t * row = tall_row(screen, &small, &big);
    trlv_align_items(row, TRLV_ALIGN_ITEMS_CENTER);
    lv_obj_set_style_flex_track_place(row, LV_FLEX_ALIGN_START, LV_PART_MAIN);

    TRLV_ASSERT_Y(small, 10);
    TRLV_ASSERT_Y(big, 0);
}

/* Works on a flex lv_obj TreLVis did not create. */
static void align_items_on_a_plain_object(lv_obj_t * screen)
{
    lv_obj_t * obj = lv_obj_create(setup(screen));
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 100, 50);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    lv_obj_t * small = child(obj, 20, 10);
    child(obj, 20, 30);
    trlv_align_items(obj, TRLV_ALIGN_ITEMS_END);

    TRLV_ASSERT_Y(small, 40);
}

/* Justify and align items are separate groups: one doesn't clear the other. */
static void justify_and_align_items_are_independent(lv_obj_t * screen)
{
    lv_obj_t * small;
    lv_obj_t * big;
    lv_obj_t * row = tall_row(screen, &small, &big);
    trlv_justify(row, TRLV_JUSTIFY_END);
    trlv_align_items(row, TRLV_ALIGN_ITEMS_CENTER);
    trlv_justify(row, TRLV_JUSTIFY_END);

    TRLV_ASSERT_RECT(small, 60, 20, 20, 10);
    TRLV_ASSERT_RECT(big, 80, 10, 20, 30);
}

int main(void)
{
    TRLV_TEST_RUN(justify_start);
    TRLV_TEST_RUN(justify_center);
    TRLV_TEST_RUN(justify_end);
    TRLV_TEST_RUN(justify_space_between);
    TRLV_TEST_RUN(justify_space_around);
    TRLV_TEST_RUN(justify_space_evenly);
    TRLV_TEST_RUN(justify_spacing_adds_to_the_gap);
    TRLV_TEST_RUN(justify_in_a_column);
    TRLV_TEST_RUN(justify_replaces_its_previous_style);
    TRLV_TEST_RUN(justify_on_a_plain_object);
    TRLV_TEST_RUN(justify_ignores_an_invalid_value);
    TRLV_TEST_RUN(align_items_in_a_row);
    TRLV_TEST_RUN(align_items_in_a_column);
    TRLV_TEST_RUN(align_items_replaces_its_previous_style);
    TRLV_TEST_RUN(align_items_line_placement_can_be_overridden);
    TRLV_TEST_RUN(align_items_on_a_plain_object);
    TRLV_TEST_RUN(justify_and_align_items_are_independent);
    return trlv_test_finish();
}
