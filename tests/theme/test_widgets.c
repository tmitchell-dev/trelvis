/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * MD3 treatment of LVGL's built-in widgets (docs/concepts.md section 6.4):
 * switch, checkbox, slider, bar, arc, spinner, dropdown and textarea. Each
 * is checked for its key colours, its checked and disabled states where it
 * has them, and recolouring on a colour-mode switch.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#include <string.h>

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

#define L (&tokens_light)
#define D (&tokens_dark)

static int theme_errors;

static void log_cb(lv_log_level_t level, const char * buf)
{
    if(level >= LV_LOG_LEVEL_WARN && strstr(buf, "trlv") != NULL) theme_errors++;
}

/* trlv_init in light mode, catching TreLVis warnings and errors. */
static lv_obj_t * init(lv_obj_t * screen)
{
    theme_errors = 0;
    lv_log_register_print_cb(log_cb);
    trlv_init(lv_obj_get_display(screen), L, &tokens_regular);
    return screen;
}

/* Put an object in a state and let LVGL's style transitions finish. */
static void set_state(lv_obj_t * obj, lv_state_t state)
{
    lv_obj_add_state(obj, state);
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
}

static void to_dark(void)
{
    trlv_theme_set_color_mode(D);
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
}

#define BG(obj, part)  lv_obj_get_style_bg_color(obj, part)
#define ARC(obj, part) lv_obj_get_style_arc_color(obj, part)

/* Every theme style is written completely: no TreLVis log output. */
static void theme_styles_build_cleanly(lv_obj_t * screen)
{
    init(screen);
    trlv_theme_set_color_mode(D);

    TRLV_ASSERT_INT(theme_errors, 0);
}

/*---------------------------------------------------------------------------
 * Switch (#40)
 *-------------------------------------------------------------------------*/

static void switch_off(lv_obj_t * screen)
{
    lv_obj_t * sw = lv_switch_create(init(screen));
    int32_t h = tokens_regular.control_height[TRLV_SIZE_SM];

    ASSERT_COLOR(BG(sw, LV_PART_MAIN), L->surface_container_highest);
    ASSERT_COLOR(lv_obj_get_style_border_color(sw, LV_PART_MAIN), L->outline);
    ASSERT_COLOR(BG(sw, LV_PART_KNOB), L->outline);
    TRLV_ASSERT_HEIGHT(sw, h);
    TRLV_ASSERT_WIDTH(sw, h * 13 / 8);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(sw, LV_PART_KNOB), -h / 4);
}

static void switch_on(lv_obj_t * screen)
{
    lv_obj_t * sw = lv_switch_create(init(screen));
    set_state(sw, LV_STATE_CHECKED);
    int32_t h = tokens_regular.control_height[TRLV_SIZE_SM];

    ASSERT_COLOR(BG(sw, LV_PART_INDICATOR), L->primary);
    ASSERT_COLOR(lv_obj_get_style_border_color(sw, LV_PART_MAIN), L->primary);
    ASSERT_COLOR(BG(sw, LV_PART_KNOB), L->on_primary);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(sw, LV_PART_KNOB), -h / 8);
}

static void switch_pressed_grows_the_handle(lv_obj_t * screen)
{
    lv_obj_t * sw = lv_switch_create(init(screen));
    set_state(sw, LV_STATE_PRESSED);

    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(sw, LV_PART_KNOB),
                    -tokens_regular.control_height[TRLV_SIZE_SM] / 16);
    TRLV_ASSERT_INT(lv_obj_get_style_recolor_opa(sw, LV_PART_MAIN), LV_OPA_TRANSP);
}

static void switch_disabled(lv_obj_t * screen)
{
    lv_obj_t * sw = lv_switch_create(init(screen));
    set_state(sw, LV_STATE_DISABLED);

    ASSERT_COLOR(BG(sw, LV_PART_MAIN), L->on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(sw, LV_PART_MAIN), L->state.disabled_container);
    ASSERT_COLOR(BG(sw, LV_PART_KNOB), L->on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(sw, LV_PART_KNOB), L->state.disabled_content);
    TRLV_ASSERT_INT(lv_obj_get_style_recolor_opa(sw, LV_PART_MAIN), LV_OPA_TRANSP);
}

static void switch_focus_ring(lv_obj_t * screen)
{
    lv_obj_t * sw = lv_switch_create(init(screen));
    set_state(sw, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);

    ASSERT_COLOR(lv_obj_get_style_outline_color(sw, LV_PART_MAIN), L->focus);
    TRLV_ASSERT_INT(lv_obj_get_style_outline_width(sw, LV_PART_MAIN), tokens_regular.focus_width);
}

static void switch_recolours(lv_obj_t * screen)
{
    lv_obj_t * sw = lv_switch_create(init(screen));
    lv_obj_add_state(sw, LV_STATE_CHECKED);
    to_dark();

    ASSERT_COLOR(BG(sw, LV_PART_INDICATOR), D->primary);
    ASSERT_COLOR(BG(sw, LV_PART_KNOB), D->on_primary);
}

/*---------------------------------------------------------------------------
 * Checkbox (#41)
 *-------------------------------------------------------------------------*/

static void checkbox_unchecked(lv_obj_t * screen)
{
    lv_obj_t * cb = lv_checkbox_create(init(screen));

    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(cb, LV_PART_INDICATOR), LV_OPA_TRANSP);
    ASSERT_COLOR(lv_obj_get_style_border_color(cb, LV_PART_INDICATOR), L->on_surface_variant);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(cb, LV_PART_INDICATOR), tokens_regular.border_width);
    ASSERT_COLOR(lv_obj_get_style_text_color(cb, LV_PART_MAIN), L->on_surface);
}

static void checkbox_checked(lv_obj_t * screen)
{
    lv_obj_t * cb = lv_checkbox_create(init(screen));
    set_state(cb, LV_STATE_CHECKED);

    ASSERT_COLOR(BG(cb, LV_PART_INDICATOR), L->primary);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(cb, LV_PART_INDICATOR), LV_OPA_COVER);
    ASSERT_COLOR(lv_obj_get_style_text_color(cb, LV_PART_INDICATOR), L->on_primary);
}

static void checkbox_pressed_does_not_tint(lv_obj_t * screen)
{
    lv_obj_t * cb = lv_checkbox_create(init(screen));
    set_state(cb, LV_STATE_CHECKED | LV_STATE_PRESSED);

    TRLV_ASSERT_INT(lv_obj_get_style_recolor_opa(cb, LV_PART_INDICATOR), LV_OPA_TRANSP);
    ASSERT_COLOR(BG(cb, LV_PART_INDICATOR), L->primary);
}

static void checkbox_disabled(lv_obj_t * screen)
{
    lv_obj_t * cb = lv_checkbox_create(init(screen));
    set_state(cb, LV_STATE_CHECKED | LV_STATE_DISABLED);

    ASSERT_COLOR(BG(cb, LV_PART_INDICATOR), L->on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(cb, LV_PART_INDICATOR), L->state.disabled_content);
    TRLV_ASSERT_INT(lv_obj_get_style_text_opa(cb, LV_PART_MAIN), L->state.disabled_content);
    TRLV_ASSERT_INT(lv_obj_get_style_recolor_opa(cb, LV_PART_INDICATOR), LV_OPA_TRANSP);
}

static void checkbox_recolours(lv_obj_t * screen)
{
    lv_obj_t * cb = lv_checkbox_create(init(screen));
    lv_obj_add_state(cb, LV_STATE_CHECKED);
    to_dark();

    ASSERT_COLOR(BG(cb, LV_PART_INDICATOR), D->primary);
    ASSERT_COLOR(lv_obj_get_style_text_color(cb, LV_PART_MAIN), D->on_surface);
}

/*---------------------------------------------------------------------------
 * Slider (#42) and bar (#43)
 *-------------------------------------------------------------------------*/

static void slider_parts(lv_obj_t * screen)
{
    lv_obj_t * slider = lv_slider_create(init(screen));

    ASSERT_COLOR(BG(slider, LV_PART_MAIN), L->surface_container_highest);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(slider, LV_PART_MAIN), LV_OPA_COVER);
    ASSERT_COLOR(BG(slider, LV_PART_INDICATOR), L->primary);
    ASSERT_COLOR(BG(slider, LV_PART_KNOB), L->primary);
}

static void slider_pressed_knob_does_not_grow(lv_obj_t * screen)
{
    lv_obj_t * slider = lv_slider_create(init(screen));
    set_state(slider, LV_STATE_PRESSED);

    TRLV_ASSERT_INT(lv_obj_get_style_transform_width(slider, LV_PART_KNOB), 0);
}

static void slider_disabled(lv_obj_t * screen)
{
    lv_obj_t * slider = lv_slider_create(init(screen));
    set_state(slider, LV_STATE_DISABLED);

    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(slider, LV_PART_MAIN), L->state.disabled_container);
    ASSERT_COLOR(BG(slider, LV_PART_INDICATOR), L->on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(slider, LV_PART_INDICATOR), L->state.disabled_content);
    ASSERT_COLOR(BG(slider, LV_PART_KNOB), L->on_surface);
}

static void slider_editing_keeps_the_ring(lv_obj_t * screen)
{
    lv_obj_t * slider = lv_slider_create(init(screen));
    set_state(slider, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY | LV_STATE_EDITED);

    ASSERT_COLOR(lv_obj_get_style_outline_color(slider, LV_PART_MAIN), L->focus);
    TRLV_ASSERT_INT(lv_obj_get_style_outline_opa(slider, LV_PART_MAIN), LV_OPA_COVER);
}

static void slider_recolours(lv_obj_t * screen)
{
    lv_obj_t * slider = lv_slider_create(init(screen));
    to_dark();

    ASSERT_COLOR(BG(slider, LV_PART_MAIN), D->surface_container_highest);
    ASSERT_COLOR(BG(slider, LV_PART_INDICATOR), D->primary);
}

static void bar_parts(lv_obj_t * screen)
{
    lv_obj_t * bar = lv_bar_create(init(screen));

    ASSERT_COLOR(BG(bar, LV_PART_MAIN), L->surface_container_highest);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(bar, LV_PART_MAIN), LV_OPA_COVER);
    ASSERT_COLOR(BG(bar, LV_PART_INDICATOR), L->primary);
}

static void bar_recolours(lv_obj_t * screen)
{
    lv_obj_t * bar = lv_bar_create(init(screen));
    to_dark();

    ASSERT_COLOR(BG(bar, LV_PART_MAIN), D->surface_container_highest);
    ASSERT_COLOR(BG(bar, LV_PART_INDICATOR), D->primary);
}

/*---------------------------------------------------------------------------
 * Arc (#44) and spinner (#47)
 *-------------------------------------------------------------------------*/

static void arc_parts(lv_obj_t * screen)
{
    lv_obj_t * arc = lv_arc_create(init(screen));

    ASSERT_COLOR(ARC(arc, LV_PART_MAIN), L->surface_container_highest);
    ASSERT_COLOR(ARC(arc, LV_PART_INDICATOR), L->primary);
    ASSERT_COLOR(BG(arc, LV_PART_KNOB), L->primary);
}

static void arc_disabled(lv_obj_t * screen)
{
    lv_obj_t * arc = lv_arc_create(init(screen));
    set_state(arc, LV_STATE_DISABLED);

    ASSERT_COLOR(ARC(arc, LV_PART_INDICATOR), L->on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_arc_opa(arc, LV_PART_INDICATOR), L->state.disabled_content);
    TRLV_ASSERT_INT(lv_obj_get_style_arc_opa(arc, LV_PART_MAIN), L->state.disabled_container);
}

static void arc_recolours(lv_obj_t * screen)
{
    lv_obj_t * arc = lv_arc_create(init(screen));
    to_dark();

    ASSERT_COLOR(ARC(arc, LV_PART_MAIN), D->surface_container_highest);
    ASSERT_COLOR(ARC(arc, LV_PART_INDICATOR), D->primary);
}

static void spinner_parts(lv_obj_t * screen)
{
    lv_obj_t * spinner = lv_spinner_create(init(screen));

    ASSERT_COLOR(ARC(spinner, LV_PART_MAIN), L->surface_container_highest);
    ASSERT_COLOR(ARC(spinner, LV_PART_INDICATOR), L->primary);
}

static void spinner_recolours(lv_obj_t * screen)
{
    lv_obj_t * spinner = lv_spinner_create(init(screen));
    to_dark();

    ASSERT_COLOR(ARC(spinner, LV_PART_INDICATOR), D->primary);
}

/*---------------------------------------------------------------------------
 * Dropdown (#45) and textarea (#46)
 *-------------------------------------------------------------------------*/

static void dropdown_field(lv_obj_t * screen)
{
    lv_obj_t * dd = lv_dropdown_create(init(screen));

    ASSERT_COLOR(BG(dd, LV_PART_MAIN), L->surface);
    ASSERT_COLOR(lv_obj_get_style_border_color(dd, LV_PART_MAIN), L->outline);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(dd, LV_PART_MAIN), tokens_regular.border_width);
    TRLV_ASSERT_INT(lv_obj_get_style_radius(dd, LV_PART_MAIN), tokens_regular.radius[TRLV_RADIUS_XS]);
    ASSERT_COLOR(lv_obj_get_style_text_color(dd, LV_PART_MAIN), L->on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(dd, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_LG]);
}

static void dropdown_focused(lv_obj_t * screen)
{
    lv_obj_t * dd = lv_dropdown_create(init(screen));
    set_state(dd, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);

    ASSERT_COLOR(lv_obj_get_style_border_color(dd, LV_PART_MAIN), L->primary);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(dd, LV_PART_MAIN), tokens_regular.focus_width);
    TRLV_ASSERT_INT(lv_obj_get_style_outline_width(dd, LV_PART_MAIN), 0);
}

static void dropdown_menu(lv_obj_t * screen)
{
    lv_obj_t * dd = lv_dropdown_create(init(screen));
    lv_dropdown_set_options(dd, "One\nTwo\nThree");
    lv_dropdown_open(dd);
    lv_obj_t * list = lv_dropdown_get_list(dd);

    ASSERT_COLOR(BG(list, LV_PART_MAIN), L->surface_container);
    ASSERT_COLOR(lv_obj_get_style_text_color(list, LV_PART_MAIN), L->on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_width(list, LV_PART_MAIN),
                    tokens_regular.elevation[TRLV_ELEVATION_2].blur);
    ASSERT_COLOR(BG(list, LV_PART_SELECTED), L->surface_container);

    lv_obj_add_state(list, LV_STATE_CHECKED);
    ASSERT_COLOR(BG(list, LV_PART_SELECTED), L->secondary_container);
    ASSERT_COLOR(lv_obj_get_style_text_color(list, LV_PART_SELECTED), L->on_secondary_container);
}

static void dropdown_disabled(lv_obj_t * screen)
{
    lv_obj_t * dd = lv_dropdown_create(init(screen));
    set_state(dd, LV_STATE_DISABLED);

    ASSERT_COLOR(lv_obj_get_style_border_color(dd, LV_PART_MAIN), L->on_surface);
    TRLV_ASSERT_INT(lv_obj_get_style_border_opa(dd, LV_PART_MAIN), L->state.disabled_container);
    TRLV_ASSERT_INT(lv_obj_get_style_text_opa(dd, LV_PART_MAIN), L->state.disabled_content);
    TRLV_ASSERT_INT(lv_obj_get_style_recolor_opa(dd, LV_PART_MAIN), LV_OPA_TRANSP);
}

static void dropdown_recolours(lv_obj_t * screen)
{
    lv_obj_t * dd = lv_dropdown_create(init(screen));
    to_dark();

    ASSERT_COLOR(BG(dd, LV_PART_MAIN), D->surface);
    ASSERT_COLOR(lv_obj_get_style_border_color(dd, LV_PART_MAIN), D->outline);
}

static void textarea_field(lv_obj_t * screen)
{
    lv_obj_t * ta = lv_textarea_create(init(screen));

    ASSERT_COLOR(lv_obj_get_style_border_color(ta, LV_PART_MAIN), L->outline);
    ASSERT_COLOR(lv_obj_get_style_text_color(ta, LV_PART_MAIN), L->on_surface);
    ASSERT_COLOR(lv_obj_get_style_text_color(ta, LV_PART_TEXTAREA_PLACEHOLDER), L->on_surface_variant);
}

static void textarea_focused(lv_obj_t * screen)
{
    lv_obj_t * ta = lv_textarea_create(init(screen));
    set_state(ta, LV_STATE_FOCUSED);

    ASSERT_COLOR(lv_obj_get_style_border_color(ta, LV_PART_MAIN), L->primary);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(ta, LV_PART_MAIN), tokens_regular.focus_width);
    ASSERT_COLOR(lv_obj_get_style_border_color(ta, LV_PART_CURSOR), L->primary);
}

static void textarea_recolours(lv_obj_t * screen)
{
    lv_obj_t * ta = lv_textarea_create(init(screen));
    to_dark();

    ASSERT_COLOR(lv_obj_get_style_border_color(ta, LV_PART_MAIN), D->outline);
    ASSERT_COLOR(lv_obj_get_style_text_color(ta, LV_PART_TEXTAREA_PLACEHOLDER), D->on_surface_variant);
}

int main(void)
{
    TRLV_TEST_RUN(theme_styles_build_cleanly);

    TRLV_TEST_RUN(switch_off);
    TRLV_TEST_RUN(switch_on);
    TRLV_TEST_RUN(switch_pressed_grows_the_handle);
    TRLV_TEST_RUN(switch_disabled);
    TRLV_TEST_RUN(switch_focus_ring);
    TRLV_TEST_RUN(switch_recolours);

    TRLV_TEST_RUN(checkbox_unchecked);
    TRLV_TEST_RUN(checkbox_checked);
    TRLV_TEST_RUN(checkbox_pressed_does_not_tint);
    TRLV_TEST_RUN(checkbox_disabled);
    TRLV_TEST_RUN(checkbox_recolours);

    TRLV_TEST_RUN(slider_parts);
    TRLV_TEST_RUN(slider_pressed_knob_does_not_grow);
    TRLV_TEST_RUN(slider_disabled);
    TRLV_TEST_RUN(slider_editing_keeps_the_ring);
    TRLV_TEST_RUN(slider_recolours);
    TRLV_TEST_RUN(bar_parts);
    TRLV_TEST_RUN(bar_recolours);

    TRLV_TEST_RUN(arc_parts);
    TRLV_TEST_RUN(arc_disabled);
    TRLV_TEST_RUN(arc_recolours);
    TRLV_TEST_RUN(spinner_parts);
    TRLV_TEST_RUN(spinner_recolours);

    TRLV_TEST_RUN(dropdown_field);
    TRLV_TEST_RUN(dropdown_focused);
    TRLV_TEST_RUN(dropdown_menu);
    TRLV_TEST_RUN(dropdown_disabled);
    TRLV_TEST_RUN(dropdown_recolours);
    TRLV_TEST_RUN(textarea_field);
    TRLV_TEST_RUN(textarea_focused);
    TRLV_TEST_RUN(textarea_recolours);
    return trlv_test_finish();
}
