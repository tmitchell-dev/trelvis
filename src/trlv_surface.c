/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_surface.c
 * Surfaces, custom colours, cards and button types: MD3 container
 * colours with their inherited content colour.
 */

#include "trelvis/trlv_surface.h"
#include "trelvis/trlv_theme.h"
#include "trlv_internal.h"
#include "trlv_style_group.h"

/*---------------------------------------------------------------------------
 * Token styles, written by trlv_surface_styles_build on trlv_init and on
 * each colour-mode switch. See trlv_internal.h for how token styles work.
 *-------------------------------------------------------------------------*/

/* Surface: bg_color, bg_opa, text_color (inherited), end marker. */
#define SURFACE_PROPS 4

static lv_style_const_prop_t surface_props[TRLV_SURFACE_COUNT][SURFACE_PROPS];

static LV_STYLE_CONST_INIT(surface_surface, surface_props[TRLV_SURFACE_SURFACE]);
static LV_STYLE_CONST_INIT(surface_container_lowest, surface_props[TRLV_SURFACE_CONTAINER_LOWEST]);
static LV_STYLE_CONST_INIT(surface_container_low, surface_props[TRLV_SURFACE_CONTAINER_LOW]);
static LV_STYLE_CONST_INIT(surface_container, surface_props[TRLV_SURFACE_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_container_high, surface_props[TRLV_SURFACE_CONTAINER_HIGH]);
static LV_STYLE_CONST_INIT(surface_container_highest, surface_props[TRLV_SURFACE_CONTAINER_HIGHEST]);
static LV_STYLE_CONST_INIT(surface_primary_container, surface_props[TRLV_SURFACE_PRIMARY_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_secondary_container, surface_props[TRLV_SURFACE_SECONDARY_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_tertiary_container, surface_props[TRLV_SURFACE_TERTIARY_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_error_container, surface_props[TRLV_SURFACE_ERROR_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_inverse, surface_props[TRLV_SURFACE_INVERSE]);
static LV_STYLE_CONST_INIT(surface_dim, surface_props[TRLV_SURFACE_DIM]);
static LV_STYLE_CONST_INIT(surface_bright, surface_props[TRLV_SURFACE_BRIGHT]);

/* Indexed by trlv_surface_t. */
static const lv_style_t * const surface_styles[] = {
    &surface_surface, &surface_container_lowest, &surface_container_low,
    &surface_container, &surface_container_high, &surface_container_highest,
    &surface_primary_container, &surface_secondary_container,
    &surface_tertiary_container, &surface_error_container, &surface_inverse,
    &surface_dim, &surface_bright
};
TRLV_STATIC_ASSERT(sizeof(surface_styles) / sizeof(surface_styles[0]) == TRLV_SURFACE_COUNT,
                   surface_styles_cover_the_enum);

/* Card: the tone (bg_color, bg_opa, text_color), radius, then the border
 * (width, colour, opa) and the shadow (width, spread, offset x and y, opa,
 * colour), end marker. Every card sets every property, so a card replaces
 * another card fully and overrides a theme border or shadow on the object. */
#define CARD_PROPS 14

static lv_style_const_prop_t card_props[TRLV_CARD_COUNT][CARD_PROPS];

static LV_STYLE_CONST_INIT(card_filled, card_props[TRLV_CARD_FILLED]);
static LV_STYLE_CONST_INIT(card_elevated, card_props[TRLV_CARD_ELEVATED]);
static LV_STYLE_CONST_INIT(card_outlined, card_props[TRLV_CARD_OUTLINED]);

/* Indexed by trlv_card_type_t. */
static const lv_style_t * const card_styles[] = { &card_filled, &card_elevated, &card_outlined };
TRLV_STATIC_ASSERT(sizeof(card_styles) / sizeof(card_styles[0]) == TRLV_CARD_COUNT,
                   card_styles_cover_the_enum);

/* Button types: one style per type at each of four selectors. The
 * theme's filled-button styles stay on the button; LVGL resolves a property
 * by state first, so each type sets the same properties as the theme at
 * every state it styles, and wins at each because it is added later.
 *
 * Default: bg_color, bg_opa, text_color, border_width, border_color,
 * border_opa. Pressed and focus: bg_color, bg_opa. Disabled: bg_color,
 * bg_opa, text_color, text_opa, border_color, border_opa. Plus end markers.
 *
 * A toggle type's styles at these selectors are its unselected look. Where
 * that is the same as a common type's, the toggle shares its styles rather
 * than holding a copy: a tonal toggle unselected is a tonal button, and
 * disabled doesn't depend on the container, so every toggle shares its
 * common type's disabled style. A shared style still replaces the others
 * of its group, so types replace each other as before. */
#define BUTTON_PROPS 7
#define BUTTON_STATE_PROPS 3
#define BUTTON_DISABLED_PROPS 7

/* The distinct looks at rest, pressed and focused. */
enum {
    LOOK_FILLED, LOOK_TONAL, LOOK_OUTLINED, LOOK_TEXT,
    LOOK_FILLED_TOGGLE, LOOK_OUTLINED_TOGGLE,
    LOOK_COUNT
};
#define LOOK_COMMON_COUNT (LOOK_TEXT + 1)   /* the looks with a disabled style of their own */

static lv_style_const_prop_t button_props[LOOK_COUNT][BUTTON_PROPS];
static lv_style_const_prop_t button_pressed_props[LOOK_COUNT][BUTTON_STATE_PROPS];
static lv_style_const_prop_t button_focus_props[LOOK_COUNT][BUTTON_STATE_PROPS];
static lv_style_const_prop_t button_disabled_props[LOOK_COMMON_COUNT][BUTTON_DISABLED_PROPS];

#define BUTTON_STYLES(name_, look_)                                                             \
    static LV_STYLE_CONST_INIT(button_##name_, button_props[look_]);                           \
    static LV_STYLE_CONST_INIT(button_##name_##_pressed, button_pressed_props[look_]);         \
    static LV_STYLE_CONST_INIT(button_##name_##_focus, button_focus_props[look_])
#define BUTTON_DISABLED_STYLE(name_, look_) \
    static LV_STYLE_CONST_INIT(button_##name_##_disabled, button_disabled_props[look_])

BUTTON_STYLES(filled, LOOK_FILLED);
BUTTON_STYLES(tonal, LOOK_TONAL);
BUTTON_STYLES(outlined, LOOK_OUTLINED);
BUTTON_STYLES(text, LOOK_TEXT);
BUTTON_STYLES(filled_toggle, LOOK_FILLED_TOGGLE);
BUTTON_STYLES(outlined_toggle, LOOK_OUTLINED_TOGGLE);
BUTTON_DISABLED_STYLE(filled, LOOK_FILLED);
BUTTON_DISABLED_STYLE(tonal, LOOK_TONAL);
BUTTON_DISABLED_STYLE(outlined, LOOK_OUTLINED);
BUTTON_DISABLED_STYLE(text, LOOK_TEXT);

/* Indexed by trlv_button_type_t, one list per selector. */
static const lv_style_t * const button_styles[] = {
    &button_filled, &button_tonal, &button_outlined, &button_text,
    &button_filled_toggle, &button_tonal, &button_outlined_toggle
};
static const lv_style_t * const button_pressed_styles[] = {
    &button_filled_pressed, &button_tonal_pressed, &button_outlined_pressed, &button_text_pressed,
    &button_filled_toggle_pressed, &button_tonal_pressed, &button_outlined_toggle_pressed
};
static const lv_style_t * const button_focus_styles[] = {
    &button_filled_focus, &button_tonal_focus, &button_outlined_focus, &button_text_focus,
    &button_filled_toggle_focus, &button_tonal_focus, &button_outlined_toggle_focus
};
static const lv_style_t * const button_disabled_styles[] = {
    &button_filled_disabled, &button_tonal_disabled, &button_outlined_disabled, &button_text_disabled,
    &button_filled_disabled, &button_tonal_disabled, &button_outlined_disabled
};
TRLV_STATIC_ASSERT(sizeof(button_styles) / sizeof(button_styles[0]) == TRLV_BUTTON_COUNT,
                   button_styles_cover_the_enum);
TRLV_STATIC_ASSERT(sizeof(button_disabled_styles) / sizeof(button_disabled_styles[0]) == TRLV_BUTTON_COUNT,
                   button_disabled_styles_cover_the_enum);

static const trlv_style_group_t button_group = TRLV_STYLE_GROUP_LIST(button_styles);
static const trlv_style_group_t button_pressed_group = TRLV_STYLE_GROUP_LIST(button_pressed_styles);
static const trlv_style_group_t button_focus_group = TRLV_STYLE_GROUP_LIST(button_focus_styles);
static const trlv_style_group_t button_disabled_group = TRLV_STYLE_GROUP_LIST(button_disabled_styles);

/* Toggle types, selected (LV_STATE_CHECKED): one style per toggle type at
 * three more selectors, each combined with checked, and one disabled style
 * they share. LVGL resolves a property by the highest matching state, so
 * PRESSED (or FOCUS_KEY, DISABLED) alone would win over CHECKED: each needs
 * its own CHECKED | state style.
 *
 * Selected: bg_color, bg_opa, text_color, border_width (0: a selected
 * outlined toggle has no border). Pressed and focus: bg_color, bg_opa.
 * Disabled: bg_color, bg_opa, text_color, text_opa. Plus end markers. */
#define BUTTON_TOGGLE_FIRST TRLV_BUTTON_FILLED_TOGGLE
#define BUTTON_TOGGLE_COUNT (TRLV_BUTTON_COUNT - BUTTON_TOGGLE_FIRST)
#define BUTTON_CHECKED_PROPS 5
#define BUTTON_CHECKED_DISABLED_PROPS 5

static lv_style_const_prop_t button_checked_props[BUTTON_TOGGLE_COUNT][BUTTON_CHECKED_PROPS];
static lv_style_const_prop_t button_checked_pressed_props[BUTTON_TOGGLE_COUNT][BUTTON_STATE_PROPS];
static lv_style_const_prop_t button_checked_focus_props[BUTTON_TOGGLE_COUNT][BUTTON_STATE_PROPS];
static lv_style_const_prop_t button_checked_disabled_props[BUTTON_CHECKED_DISABLED_PROPS];

#define BUTTON_CHECKED_STYLES(name_, index_)                                                              \
    static LV_STYLE_CONST_INIT(button_##name_##_checked, button_checked_props[index_]);                   \
    static LV_STYLE_CONST_INIT(button_##name_##_checked_pressed, button_checked_pressed_props[index_]);   \
    static LV_STYLE_CONST_INIT(button_##name_##_checked_focus, button_checked_focus_props[index_])

BUTTON_CHECKED_STYLES(filled_toggle, TRLV_BUTTON_FILLED_TOGGLE - BUTTON_TOGGLE_FIRST);
BUTTON_CHECKED_STYLES(tonal_toggle, TRLV_BUTTON_TONAL_TOGGLE - BUTTON_TOGGLE_FIRST);
BUTTON_CHECKED_STYLES(outlined_toggle, TRLV_BUTTON_OUTLINED_TOGGLE - BUTTON_TOGGLE_FIRST);
static LV_STYLE_CONST_INIT(button_checked_disabled, button_checked_disabled_props);

/* Indexed by trlv_button_type_t - BUTTON_TOGGLE_FIRST, one list per selector. */
static const lv_style_t * const button_checked_styles[] = {
    &button_filled_toggle_checked, &button_tonal_toggle_checked, &button_outlined_toggle_checked
};
static const lv_style_t * const button_checked_pressed_styles[] = {
    &button_filled_toggle_checked_pressed, &button_tonal_toggle_checked_pressed,
    &button_outlined_toggle_checked_pressed
};
static const lv_style_t * const button_checked_focus_styles[] = {
    &button_filled_toggle_checked_focus, &button_tonal_toggle_checked_focus,
    &button_outlined_toggle_checked_focus
};
static const lv_style_t * const button_checked_disabled_styles[] = { &button_checked_disabled };
TRLV_STATIC_ASSERT(sizeof(button_checked_styles) / sizeof(button_checked_styles[0]) == BUTTON_TOGGLE_COUNT,
                   button_checked_styles_cover_the_toggles);

static const trlv_style_group_t button_checked_group = TRLV_STYLE_GROUP_LIST(button_checked_styles);
static const trlv_style_group_t button_checked_pressed_group =
    TRLV_STYLE_GROUP_LIST(button_checked_pressed_styles);
static const trlv_style_group_t button_checked_focus_group = TRLV_STYLE_GROUP_LIST(button_checked_focus_styles);
static const trlv_style_group_t button_checked_disabled_group =
    TRLV_STYLE_GROUP_LIST(button_checked_disabled_styles);

#define BUTTON_CHECKED            (LV_PART_MAIN | LV_STATE_CHECKED)
#define BUTTON_CHECKED_PRESSED    (LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_PRESSED)
#define BUTTON_CHECKED_FOCUS      (LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_FOCUS_KEY)
#define BUTTON_CHECKED_DISABLED   (LV_PART_MAIN | LV_STATE_CHECKED | LV_STATE_DISABLED)

/* Divider (trlv_divider_create in trlv_layout.c): bg_color, bg_opa, the line's
 * thickness (height when horizontal, width when vertical), outline_width 0
 * so the debug outline doesn't hide the line, end marker. */
#define DIVIDER_PROPS 5

static lv_style_const_prop_t divider_props[2][DIVIDER_PROPS];

static LV_STYLE_CONST_INIT(divider_horizontal, divider_props[0]);
static LV_STYLE_CONST_INIT(divider_vertical, divider_props[1]);

/* Custom colours: two styles (colour, container) for each of the first
 * TRLV_CUSTOM_COLOR_MAX indexes. LV_STYLE_CONST_INIT defines one variable,
 * so the styles are spelled out per index, up to a fixed cap. */
#if TRLV_CUSTOM_COLOR_MAX < 0 || TRLV_CUSTOM_COLOR_MAX > 8
#error "TRLV_CUSTOM_COLOR_MAX must be between 0 and 8"
#endif

#if TRLV_CUSTOM_COLOR_MAX > 0
static lv_style_const_prop_t custom_props[TRLV_CUSTOM_COLOR_MAX][TRLV_CUSTOM_SURFACE_COUNT][SURFACE_PROPS];
#endif

/* CUSTOM_EACH(X) expands X(i) for every index below TRLV_CUSTOM_COLOR_MAX. */
#if TRLV_CUSTOM_COLOR_MAX > 0
#define CUSTOM_0(X) X(0)
#else
#define CUSTOM_0(X)
#endif
#if TRLV_CUSTOM_COLOR_MAX > 1
#define CUSTOM_1(X) X(1)
#else
#define CUSTOM_1(X)
#endif
#if TRLV_CUSTOM_COLOR_MAX > 2
#define CUSTOM_2(X) X(2)
#else
#define CUSTOM_2(X)
#endif
#if TRLV_CUSTOM_COLOR_MAX > 3
#define CUSTOM_3(X) X(3)
#else
#define CUSTOM_3(X)
#endif
#if TRLV_CUSTOM_COLOR_MAX > 4
#define CUSTOM_4(X) X(4)
#else
#define CUSTOM_4(X)
#endif
#if TRLV_CUSTOM_COLOR_MAX > 5
#define CUSTOM_5(X) X(5)
#else
#define CUSTOM_5(X)
#endif
#if TRLV_CUSTOM_COLOR_MAX > 6
#define CUSTOM_6(X) X(6)
#else
#define CUSTOM_6(X)
#endif
#if TRLV_CUSTOM_COLOR_MAX > 7
#define CUSTOM_7(X) X(7)
#else
#define CUSTOM_7(X)
#endif
#define CUSTOM_EACH(X) \
    CUSTOM_0(X) CUSTOM_1(X) CUSTOM_2(X) CUSTOM_3(X) CUSTOM_4(X) CUSTOM_5(X) CUSTOM_6(X) CUSTOM_7(X)

#define CUSTOM_DEFINE(i)                                                                     \
    static LV_STYLE_CONST_INIT(custom_color_##i, custom_props[i][TRLV_CUSTOM_SURFACE_COLOR]); \
    static LV_STYLE_CONST_INIT(custom_container_##i, custom_props[i][TRLV_CUSTOM_SURFACE_CONTAINER]);
#define CUSTOM_POINTERS(i) &custom_color_##i, &custom_container_##i,

CUSTOM_EACH(CUSTOM_DEFINE)

#if TRLV_CUSTOM_COLOR_MAX > 0
/* Indexed by index * TRLV_CUSTOM_SURFACE_COUNT + trlv_custom_surface_t. */
static const lv_style_t * const custom_styles[] = { CUSTOM_EACH(CUSTOM_POINTERS) };
TRLV_STATIC_ASSERT(sizeof(custom_styles) / sizeof(custom_styles[0])
                   == TRLV_CUSTOM_COLOR_MAX * TRLV_CUSTOM_SURFACE_COUNT,
                   custom_styles_cover_every_index);
#endif

/* The fill group: every style that sets an object's container colour.
 * trlv_apply_surface, trlv_apply_surface_custom and trlv_apply_card replace
 * each other. */
static const lv_style_t * const fill_styles[] = {
    &surface_surface, &surface_container_lowest, &surface_container_low,
    &surface_container, &surface_container_high, &surface_container_highest,
    &surface_primary_container, &surface_secondary_container,
    &surface_tertiary_container, &surface_error_container, &surface_inverse,
    &card_filled, &card_elevated, &card_outlined,
    CUSTOM_EACH(CUSTOM_POINTERS)
};
static const trlv_style_group_t fill_group = TRLV_STYLE_GROUP_LIST(fill_styles);

/* Write a tone: an opaque container colour and its inherited content colour.
 * Returns the next free entry. */
static lv_style_const_prop_t * put_tone(lv_style_const_prop_t * entry, lv_color_t container,
                                        lv_color_t content)
{
    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, container);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, LV_OPA_COVER);
    trlv_prop_set_color(entry++, LV_STYLE_TEXT_COLOR, content);
    return entry;
}

static void build_surface(trlv_surface_t surface, lv_color_t container, lv_color_t content)
{
    trlv_prop_set_end(put_tone(surface_props[surface], container, content));
}

/* A card: a surface tone, radius MD, an optional border and shadow. */
static void build_card(trlv_card_type_t card, lv_color_t container, int32_t border_width,
                       const trlv_shadow_t * shadow, const trlv_color_tokens_t * color,
                       const trlv_layout_tokens_t * layout)
{
    static const trlv_shadow_t no_shadow = { 0, 0, 0, 0, LV_OPA_TRANSP };
    if(shadow == NULL) shadow = &no_shadow;

    lv_style_const_prop_t * entry = put_tone(card_props[card], container, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_MD]);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_WIDTH, border_width);
    trlv_prop_set_color(entry++, LV_STYLE_BORDER_COLOR, color->outline_variant);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_OPA, LV_OPA_COVER);
    trlv_prop_set_num(entry++, LV_STYLE_SHADOW_WIDTH, shadow->blur);
    trlv_prop_set_num(entry++, LV_STYLE_SHADOW_SPREAD, shadow->spread);
    trlv_prop_set_num(entry++, LV_STYLE_SHADOW_OFFSET_X, shadow->offset_x);
    trlv_prop_set_num(entry++, LV_STYLE_SHADOW_OFFSET_Y, shadow->offset_y);
    trlv_prop_set_num(entry++, LV_STYLE_SHADOW_OPA, shadow->opa);
    trlv_prop_set_color(entry++, LV_STYLE_SHADOW_COLOR, color->shadow);
    trlv_prop_set_end(entry);
    LV_ASSERT(entry == &card_props[card][CARD_PROPS - 1]);
}

/* A state layer: the content colour over the container at `opa`. On an
 * opaque container it is pre-mixed, so the content keeps its colour. A
 * transparent container (outlined, text) has nothing to mix with, so the
 * layer is the content colour at `opa`, over whatever is behind. */
static void put_state_layer(lv_style_const_prop_t * entry, bool opaque, lv_color_t container,
                            lv_color_t content, lv_opa_t opa)
{
    if(opaque) {
        trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, trlv_state_layer(container, content, opa));
        trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, LV_OPA_COVER);
    }
    else {
        trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, content);
        trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, opa);
    }
    trlv_prop_set_end(entry);
}

/* One look at rest, pressed and focused, and, for a common type, its
 * disabled style. `opaque` is false for the transparent types;
 * `border_width` is 0 except for the outlined types, whose border is
 * `border`. */
static void build_button(uint32_t look, bool opaque, lv_color_t container, lv_color_t content,
                         int32_t border_width, lv_color_t border, const trlv_color_tokens_t * color)
{
    const trlv_state_tokens_t * state = &color->state;
    lv_style_const_prop_t * entry;

    entry = button_props[look];
    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, container);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, opaque ? LV_OPA_COVER : LV_OPA_TRANSP);
    trlv_prop_set_color(entry++, LV_STYLE_TEXT_COLOR, content);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_WIDTH, border_width);
    trlv_prop_set_color(entry++, LV_STYLE_BORDER_COLOR, border);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_OPA, LV_OPA_COVER);
    trlv_prop_set_end(entry);

    put_state_layer(button_pressed_props[look], opaque, container, content, state->pressed);
    put_state_layer(button_focus_props[look], opaque, container, content, state->focus);

    if(look >= LOOK_COMMON_COUNT) return;

    /* MD3 disabled: content on_surface at 38 %; an opaque container, or
     * the outlined border, on_surface at 12 %. */
    entry = button_disabled_props[look];
    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, opaque ? state->disabled_container : LV_OPA_TRANSP);
    trlv_prop_set_color(entry++, LV_STYLE_TEXT_COLOR, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_TEXT_OPA, state->disabled_content);
    trlv_prop_set_color(entry++, LV_STYLE_BORDER_COLOR, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_OPA, state->disabled_container);
    trlv_prop_set_end(entry);
}

/* A toggle type's selected look, at rest, pressed and focused. Every
 * selected container is opaque, so the state layers are pre-mixed. */
static void build_button_checked(trlv_button_type_t type, lv_color_t container, lv_color_t content,
                                 const trlv_color_tokens_t * color)
{
    const trlv_state_tokens_t * state = &color->state;
    uint32_t i = (uint32_t)type - BUTTON_TOGGLE_FIRST;
    lv_style_const_prop_t * entry;

    entry = button_checked_props[i];
    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, container);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, LV_OPA_COVER);
    trlv_prop_set_color(entry++, LV_STYLE_TEXT_COLOR, content);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_WIDTH, 0);
    trlv_prop_set_end(entry);

    put_state_layer(button_checked_pressed_props[i], true, container, content, state->pressed);
    put_state_layer(button_checked_focus_props[i], true, container, content, state->focus);
}

/* Selected and disabled, shared by every toggle type: MD3 disables a
 * selected button as an opaque container in every type, including the
 * outlined toggle, which has no border when selected. */
static void build_button_checked_disabled(const trlv_color_tokens_t * color)
{
    const trlv_state_tokens_t * state = &color->state;
    lv_style_const_prop_t * entry = button_checked_disabled_props;

    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, state->disabled_container);
    trlv_prop_set_color(entry++, LV_STYLE_TEXT_COLOR, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_TEXT_OPA, state->disabled_content);
    trlv_prop_set_end(entry);
}

static void build_divider(uint32_t index, lv_style_prop_t thickness, const trlv_color_tokens_t * color,
                          const trlv_layout_tokens_t * layout)
{
    lv_style_const_prop_t * entry = divider_props[index];
    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, color->outline_variant);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, LV_OPA_COVER);
    trlv_prop_set_num(entry++, thickness, layout->border_width);
    trlv_prop_set_num(entry++, LV_STYLE_OUTLINE_WIDTH, 0);
    trlv_prop_set_end(entry);
}

const lv_style_t * trlv_divider_style(bool vertical)
{
    return vertical ? &divider_vertical : &divider_horizontal;
}

void trlv_surface_styles_build(const trlv_color_tokens_t * color, const trlv_layout_tokens_t * layout)
{
    build_divider(0, LV_STYLE_HEIGHT, color, layout);
    build_divider(1, LV_STYLE_WIDTH, color, layout);

    build_button(LOOK_FILLED, true, color->primary, color->on_primary, 0, color->outline, color);
    build_button(LOOK_TONAL, true, color->secondary_container, color->on_secondary_container, 0,
                 color->outline, color);
    build_button(LOOK_OUTLINED, false, color->primary, color->primary, layout->border_width,
                 color->outline, color);
    build_button(LOOK_TEXT, false, color->primary, color->primary, 0, color->outline, color);

    /* Toggle buttons, as in MD3's button tokens: unselected (the tonal
     * toggle's is the tonal button), then selected. */
    build_button(LOOK_FILLED_TOGGLE, true, color->surface_container, color->on_surface_variant, 0,
                 color->outline, color);
    build_button(LOOK_OUTLINED_TOGGLE, false, color->on_surface_variant, color->on_surface_variant,
                 layout->border_width, color->outline_variant, color);
    build_button_checked(TRLV_BUTTON_FILLED_TOGGLE, color->primary, color->on_primary, color);
    build_button_checked(TRLV_BUTTON_TONAL_TOGGLE, color->secondary, color->on_secondary, color);
    build_button_checked(TRLV_BUTTON_OUTLINED_TOGGLE, color->inverse_surface, color->inverse_on_surface, color);
    build_button_checked_disabled(color);

    build_card(TRLV_CARD_FILLED, color->surface_container_highest, 0, NULL, color, layout);
    build_card(TRLV_CARD_ELEVATED, color->surface_container_low, 0,
               &layout->elevation[TRLV_ELEVATION_1], color, layout);
    build_card(TRLV_CARD_OUTLINED, color->surface, layout->border_width, NULL, color, layout);

    build_surface(TRLV_SURFACE_SURFACE, color->surface, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER_LOWEST, color->surface_container_lowest, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER_LOW, color->surface_container_low, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER, color->surface_container, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER_HIGH, color->surface_container_high, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER_HIGHEST, color->surface_container_highest, color->on_surface);
    build_surface(TRLV_SURFACE_PRIMARY_CONTAINER, color->primary_container, color->on_primary_container);
    build_surface(TRLV_SURFACE_SECONDARY_CONTAINER, color->secondary_container,
                  color->on_secondary_container);
    build_surface(TRLV_SURFACE_TERTIARY_CONTAINER, color->tertiary_container,
                  color->on_tertiary_container);
    build_surface(TRLV_SURFACE_ERROR_CONTAINER, color->error_container, color->on_error_container);
    build_surface(TRLV_SURFACE_INVERSE, color->inverse_surface, color->inverse_on_surface);
    build_surface(TRLV_SURFACE_DIM, color->surface_dim, color->on_surface);
    build_surface(TRLV_SURFACE_BRIGHT, color->surface_bright, color->on_surface);

#if TRLV_CUSTOM_COLOR_MAX > 0
    /* An index the tokens don't define gets an empty style, so an object
     * that has it shows its other styles rather than a stale colour. */
    for(uint32_t i = 0; i < TRLV_CUSTOM_COLOR_MAX; i++) {
        lv_style_const_prop_t * color_props = custom_props[i][TRLV_CUSTOM_SURFACE_COLOR];
        lv_style_const_prop_t * container_props = custom_props[i][TRLV_CUSTOM_SURFACE_CONTAINER];

        if(color->custom != NULL && i < color->custom_count) {
            const trlv_custom_color_t * custom = &color->custom[i];
            trlv_prop_set_end(put_tone(color_props, custom->color, custom->on_color));
            trlv_prop_set_end(put_tone(container_props, custom->color_container,
                                       custom->on_color_container));
        }
        else {
            trlv_prop_set_end(color_props);
            trlv_prop_set_end(container_props);
        }
    }
#endif
}

/*---------------------------------------------------------------------------
 * Target checks. Each look applies to any object, as LVGL styles do; on an
 * object where it makes no sense these log a warning and change nothing.
 * Compiled out unless LVGL logs warnings.
 *-------------------------------------------------------------------------*/

#if LV_USE_LOG && LV_LOG_LEVEL <= LV_LOG_LEVEL_WARN
#define TRLV_CHECK_TARGETS 1
#else
#define TRLV_CHECK_TARGETS 0
#endif

/* A fill on a button changes its colour at rest only: the theme's or the
 * button type's pressed and focus styles keep their own colours. */
static void warn_if_button(lv_obj_t * obj, const char * func)
{
#if TRLV_CHECK_TARGETS && LV_USE_BUTTON
    if(lv_obj_check_type(obj, &lv_button_class)) {
        LV_LOG_WARN("%s(%p): on an lv_button only the colour at rest changes; pressed and "
                    "focus keep the button's colours. Use trlv_apply_button_type for a button.",
                    func, (void *)obj);
    }
#else
    LV_UNUSED(obj);
    LV_UNUSED(func);
#endif
}

/* A card is a container look: on a widget it mixes with the widget's own
 * parts and states. */
static void warn_if_not_container(lv_obj_t * obj)
{
#if TRLV_CHECK_TARGETS
    if(!lv_obj_check_type(obj, &lv_obj_class)) {
        LV_LOG_WARN("trlv_apply_card(%p): a card is meant for a row, column, box or plain "
                    "lv_obj, not an LVGL widget.", (void *)obj);
    }
#else
    LV_UNUSED(obj);
#endif
}

/* Button states only show on an object that can be pressed or focused. */
static void warn_if_not_clickable(lv_obj_t * obj)
{
#if TRLV_CHECK_TARGETS
    if(!lv_obj_is_clickable(obj)) {
        LV_LOG_WARN("trlv_apply_button_type(%p): the object is not clickable, so its pressed "
                    "and focus states never show. Use an lv_button, or make it clickable "
                    "with lv_obj_set_clickable.", (void *)obj);
    }
#else
    LV_UNUSED(obj);
#endif
}

/* A toggle type's selected look only shows on a checkable object. */
static void warn_if_not_checkable(lv_obj_t * obj)
{
#if TRLV_CHECK_TARGETS
    if(!lv_obj_is_checkable(obj)) {
        LV_LOG_WARN("trlv_apply_button_type(%p): a toggle type on an object that is not "
                    "checkable never shows its selected look. Call "
                    "lv_obj_set_checkable(btn, true) first.", (void *)obj);
    }
#else
    LV_UNUSED(obj);
#endif
}

/*---------------------------------------------------------------------------
 * Appearance
 *-------------------------------------------------------------------------*/

void trlv_apply_surface(lv_obj_t * obj, trlv_surface_t surface)
{
    LV_ASSERT_NULL(obj);

    if((uint32_t)surface >= TRLV_SURFACE_COUNT) {
        LV_LOG_WARN("trlv_apply_surface: %d is not a trlv_surface_t", (int)surface);
        return;
    }
    warn_if_button(obj, "trlv_apply_surface");
    trlv_style_group_apply(obj, &fill_group, surface_styles[surface], LV_PART_MAIN);
}

void trlv_apply_card(lv_obj_t * obj, trlv_card_type_t card)
{
    LV_ASSERT_NULL(obj);

    if((uint32_t)card >= TRLV_CARD_COUNT) {
        LV_LOG_WARN("trlv_apply_card: %d is not a trlv_card_type_t", (int)card);
        return;
    }
    warn_if_not_container(obj);
    trlv_style_group_apply(obj, &fill_group, card_styles[card], LV_PART_MAIN);
}

void trlv_apply_button_type(lv_obj_t * btn, trlv_button_type_t type)
{
    LV_ASSERT_NULL(btn);

    if((uint32_t)type >= TRLV_BUTTON_COUNT) {
        LV_LOG_WARN("trlv_apply_button_type: %d is not a trlv_button_type_t", (int)type);
        return;
    }
    warn_if_not_clickable(btn);
    trlv_style_group_apply(btn, &button_group, button_styles[type], LV_PART_MAIN);
    trlv_style_group_apply(btn, &button_pressed_group, button_pressed_styles[type],
                           LV_PART_MAIN | LV_STATE_PRESSED);
    trlv_style_group_apply(btn, &button_focus_group, button_focus_styles[type],
                           LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    trlv_style_group_apply(btn, &button_disabled_group, button_disabled_styles[type],
                           LV_PART_MAIN | LV_STATE_DISABLED);

    if(type >= BUTTON_TOGGLE_FIRST) {
        uint32_t i = (uint32_t)type - BUTTON_TOGGLE_FIRST;
        warn_if_not_checkable(btn);
        trlv_style_group_apply(btn, &button_checked_group, button_checked_styles[i], BUTTON_CHECKED);
        trlv_style_group_apply(btn, &button_checked_pressed_group, button_checked_pressed_styles[i],
                               BUTTON_CHECKED_PRESSED);
        trlv_style_group_apply(btn, &button_checked_focus_group, button_checked_focus_styles[i],
                               BUTTON_CHECKED_FOCUS);
        trlv_style_group_apply(btn, &button_checked_disabled_group, &button_checked_disabled,
                               BUTTON_CHECKED_DISABLED);
    }
    else {
        trlv_style_group_remove(btn, &button_checked_group, BUTTON_CHECKED);
        trlv_style_group_remove(btn, &button_checked_pressed_group, BUTTON_CHECKED_PRESSED);
        trlv_style_group_remove(btn, &button_checked_focus_group, BUTTON_CHECKED_FOCUS);
        trlv_style_group_remove(btn, &button_checked_disabled_group, BUTTON_CHECKED_DISABLED);
    }
}

void trlv_apply_surface_custom(lv_obj_t * obj, uint32_t index, trlv_custom_surface_t surface)
{
    LV_ASSERT_NULL(obj);

    if((uint32_t)surface >= TRLV_CUSTOM_SURFACE_COUNT) {
        LV_LOG_WARN("trlv_apply_surface_custom: %d is not a trlv_custom_surface_t", (int)surface);
        return;
    }

#if TRLV_CUSTOM_COLOR_MAX > 0
    if(index >= TRLV_CUSTOM_COLOR_MAX) {
        LV_LOG_WARN("trlv_apply_surface_custom: index %d is at or beyond TRLV_CUSTOM_COLOR_MAX (%d)",
                    (int)index, (int)TRLV_CUSTOM_COLOR_MAX);
        return;
    }

    /* Before trlv_init the count is unknown; the style is empty until then. */
    const trlv_color_tokens_t * color = trlv_get_color_tokens();
    if(color != NULL && index >= color->custom_count) {
        LV_LOG_WARN("trlv_apply_surface_custom: index %d is beyond the tokens' custom_count (%d)",
                    (int)index, (int)color->custom_count);
        return;
    }

    warn_if_button(obj, "trlv_apply_surface_custom");
    trlv_style_group_apply(obj, &fill_group,
                           custom_styles[index * TRLV_CUSTOM_SURFACE_COUNT + (uint32_t)surface],
                           LV_PART_MAIN);
#else
    LV_UNUSED(obj);
    LV_LOG_WARN("trlv_apply_surface_custom: index %d is at or beyond TRLV_CUSTOM_COLOR_MAX (0)", (int)index);
    LV_UNUSED(index);
#endif
}
