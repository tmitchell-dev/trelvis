/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_apply_surface_custom with the token template's two custom colours
 * (success, warning). Built twice: with the default TRLV_CUSTOM_COLOR_MAX
 * (4) and with 0, where every call warns and changes nothing.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#include <string.h>

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

static int custom_warnings;

static void log_cb(lv_log_level_t level, const char * buf)
{
    if(level == LV_LOG_LEVEL_WARN && strstr(buf, "trlv_apply_surface_custom:") != NULL) custom_warnings++;
}

/* lv_deinit clears the log callback, so it is registered in every test. */
static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    custom_warnings = 0;
    lv_log_register_print_cb(log_cb);
    return screen;
}

static lv_color_t bg_of(lv_obj_t * obj)
{
    return lv_obj_get_style_bg_color(obj, LV_PART_MAIN);
}

#if TRLV_CUSTOM_COLOR_MAX > 0

/* Let LVGL's style transitions finish. */
static void settle(void)
{
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
}

static lv_color_t text_of(lv_obj_t * obj)
{
    return lv_obj_get_style_text_color(obj, LV_PART_MAIN);
}

static lv_obj_t * column_with_label(lv_obj_t * parent, lv_obj_t ** label)
{
    lv_obj_t * column = trlv_column_create(parent);
    trlv_set_gap(column, TRLV_SPACE_SM);
    *label = lv_label_create(column);
    return column;
}

/* Each pair of each custom colour, with its content colour inherited. */
static void every_pair(lv_obj_t * screen)
{
    setup(screen);
    TRLV_ASSERT_INT(TRLV_CUSTOM_COLOR_MAX >= TOKEN_CUSTOM_COUNT, 1);
    for(uint32_t i = 0; i < TOKEN_CUSTOM_COUNT; i++) {
        const trlv_custom_color_t * custom = &tokens_light.custom[i];
        lv_obj_t * label;
        lv_obj_t * strong = column_with_label(screen, &label);
        trlv_apply_surface_custom(strong, i, TRLV_CUSTOM_SURFACE_COLOR);
        ASSERT_COLOR(bg_of(strong), custom->color);
        TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(strong, LV_PART_MAIN), LV_OPA_COVER);
        ASSERT_COLOR(text_of(label), custom->on_color);

        lv_obj_t * container = column_with_label(screen, &label);
        trlv_apply_surface_custom(container, i, TRLV_CUSTOM_SURFACE_CONTAINER);
        ASSERT_COLOR(bg_of(container), custom->color_container);
        ASSERT_COLOR(text_of(label), custom->on_color_container);
    }
    TRLV_ASSERT_INT(custom_warnings, 0);
}

/* Recolours on a colour-mode switch. */
static void recolours(lv_obj_t * screen)
{
    lv_obj_t * label;
    lv_obj_t * column = column_with_label(setup(screen), &label);
    trlv_apply_surface_custom(column, TOKEN_CUSTOM_WARNING, TRLV_CUSTOM_SURFACE_CONTAINER);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();

    ASSERT_COLOR(bg_of(column), tokens_dark.custom[TOKEN_CUSTOM_WARNING].color_container);
    ASSERT_COLOR(text_of(label), tokens_dark.custom[TOKEN_CUSTOM_WARNING].on_color_container);
}

/* Replaces trlv_apply_surface and is replaced by it: one group. */
static void shares_the_group_with_surface(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    trlv_apply_surface(box, TRLV_SURFACE_CONTAINER_HIGH);
    trlv_apply_surface_custom(box, TOKEN_CUSTOM_SUCCESS, TRLV_CUSTOM_SURFACE_COLOR);
    ASSERT_COLOR(bg_of(box), tokens_light.custom[TOKEN_CUSTOM_SUCCESS].color);

    trlv_apply_surface_custom(box, TOKEN_CUSTOM_SUCCESS, TRLV_CUSTOM_SURFACE_CONTAINER);
    ASSERT_COLOR(bg_of(box), tokens_light.custom[TOKEN_CUSTOM_SUCCESS].color_container);

    trlv_apply_surface(box, TRLV_SURFACE_CONTAINER_HIGH);
    ASSERT_COLOR(bg_of(box), tokens_light.surface_container_high);
    ASSERT_COLOR(text_of(box), tokens_light.on_surface);
}

/* An index the tokens don't define (template: 2 entries) warns and changes
 * nothing, as does one at TRLV_CUSTOM_COLOR_MAX. */
static void out_of_range_index_warns(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    trlv_apply_surface(box, TRLV_SURFACE_CONTAINER);

    trlv_apply_surface_custom(box, TOKEN_CUSTOM_COUNT, TRLV_CUSTOM_SURFACE_COLOR);
    trlv_apply_surface_custom(box, TRLV_CUSTOM_COLOR_MAX, TRLV_CUSTOM_SURFACE_COLOR);
    trlv_apply_surface_custom(box, TOKEN_CUSTOM_SUCCESS, TRLV_CUSTOM_SURFACE_COUNT);

    TRLV_ASSERT_INT(custom_warnings, 3);
    ASSERT_COLOR(bg_of(box), tokens_light.surface_container);
}

#else /* TRLV_CUSTOM_COLOR_MAX 0 */

/* With no custom styles compiled in, every call warns and changes nothing. */
static void every_call_warns(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    trlv_apply_surface(box, TRLV_SURFACE_CONTAINER);
    trlv_apply_surface_custom(box, TOKEN_CUSTOM_SUCCESS, TRLV_CUSTOM_SURFACE_COLOR);

    TRLV_ASSERT_INT(custom_warnings, 1);
    ASSERT_COLOR(bg_of(box), tokens_light.surface_container);
}

#endif

int main(void)
{
#if TRLV_CUSTOM_COLOR_MAX > 0
    TRLV_TEST_RUN(every_pair);
    TRLV_TEST_RUN(recolours);
    TRLV_TEST_RUN(shares_the_group_with_surface);
    TRLV_TEST_RUN(out_of_range_index_warns);
#else
    TRLV_TEST_RUN(every_call_warns);
#endif
    return trlv_test_finish();
}
