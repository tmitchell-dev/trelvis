/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_theme_set_color_mode: existing objects recolour, built-in widgets
 * included, with no per-screen code and no memory growth.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

#define ASSERT_TRUE(cond) \
    trlv_test_check_int(__FILE__, __LINE__, #cond, 1, (cond) ? 1 : 0)

static void init_light(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
}

/* Let LVGL's style transitions finish. */
static void settle(void)
{
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
}

/* Before trlv_init a switch is rejected (this test runs first). */
static void switch_before_init_is_rejected(lv_obj_t * screen)
{
    (void)screen;
    trlv_theme_set_color_mode(&tokens_dark);

    ASSERT_TRUE(trlv_color() == NULL);
}

/* The screen, a label on it, a plain container and a button made in light
 * mode all take the dark colours. */
static void existing_objects_recolour(lv_obj_t * screen)
{
    init_light(screen);
    lv_obj_t * text = lv_label_create(screen);
    lv_obj_t * obj = lv_obj_create(screen);
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_t * btn_label = lv_label_create(btn);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();

    ASSERT_TRUE(trlv_color() == &tokens_dark);
    ASSERT_COLOR(lv_obj_get_style_bg_color(screen, LV_PART_MAIN), tokens_dark.surface);
    ASSERT_COLOR(lv_obj_get_style_text_color(text, LV_PART_MAIN), tokens_dark.on_surface);
    ASSERT_COLOR(lv_obj_get_style_border_color(obj, LV_PART_MAIN), tokens_dark.outline_variant);
    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_dark.primary);
    ASSERT_COLOR(lv_obj_get_style_text_color(btn_label, LV_PART_MAIN), tokens_dark.on_primary);
}

/* The pre-mixed state colours are mixed again from the new colours. */
static void state_layers_recolour(lv_obj_t * screen)
{
    init_light(screen);
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_add_state(btn, LV_STATE_PRESSED);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN),
                 lv_color_mix(tokens_dark.on_primary, tokens_dark.primary, tokens_dark.state.pressed));
}

/* A widget the theme doesn't cover follows too: the parent theme is
 * re-initialised with the new primary colour and dark flag. */
static void parent_theme_recolours(lv_obj_t * screen)
{
    init_light(screen);
    lv_obj_t * roller = lv_roller_create(screen);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();

    ASSERT_COLOR(lv_obj_get_style_bg_color(roller, LV_PART_SELECTED), tokens_dark.primary);
    ASSERT_TRUE(lv_color_luminance(lv_obj_get_style_bg_color(roller, LV_PART_MAIN)) < 128);
    ASSERT_COLOR(lv_theme_get_color_primary(screen), tokens_dark.primary);
}

/* Switching back restores the light colours. */
static void switching_back_restores_light(lv_obj_t * screen)
{
    init_light(screen);
    lv_obj_t * btn = lv_button_create(screen);

    trlv_theme_set_color_mode(&tokens_dark);
    trlv_theme_set_color_mode(&tokens_light);
    settle();

    ASSERT_COLOR(lv_obj_get_style_bg_color(screen, LV_PART_MAIN), tokens_light.surface);
    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.primary);
}

/* The theme stays the same object, and repeated switches use no more
 * memory. */
static void switches_do_not_grow_memory(lv_obj_t * screen)
{
    init_light(screen);
    lv_display_t * disp = lv_obj_get_display(screen);
    lv_theme_t * theme = lv_display_get_theme(disp);
    lv_button_create(screen);

    /* One round first, so anything allocated once is in place. */
    trlv_theme_set_color_mode(&tokens_dark);
    trlv_theme_set_color_mode(&tokens_light);
    lv_mem_monitor_t before;
    lv_mem_monitor(&before);

    for(int i = 0; i < 20; i++) {
        trlv_theme_set_color_mode(&tokens_dark);
        trlv_theme_set_color_mode(&tokens_light);
    }

    lv_mem_monitor_t after;
    lv_mem_monitor(&after);
    ASSERT_TRUE(lv_display_get_theme(disp) == theme);
    TRLV_ASSERT_INT((int32_t)after.total_size - (int32_t)after.free_size,
                    (int32_t)before.total_size - (int32_t)before.free_size);
}

/* NULL is rejected and the mode stays. */
static void null_is_rejected(lv_obj_t * screen)
{
    init_light(screen);
    trlv_theme_set_color_mode(NULL);

    ASSERT_TRUE(trlv_color() == &tokens_light);
}

int main(void)
{
    TRLV_TEST_RUN(switch_before_init_is_rejected);
    TRLV_TEST_RUN(existing_objects_recolour);
    TRLV_TEST_RUN(state_layers_recolour);
    TRLV_TEST_RUN(parent_theme_recolours);
    TRLV_TEST_RUN(switching_back_restores_light);
    TRLV_TEST_RUN(switches_do_not_grow_memory);
    TRLV_TEST_RUN(null_is_rejected);
    return trlv_test_finish();
}
