/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_button_variant: the four MD3 button types in their default, pressed,
 * focused and disabled states, over the theme's filled button.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

static lv_obj_t * label;

/* A button with a label, after trlv_init with `color`. */
static lv_obj_t * button(lv_obj_t * screen, const trlv_color_tokens_t * color)
{
    trlv_init(lv_obj_get_display(screen), color, &tokens_regular);
    lv_obj_t * btn = lv_button_create(screen);
    label = lv_label_create(btn);
    lv_label_set_text(label, "OK");
    return btn;
}

/* Let LVGL's style transitions finish. */
static void settle(void)
{
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
}

static void set_state(lv_obj_t * obj, lv_state_t state)
{
    lv_obj_add_state(obj, state);
    settle();
}

static lv_color_t bg_of(lv_obj_t * obj)
{
    return lv_obj_get_style_bg_color(obj, LV_PART_MAIN);
}

static lv_opa_t bg_opa_of(lv_obj_t * obj)
{
    return lv_obj_get_style_bg_opa(obj, LV_PART_MAIN);
}

/* What a variant looks like with the given colours. */
typedef struct {
    trlv_button_t variant;
    bool opaque;
    lv_color_t container;
    lv_color_t content;
    int32_t border_width;
} variant_case_t;

static void variant_cases(const trlv_color_tokens_t * c, variant_case_t out[TRLV_BUTTON_COUNT])
{
    out[0] = (variant_case_t) { TRLV_BUTTON_FILLED, true, c->primary, c->on_primary, 0 };
    out[1] = (variant_case_t) { TRLV_BUTTON_TONAL, true, c->secondary_container, c->on_secondary_container, 0 };
    out[2] = (variant_case_t) { TRLV_BUTTON_OUTLINED, false, c->primary, c->primary, tokens_regular.border_width };
    out[3] = (variant_case_t) { TRLV_BUTTON_TEXT, false, c->primary, c->primary, 0 };
}

static void assert_default(lv_obj_t * btn, const variant_case_t * v, const trlv_color_tokens_t * c)
{
    if(v->opaque) {
        ASSERT_COLOR(bg_of(btn), v->container);
        TRLV_ASSERT_INT(bg_opa_of(btn), LV_OPA_COVER);
    }
    else {
        TRLV_ASSERT_INT(bg_opa_of(btn), LV_OPA_TRANSP);
    }
    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), v->content);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(btn, LV_PART_MAIN), v->border_width);
    if(v->border_width > 0) {
        ASSERT_COLOR(lv_obj_get_style_border_color(btn, LV_PART_MAIN), c->outline);
    }
}

/* The state layer: pre-mixed on an opaque container, the content colour at
 * the state opacity on a transparent one. */
static void assert_layer(lv_obj_t * btn, const variant_case_t * v, lv_opa_t opa)
{
    if(v->opaque) {
        ASSERT_COLOR(bg_of(btn), lv_color_mix(v->content, v->container, opa));
        TRLV_ASSERT_INT(bg_opa_of(btn), LV_OPA_COVER);
    }
    else {
        ASSERT_COLOR(bg_of(btn), v->content);
        TRLV_ASSERT_INT(bg_opa_of(btn), opa);
    }
    /* The content keeps its colour. */
    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), v->content);
}

static void every_variant_default(lv_obj_t * screen)
{
    variant_case_t cases[TRLV_BUTTON_COUNT];
    variant_cases(&tokens_light, cases);
    for(int i = 0; i < TRLV_BUTTON_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_button_variant(btn, cases[i].variant);
        assert_default(btn, &cases[i], &tokens_light);
    }
}

static void every_variant_pressed(lv_obj_t * screen)
{
    variant_case_t cases[TRLV_BUTTON_COUNT];
    variant_cases(&tokens_light, cases);
    for(int i = 0; i < TRLV_BUTTON_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_button_variant(btn, cases[i].variant);
        set_state(btn, LV_STATE_PRESSED);
        assert_layer(btn, &cases[i], tokens_light.state.pressed);
    }
}

/* Focused by keypad or encoder: the focus layer and the theme's ring. */
static void every_variant_focused(lv_obj_t * screen)
{
    variant_case_t cases[TRLV_BUTTON_COUNT];
    variant_cases(&tokens_light, cases);
    for(int i = 0; i < TRLV_BUTTON_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_button_variant(btn, cases[i].variant);
        set_state(btn, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
        assert_layer(btn, &cases[i], tokens_light.state.focus);
        TRLV_ASSERT_INT(lv_obj_get_style_outline_width(btn, LV_PART_MAIN), tokens_regular.focus_width);
    }
}

/* Disabled: content on_surface at 38 %; an opaque container on_surface at
 * 12 %, a transparent one stays transparent; the outlined border on_surface
 * at 12 %. */
static void every_variant_disabled(lv_obj_t * screen)
{
    const trlv_state_tokens_t * s = &tokens_light.state;
    variant_case_t cases[TRLV_BUTTON_COUNT];
    variant_cases(&tokens_light, cases);
    for(int i = 0; i < TRLV_BUTTON_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_button_variant(btn, cases[i].variant);
        set_state(btn, LV_STATE_DISABLED);

        ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_light.on_surface);
        TRLV_ASSERT_INT(lv_obj_get_style_text_opa(btn, LV_PART_MAIN), s->disabled_content);
        if(cases[i].opaque) {
            ASSERT_COLOR(bg_of(btn), tokens_light.on_surface);
            TRLV_ASSERT_INT(bg_opa_of(btn), s->disabled_container);
        }
        else {
            TRLV_ASSERT_INT(bg_opa_of(btn), LV_OPA_TRANSP);
        }
        TRLV_ASSERT_INT(lv_obj_get_style_border_width(btn, LV_PART_MAIN), cases[i].border_width);
        ASSERT_COLOR(lv_obj_get_style_border_color(btn, LV_PART_MAIN), tokens_light.on_surface);
        TRLV_ASSERT_INT(lv_obj_get_style_border_opa(btn, LV_PART_MAIN), s->disabled_container);
    }
}

/* Shape, height and text role stay the theme's. */
static void keeps_the_theme_shape(lv_obj_t * screen)
{
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_button_variant(btn, TRLV_BUTTON_OUTLINED);
    settle();

    TRLV_ASSERT_INT(lv_obj_get_style_radius(btn, LV_PART_MAIN), tokens_regular.radius[TRLV_RADIUS_FULL]);
    TRLV_ASSERT_HEIGHT(btn, tokens_regular.control_height[TRLV_SIZE_MD]);
    TRLV_ASSERT_INT(lv_obj_get_style_text_font(label, LV_PART_MAIN) ==
                    tokens_regular.text[TRLV_TEXT_LABEL_LARGE].font, 1);
}

/* A second call replaces all four styles: back to FILLED looks like the
 * theme's default in every state. */
static void a_second_call_replaces_the_first(lv_obj_t * screen)
{
    variant_case_t cases[TRLV_BUTTON_COUNT];
    variant_cases(&tokens_light, cases);
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_button_variant(btn, TRLV_BUTTON_TEXT);
    trlv_button_variant(btn, TRLV_BUTTON_FILLED);
    assert_default(btn, &cases[TRLV_BUTTON_FILLED], &tokens_light);

    set_state(btn, LV_STATE_PRESSED);
    assert_layer(btn, &cases[TRLV_BUTTON_FILLED], tokens_light.state.pressed);
}

/* Variants recolour on a colour-mode switch, state layers included. */
static void variants_recolour(lv_obj_t * screen)
{
    variant_case_t cases[TRLV_BUTTON_COUNT];
    variant_cases(&tokens_dark, cases);
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_button_variant(btn, TRLV_BUTTON_TONAL);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();
    assert_default(btn, &cases[TRLV_BUTTON_TONAL], &tokens_dark);

    set_state(btn, LV_STATE_PRESSED);
    assert_layer(btn, &cases[TRLV_BUTTON_TONAL], tokens_dark.state.pressed);
}

/* A value outside the enum changes nothing. */
static void invalid_variant_changes_nothing(lv_obj_t * screen)
{
    variant_case_t cases[TRLV_BUTTON_COUNT];
    variant_cases(&tokens_light, cases);
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_button_variant(btn, TRLV_BUTTON_TONAL);
    trlv_button_variant(btn, TRLV_BUTTON_COUNT);

    assert_default(btn, &cases[TRLV_BUTTON_TONAL], &tokens_light);
}

int main(void)
{
    TRLV_TEST_RUN(every_variant_default);
    TRLV_TEST_RUN(every_variant_pressed);
    TRLV_TEST_RUN(every_variant_focused);
    TRLV_TEST_RUN(every_variant_disabled);
    TRLV_TEST_RUN(keeps_the_theme_shape);
    TRLV_TEST_RUN(a_second_call_replaces_the_first);
    TRLV_TEST_RUN(variants_recolour);
    TRLV_TEST_RUN(invalid_variant_changes_nothing);
    return trlv_test_finish();
}
