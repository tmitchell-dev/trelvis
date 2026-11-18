/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_divider_create: a border_width line in outline_variant, horizontal in a
 * column or a parent without flex, vertical in a row.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    TRLV_ASSERT_INT(tokens_regular.border_width > 0, 1);
    return screen;
}

static lv_obj_t * child(lv_obj_t * parent, int32_t w, int32_t h)
{
    lv_obj_t * obj = trlv_box_create(parent);
    lv_obj_set_size(obj, w, h);
    return obj;
}

static int32_t line(void)
{
    return tokens_regular.border_width;
}

/* In a column: a horizontal line across the column's width, between the
 * items, with the gap on both sides. */
static void horizontal_in_a_column(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_SM);
    lv_obj_set_width(column, 200);
    child(column, 50, 20);
    lv_obj_t * divider = trlv_divider_create(column);
    lv_obj_t * after = child(column, 50, 20);

    TRLV_ASSERT_RECT(divider, 0, 20 + 8, 200, line());
    TRLV_ASSERT_Y(after, 20 + 8 + line() + 8);
    ASSERT_COLOR(lv_obj_get_style_bg_color(divider, LV_PART_MAIN), tokens_light.outline_variant);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(divider, LV_PART_MAIN), LV_OPA_COVER);
}

/* In a column that fits its width to its content: as wide as the widest sibling. */
static void horizontal_in_a_fit_content_column(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    child(column, 80, 20);
    lv_obj_t * divider = trlv_divider_create(column);

    TRLV_ASSERT_RECT(divider, 0, 20, 80, line());
}

/* In a row: a vertical line the row's height. */
static void vertical_in_a_row(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    lv_obj_set_height(row, 40);
    child(row, 30, 20);
    lv_obj_t * divider = trlv_divider_create(row);
    lv_obj_t * after = child(row, 30, 20);

    TRLV_ASSERT_RECT(divider, 30 + 8, 0, line(), 40);
    TRLV_ASSERT_X(after, 30 + 8 + line() + 8);
}

/* In a row that fits its height to its content: as tall as the tallest sibling. */
static void vertical_in_a_fit_content_row(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    child(row, 30, 20);
    trlv_divider_create(row);
    child(row, 30, 36);
    lv_obj_t * divider = lv_obj_get_child(row, 1);

    TRLV_ASSERT_RECT(divider, 30, 0, line(), 36);
}

/* Without a flex parent: horizontal, the parent's full width. */
static void horizontal_without_flex(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    lv_obj_set_size(box, 120, 60);
    lv_obj_t * divider = trlv_divider_create(box);

    TRLV_ASSERT_RECT(divider, 0, 0, 120, line());
}

/* In a flex lv_obj TreLVis did not create: follows its flow. */
static void vertical_in_a_plain_flex_row(lv_obj_t * screen)
{
    lv_obj_t * obj = lv_obj_create(setup(screen));
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 200, 40);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    child(obj, 30, 20);
    lv_obj_t * divider = trlv_divider_create(obj);

    TRLV_ASSERT_RECT(divider, 30, 0, line(), 40);
}

/* A margin insets it, as for an MD3 inset divider. */
static void margin_insets_it(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    lv_obj_set_width(column, 200);
    lv_obj_t * divider = trlv_divider_create(column);
    lv_obj_set_style_margin_left(divider, 16, LV_PART_MAIN);

    TRLV_ASSERT_X(divider, 16);
}

/* The colour follows a colour-mode switch. */
static void recolours(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    lv_obj_t * divider = trlv_divider_create(column);
    trlv_theme_set_color_mode(&tokens_dark);

    ASSERT_COLOR(lv_obj_get_style_bg_color(divider, LV_PART_MAIN), tokens_dark.outline_variant);
}

/* The thickness follows the layout tokens on trlv_init. */
static void thickness_follows_the_tokens(lv_obj_t * screen)
{
    static trlv_layout_tokens_t thick;
    lv_obj_t * column = trlv_column_create(setup(screen));
    lv_obj_set_width(column, 100);
    lv_obj_t * divider = trlv_divider_create(column);

    thick = tokens_regular;
    thick.border_width = 3;
    trlv_init(lv_obj_get_display(screen), &tokens_light, &thick);

    TRLV_ASSERT_HEIGHT(divider, 3);
}

int main(void)
{
    TRLV_TEST_RUN(horizontal_in_a_column);
    TRLV_TEST_RUN(horizontal_in_a_fit_content_column);
    TRLV_TEST_RUN(vertical_in_a_row);
    TRLV_TEST_RUN(vertical_in_a_fit_content_row);
    TRLV_TEST_RUN(horizontal_without_flex);
    TRLV_TEST_RUN(vertical_in_a_plain_flex_row);
    TRLV_TEST_RUN(margin_insets_it);
    TRLV_TEST_RUN(recolours);
    TRLV_TEST_RUN(thickness_follows_the_tokens);
    return trlv_test_finish();
}
