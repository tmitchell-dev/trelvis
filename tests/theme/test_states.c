/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * MD3 states on the filled button: pressed and focus state layers
 * (pre-mixed colours), the focus-key ring and the disabled treatment.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

static lv_obj_t * label;

/* A filled button with a label, after trlv_init with `color`. */
static lv_obj_t * button(lv_obj_t * screen, const trlv_color_tokens_t * color)
{
    trlv_init(lv_obj_get_display(screen), color, &tokens_regular);
    lv_obj_t * btn = lv_button_create(screen);
    label = lv_label_create(btn);
    lv_label_set_text(label, "OK");
    return btn;
}

/* Put the button in a state and let LVGL's style transitions finish (the
 * parent theme animates the change). */
static void set_state(lv_obj_t * obj, lv_state_t state)
{
    lv_obj_add_state(obj, state);
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
}

/* Pressed: on_primary over primary at state.pressed, pre-mixed. The label
 * keeps on_primary: nothing tints the content, and nothing grows. */
static void pressed_is_a_state_layer(lv_obj_t * screen)
{
    lv_obj_t * btn = button(screen, &tokens_light);
    set_state(btn, LV_STATE_PRESSED);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN),
                 lv_color_mix(tokens_light.on_primary, tokens_light.primary, tokens_light.state.pressed));
    TRLV_ASSERT_INT(lv_obj_get_style_recolor_opa(btn, LV_PART_MAIN), LV_OPA_TRANSP);
    TRLV_ASSERT_INT(lv_obj_get_style_transform_width(btn, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_transform_height(btn, LV_PART_MAIN), 0);
    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_light.on_primary);
}

/* The state layer follows the token opacity. */
static void pressed_follows_the_state_token(lv_obj_t * screen)
{
    static trlv_color_tokens_t color;
    color = tokens_light;
    color.state.pressed = LV_OPA_30;
    lv_obj_t * btn = button(screen, &color);
    set_state(btn, LV_STATE_PRESSED);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN),
                 lv_color_mix(color.on_primary, color.primary, LV_OPA_30));
}

/* Focused by keypad or encoder: the focus state layer and a ring in the
 * focus colour, focus_width thick and 2 px (space 2XS) outside. */
static void focus_key_has_ring_and_state_layer(lv_obj_t * screen)
{
    lv_obj_t * btn = button(screen, &tokens_light);
    set_state(btn, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN),
                 lv_color_mix(tokens_light.on_primary, tokens_light.primary, tokens_light.state.focus));
    TRLV_ASSERT_INT(lv_obj_get_style_outline_width(btn, LV_PART_MAIN), tokens_regular.focus_width);
    ASSERT_COLOR(lv_obj_get_style_outline_color(btn, LV_PART_MAIN), tokens_light.focus);
    TRLV_ASSERT_INT(lv_obj_get_style_outline_opa(btn, LV_PART_MAIN), LV_OPA_COVER);
    TRLV_ASSERT_INT(lv_obj_get_style_outline_pad(btn, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_2XS]);
}

/* A pointer focus (a touch) draws no ring. */
static void pointer_focus_has_no_ring(lv_obj_t * screen)
{
    lv_obj_t * btn = button(screen, &tokens_light);
    set_state(btn, LV_STATE_FOCUSED);

    TRLV_ASSERT_INT(lv_obj_get_style_outline_width(btn, LV_PART_MAIN) == tokens_regular.focus_width &&
                    lv_color_eq(lv_obj_get_style_outline_color(btn, LV_PART_MAIN), tokens_light.focus), false);
    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.primary);
}

/* Disabled: on_surface at 12 % for the container, 38 % for the content. */
static void disabled_uses_on_surface_opacities(lv_obj_t * screen)
{
    lv_obj_t * btn = button(screen, &tokens_light);
    set_state(btn, LV_STATE_DISABLED);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(btn, LV_PART_MAIN), tokens_light.state.disabled_container);
    TRLV_ASSERT_INT(lv_obj_get_style_recolor_opa(btn, LV_PART_MAIN), LV_OPA_TRANSP);
    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_light.on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_text_opa(label, LV_PART_MAIN), tokens_light.state.disabled_content);
}

/* Disabled wins over pressed. */
static void disabled_wins_over_pressed(lv_obj_t * screen)
{
    lv_obj_t * btn = button(screen, &tokens_light);
    set_state(btn, LV_STATE_DISABLED | LV_STATE_PRESSED);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(btn, LV_PART_MAIN), tokens_light.state.disabled_container);
}

/* In dark mode the state layer mixes the dark pair. */
static void pressed_in_dark_mode(lv_obj_t * screen)
{
    lv_obj_t * btn = button(screen, &tokens_dark);
    set_state(btn, LV_STATE_PRESSED);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN),
                 lv_color_mix(tokens_dark.on_primary, tokens_dark.primary, tokens_dark.state.pressed));
}

int main(void)
{
    TRLV_TEST_RUN(pressed_is_a_state_layer);
    TRLV_TEST_RUN(pressed_follows_the_state_token);
    TRLV_TEST_RUN(focus_key_has_ring_and_state_layer);
    TRLV_TEST_RUN(pointer_focus_has_no_ring);
    TRLV_TEST_RUN(disabled_uses_on_surface_opacities);
    TRLV_TEST_RUN(disabled_wins_over_pressed);
    TRLV_TEST_RUN(pressed_in_dark_mode);
    return trlv_test_finish();
}
