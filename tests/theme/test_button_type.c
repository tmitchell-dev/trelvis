/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_apply_button_type: the MD3 button types in their default, pressed,
 * focused and disabled states, over the theme's filled button, and the
 * toggle types selected (checked) in each of those states.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

static lv_obj_t * label;

/* A checkable button with a label, after trlv_init with `color`. */
static lv_obj_t * button(lv_obj_t * screen, const trlv_color_tokens_t * color)
{
    trlv_init(lv_obj_get_display(screen), color, &tokens_regular);
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_set_checkable(btn, true);
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

/* What a type looks like with the given colours. */
typedef struct {
    trlv_button_type_t type;
    bool opaque;
    lv_color_t container;
    lv_color_t content;
    int32_t border_width;
    lv_color_t border;
} type_case_t;

static void type_cases(const trlv_color_tokens_t * c, type_case_t out[TRLV_BUTTON_COUNT])
{
    int32_t bw = tokens_regular.border_width;
    out[0] = (type_case_t) { TRLV_BUTTON_FILLED, true, c->primary, c->on_primary, 0, c->outline };
    out[1] = (type_case_t) { TRLV_BUTTON_TONAL, true, c->secondary_container, c->on_secondary_container, 0,
                             c->outline };
    out[2] = (type_case_t) { TRLV_BUTTON_OUTLINED, false, c->primary, c->primary, bw, c->outline };
    out[3] = (type_case_t) { TRLV_BUTTON_TEXT, false, c->primary, c->primary, 0, c->outline };
    /* Toggle types, unselected. */
    out[4] = (type_case_t) { TRLV_BUTTON_FILLED_TOGGLE, true, c->surface_container, c->on_surface_variant, 0,
                             c->outline };
    out[5] = (type_case_t) { TRLV_BUTTON_TONAL_TOGGLE, true, c->secondary_container,
                             c->on_secondary_container, 0, c->outline };
    out[6] = (type_case_t) { TRLV_BUTTON_OUTLINED_TOGGLE, false, c->on_surface_variant, c->on_surface_variant,
                             bw, c->outline_variant };
}

/* The toggle types selected: always an opaque container, no border. */
#define TOGGLE_COUNT 3

static void selected_cases(const trlv_color_tokens_t * c, type_case_t out[TOGGLE_COUNT])
{
    out[0] = (type_case_t) { TRLV_BUTTON_FILLED_TOGGLE, true, c->primary, c->on_primary, 0, c->outline };
    out[1] = (type_case_t) { TRLV_BUTTON_TONAL_TOGGLE, true, c->secondary, c->on_secondary, 0, c->outline };
    out[2] = (type_case_t) { TRLV_BUTTON_OUTLINED_TOGGLE, true, c->inverse_surface, c->inverse_on_surface, 0,
                             c->outline };
}

static void assert_default(lv_obj_t * btn, const type_case_t * v, const trlv_color_tokens_t * c)
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
        ASSERT_COLOR(lv_obj_get_style_border_color(btn, LV_PART_MAIN), v->border);
    }
    LV_UNUSED(c);
}

/* The state layer: pre-mixed on an opaque container, the content colour at
 * the state opacity on a transparent one. */
static void assert_layer(lv_obj_t * btn, const type_case_t * v, lv_opa_t opa)
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

static void every_type_default(lv_obj_t * screen)
{
    type_case_t cases[TRLV_BUTTON_COUNT];
    type_cases(&tokens_light, cases);
    for(int i = 0; i < TRLV_BUTTON_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_apply_button_type(btn, cases[i].type);
        assert_default(btn, &cases[i], &tokens_light);
    }
}

static void every_type_pressed(lv_obj_t * screen)
{
    type_case_t cases[TRLV_BUTTON_COUNT];
    type_cases(&tokens_light, cases);
    for(int i = 0; i < TRLV_BUTTON_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_apply_button_type(btn, cases[i].type);
        set_state(btn, LV_STATE_PRESSED);
        assert_layer(btn, &cases[i], tokens_light.state.pressed);
    }
}

/* Focused by keypad or encoder: the focus layer and the theme's ring. */
static void every_type_focused(lv_obj_t * screen)
{
    type_case_t cases[TRLV_BUTTON_COUNT];
    type_cases(&tokens_light, cases);
    for(int i = 0; i < TRLV_BUTTON_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_apply_button_type(btn, cases[i].type);
        set_state(btn, LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
        assert_layer(btn, &cases[i], tokens_light.state.focus);
        TRLV_ASSERT_INT(lv_obj_get_style_outline_width(btn, LV_PART_MAIN), tokens_regular.focus_width);
    }
}

/* Disabled: content on_surface at 38 %; an opaque container on_surface at
 * 12 %, a transparent one stays transparent; the outlined border on_surface
 * at 12 %. */
static void every_type_disabled(lv_obj_t * screen)
{
    const trlv_state_tokens_t * s = &tokens_light.state;
    type_case_t cases[TRLV_BUTTON_COUNT];
    type_cases(&tokens_light, cases);
    for(int i = 0; i < TRLV_BUTTON_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_apply_button_type(btn, cases[i].type);
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

/* Selected: the toggle's selected container and content, no border. */
static void every_toggle_selected(lv_obj_t * screen)
{
    type_case_t cases[TOGGLE_COUNT];
    selected_cases(&tokens_light, cases);
    for(int i = 0; i < TOGGLE_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_apply_button_type(btn, cases[i].type);
        set_state(btn, LV_STATE_CHECKED);
        assert_default(btn, &cases[i], &tokens_light);
    }
}

/* Selected and pressed or focused: the state layer over the selected
 * container, not the unselected one. */
static void every_toggle_selected_pressed_and_focused(lv_obj_t * screen)
{
    type_case_t cases[TOGGLE_COUNT];
    selected_cases(&tokens_light, cases);
    for(int i = 0; i < TOGGLE_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_apply_button_type(btn, cases[i].type);
        set_state(btn, LV_STATE_CHECKED | LV_STATE_PRESSED);
        assert_layer(btn, &cases[i], tokens_light.state.pressed);

        btn = button(screen, &tokens_light);
        trlv_apply_button_type(btn, cases[i].type);
        set_state(btn, LV_STATE_CHECKED | LV_STATE_FOCUSED | LV_STATE_FOCUS_KEY);
        assert_layer(btn, &cases[i], tokens_light.state.focus);
    }
}

/* Selected and disabled: an on_surface container at 12 % in every toggle
 * type, the outlined one included, with no border. */
static void every_toggle_selected_disabled(lv_obj_t * screen)
{
    const trlv_state_tokens_t * s = &tokens_light.state;
    type_case_t cases[TOGGLE_COUNT];
    selected_cases(&tokens_light, cases);
    for(int i = 0; i < TOGGLE_COUNT; i++) {
        lv_obj_t * btn = button(screen, &tokens_light);
        trlv_apply_button_type(btn, cases[i].type);
        set_state(btn, LV_STATE_CHECKED | LV_STATE_DISABLED);

        ASSERT_COLOR(bg_of(btn), tokens_light.on_surface);
        TRLV_ASSERT_INT(bg_opa_of(btn), s->disabled_container);
        ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_light.on_surface);
        TRLV_ASSERT_INT(lv_obj_get_style_text_opa(btn, LV_PART_MAIN), s->disabled_content);
        TRLV_ASSERT_INT(lv_obj_get_style_border_width(btn, LV_PART_MAIN), 0);
    }
}

/* Only a toggle type has a selected look: a checked button of another
 * type looks as it does unchecked. The theme removes the parent theme's
 * checked colour, and a plain type replacing a toggle removes the toggle's
 * selected styles. */
static void other_types_have_no_selected_look(lv_obj_t * screen)
{
    type_case_t cases[TRLV_BUTTON_COUNT];
    type_cases(&tokens_light, cases);

    lv_obj_t * btn = button(screen, &tokens_light);
    set_state(btn, LV_STATE_CHECKED);
    assert_default(btn, &cases[TRLV_BUTTON_FILLED], &tokens_light);

    btn = button(screen, &tokens_light);
    trlv_apply_button_type(btn, TRLV_BUTTON_TONAL_TOGGLE);
    trlv_apply_button_type(btn, TRLV_BUTTON_TONAL);
    set_state(btn, LV_STATE_CHECKED);
    assert_default(btn, &cases[TRLV_BUTTON_TONAL], &tokens_light);
    set_state(btn, LV_STATE_PRESSED);
    assert_layer(btn, &cases[TRLV_BUTTON_TONAL], tokens_light.state.pressed);
}

/* One toggle type replaces another in every selected state. */
static void a_toggle_replaces_a_toggle(lv_obj_t * screen)
{
    type_case_t cases[TOGGLE_COUNT];
    selected_cases(&tokens_light, cases);
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_apply_button_type(btn, TRLV_BUTTON_OUTLINED_TOGGLE);
    trlv_apply_button_type(btn, TRLV_BUTTON_FILLED_TOGGLE);
    set_state(btn, LV_STATE_CHECKED);
    assert_default(btn, &cases[0], &tokens_light);
    set_state(btn, LV_STATE_PRESSED);
    assert_layer(btn, &cases[0], tokens_light.state.pressed);
}

/* The selected look recolours on a colour-mode switch. */
static void selected_recolours(lv_obj_t * screen)
{
    type_case_t cases[TOGGLE_COUNT];
    selected_cases(&tokens_dark, cases);
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_apply_button_type(btn, TRLV_BUTTON_OUTLINED_TOGGLE);
    set_state(btn, LV_STATE_CHECKED);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();
    assert_default(btn, &cases[2], &tokens_dark);
}

/* Shape, height and text role stay the theme's. */
static void keeps_the_theme_shape(lv_obj_t * screen)
{
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_apply_button_type(btn, TRLV_BUTTON_OUTLINED);
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
    type_case_t cases[TRLV_BUTTON_COUNT];
    type_cases(&tokens_light, cases);
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_apply_button_type(btn, TRLV_BUTTON_TEXT);
    trlv_apply_button_type(btn, TRLV_BUTTON_FILLED);
    assert_default(btn, &cases[TRLV_BUTTON_FILLED], &tokens_light);

    set_state(btn, LV_STATE_PRESSED);
    assert_layer(btn, &cases[TRLV_BUTTON_FILLED], tokens_light.state.pressed);
}

/* Types recolour on a colour-mode switch, state layers included. */
static void types_recolour(lv_obj_t * screen)
{
    type_case_t cases[TRLV_BUTTON_COUNT];
    type_cases(&tokens_dark, cases);
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_apply_button_type(btn, TRLV_BUTTON_TONAL);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();
    assert_default(btn, &cases[TRLV_BUTTON_TONAL], &tokens_dark);

    set_state(btn, LV_STATE_PRESSED);
    assert_layer(btn, &cases[TRLV_BUTTON_TONAL], tokens_dark.state.pressed);
}

/* A value outside the enum changes nothing. */
static void invalid_type_changes_nothing(lv_obj_t * screen)
{
    type_case_t cases[TRLV_BUTTON_COUNT];
    type_cases(&tokens_light, cases);
    lv_obj_t * btn = button(screen, &tokens_light);
    trlv_apply_button_type(btn, TRLV_BUTTON_TONAL);
    trlv_apply_button_type(btn, TRLV_BUTTON_COUNT);

    assert_default(btn, &cases[TRLV_BUTTON_TONAL], &tokens_light);
}

int main(void)
{
    TRLV_TEST_RUN(every_type_default);
    TRLV_TEST_RUN(every_type_pressed);
    TRLV_TEST_RUN(every_type_focused);
    TRLV_TEST_RUN(every_type_disabled);
    TRLV_TEST_RUN(every_toggle_selected);
    TRLV_TEST_RUN(every_toggle_selected_pressed_and_focused);
    TRLV_TEST_RUN(every_toggle_selected_disabled);
    TRLV_TEST_RUN(other_types_have_no_selected_look);
    TRLV_TEST_RUN(a_toggle_replaces_a_toggle);
    TRLV_TEST_RUN(selected_recolours);
    TRLV_TEST_RUN(keeps_the_theme_shape);
    TRLV_TEST_RUN(a_second_call_replaces_the_first);
    TRLV_TEST_RUN(types_recolour);
    TRLV_TEST_RUN(invalid_type_changes_nothing);
    return trlv_test_finish();
}
