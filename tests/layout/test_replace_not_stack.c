/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Replace, not stack: every modifier style group, through the public API.
 *
 * Each test applies one value of a modifier, adds a project style that sets
 * the same property, then applies another value. If the modifier replaced
 * its previous style in place, the project style (added later) still wins.
 * If it stacked a new style on top, the new one would win instead. Final
 * values alone can't tell the two apart, because the last-added style wins
 * either way.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

#define ASSERT_TRUE(cond) \
    trlv_test_check_int(__FILE__, __LINE__, #cond, 1, (cond) ? 1 : 0)

/* The project's style. Tests run with a fresh LVGL each time, so it is
 * initialised again in every test. */
static lv_style_t project;

static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    lv_style_init(&project);
    return screen;
}

static void add_project(lv_obj_t * obj, lv_style_selector_t selector)
{
    lv_obj_add_style(obj, &project, selector);
}

static lv_color_t project_color(void)
{
    return lv_color_hex(0x123456);
}

/* trlv_set_gap: pad_row and pad_column. */
static void gap(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_LG);
    lv_style_set_pad_column(&project, 40);
    add_project(row, LV_PART_MAIN);
    trlv_set_gap(row, TRLV_SPACE_SM);

    TRLV_ASSERT_INT(lv_obj_get_style_pad_column(row, LV_PART_MAIN), 40);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_row(row, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_SM]);
}

/* trlv_set_pad: the four sides. */
static void pad(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    trlv_set_pad(box, TRLV_SPACE_LG);
    lv_style_set_pad_left(&project, 40);
    add_project(box, LV_PART_MAIN);
    trlv_set_pad(box, TRLV_SPACE_SM);

    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(box, LV_PART_MAIN), 40);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(box, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_SM]);
}

/* Fit and fill width, one group. */
static void width(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    lv_obj_set_size(box, 200, 100);
    lv_obj_t * obj = trlv_box_create(box);
    trlv_set_width_fit(obj);
    lv_style_set_width(&project, 77);
    add_project(obj, LV_PART_MAIN);
    trlv_set_width_fill(obj);
    TRLV_ASSERT_WIDTH(obj, 77);

    trlv_set_width_fit(obj);
    TRLV_ASSERT_WIDTH(obj, 77);
}

/* Fit and fill height, one group. Without a flex parent, fill sets the
 * height (100 %), the property the project style sets. */
static void height(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    lv_obj_set_size(box, 200, 100);
    lv_obj_t * obj = trlv_box_create(box);
    trlv_set_height_fill(obj);
    lv_style_set_height(&project, 33);
    add_project(obj, LV_PART_MAIN);
    trlv_set_height_fit(obj);
    TRLV_ASSERT_HEIGHT(obj, 33);

    trlv_set_height_fill(obj);
    TRLV_ASSERT_HEIGHT(obj, 33);
}

/* trlv_set_justify: flex_main_place. */
static void justify(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_justify(row, TRLV_JUSTIFY_END);
    lv_style_set_flex_main_place(&project, LV_FLEX_ALIGN_SPACE_AROUND);
    add_project(row, LV_PART_MAIN);
    trlv_set_justify(row, TRLV_JUSTIFY_CENTER);

    TRLV_ASSERT_INT(lv_obj_get_style_flex_main_place(row, LV_PART_MAIN), LV_FLEX_ALIGN_SPACE_AROUND);
}

/* trlv_set_align_items: flex_cross_place and flex_track_place. */
static void align_items(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_align_items(row, TRLV_ALIGN_ITEMS_END);
    lv_style_set_flex_cross_place(&project, LV_FLEX_ALIGN_CENTER);
    add_project(row, LV_PART_MAIN);
    trlv_set_align_items(row, TRLV_ALIGN_ITEMS_START);

    TRLV_ASSERT_INT(lv_obj_get_style_flex_cross_place(row, LV_PART_MAIN), LV_FLEX_ALIGN_CENTER);
    TRLV_ASSERT_INT(lv_obj_get_style_flex_track_place(row, LV_PART_MAIN), LV_FLEX_ALIGN_START);
}

/* trlv_text_create and trlv_apply_text_role: one group. */
static void text_role(lv_obj_t * screen)
{
    lv_obj_t * text = trlv_text_create(setup(screen), "Text", TRLV_TEXT_TITLE_LARGE);
    lv_style_set_text_font(&project, LV_FONT_DEFAULT);
    add_project(text, LV_PART_MAIN);
    trlv_apply_text_role(text, TRLV_TEXT_LABEL_SMALL);

    ASSERT_TRUE(lv_obj_get_style_text_font(text, LV_PART_MAIN) == LV_FONT_DEFAULT);
}

/* trlv_apply_surface, trlv_apply_surface_custom and trlv_apply_card: one group. */
static void fill(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    trlv_apply_surface(box, TRLV_SURFACE_CONTAINER);
    lv_style_set_bg_color(&project, project_color());
    add_project(box, LV_PART_MAIN);

    trlv_apply_card(box, TRLV_CARD_OUTLINED);
    ASSERT_COLOR(lv_obj_get_style_bg_color(box, LV_PART_MAIN), project_color());
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(box, LV_PART_MAIN), tokens_regular.border_width);

#if TRLV_CUSTOM_COLOR_MAX > 0
    trlv_apply_surface_custom(box, TOKEN_CUSTOM_SUCCESS, TRLV_CUSTOM_SURFACE_COLOR);
    ASSERT_COLOR(lv_obj_get_style_bg_color(box, LV_PART_MAIN), project_color());
    ASSERT_COLOR(lv_obj_get_style_text_color(box, LV_PART_MAIN),
                 tokens_light.custom[TOKEN_CUSTOM_SUCCESS].on_color);
#endif

    trlv_apply_surface(box, TRLV_SURFACE_INVERSE);
    ASSERT_COLOR(lv_obj_get_style_bg_color(box, LV_PART_MAIN), project_color());
    ASSERT_COLOR(lv_obj_get_style_text_color(box, LV_PART_MAIN), tokens_light.inverse_on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(box, LV_PART_MAIN), 0);
}

/* trlv_apply_button_type: four groups, one per state selector. The project
 * style sets the pressed colour; a later type must not take it over. */
static void button_type(lv_obj_t * screen)
{
    lv_obj_t * btn = lv_button_create(setup(screen));
    trlv_apply_button_type(btn, TRLV_BUTTON_TONAL);
    lv_style_set_bg_color(&project, project_color());
    add_project(btn, LV_PART_MAIN | LV_STATE_PRESSED);
    trlv_apply_button_type(btn, TRLV_BUTTON_FILLED);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.primary);

    lv_obj_add_state(btn, LV_STATE_PRESSED);
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), project_color());
}

int main(void)
{
    TRLV_TEST_RUN(gap);
    TRLV_TEST_RUN(pad);
    TRLV_TEST_RUN(width);
    TRLV_TEST_RUN(height);
    TRLV_TEST_RUN(justify);
    TRLV_TEST_RUN(align_items);
    TRLV_TEST_RUN(text_role);
    TRLV_TEST_RUN(fill);
    TRLV_TEST_RUN(button_type);
    return trlv_test_finish();
}
