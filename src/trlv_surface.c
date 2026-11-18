/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_surface.c
 * Surfaces, custom colours, cards and button variants: MD3 container
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

/* Indexed by trlv_surface_t. */
static const lv_style_t * const surface_styles[] = {
    &surface_surface, &surface_container_lowest, &surface_container_low,
    &surface_container, &surface_container_high, &surface_container_highest,
    &surface_primary_container, &surface_secondary_container,
    &surface_tertiary_container, &surface_error_container, &surface_inverse
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

/* Indexed by trlv_card_t. */
static const lv_style_t * const card_styles[] = { &card_filled, &card_elevated, &card_outlined };
TRLV_STATIC_ASSERT(sizeof(card_styles) / sizeof(card_styles[0]) == TRLV_CARD_COUNT,
                   card_styles_cover_the_enum);

/* Button variants: one style per variant at each of four selectors. The
 * theme's filled-button styles stay on the button; LVGL resolves a property
 * by state first, so each variant sets the same properties as the theme at
 * every state it styles, and wins at each because it is added later.
 *
 * Default: bg_color, bg_opa, text_color, border_width, border_color,
 * border_opa. Pressed and focus: bg_color, bg_opa. Disabled: bg_color,
 * bg_opa, text_color, text_opa, border_color, border_opa. Plus end markers. */
#define BUTTON_PROPS 7
#define BUTTON_STATE_PROPS 3
#define BUTTON_DISABLED_PROPS 7

static lv_style_const_prop_t button_props[TRLV_BUTTON_COUNT][BUTTON_PROPS];
static lv_style_const_prop_t button_pressed_props[TRLV_BUTTON_COUNT][BUTTON_STATE_PROPS];
static lv_style_const_prop_t button_focus_props[TRLV_BUTTON_COUNT][BUTTON_STATE_PROPS];
static lv_style_const_prop_t button_disabled_props[TRLV_BUTTON_COUNT][BUTTON_DISABLED_PROPS];

#define BUTTON_STYLES(name_, variant_)                                                             \
    static LV_STYLE_CONST_INIT(button_##name_, button_props[variant_]);                           \
    static LV_STYLE_CONST_INIT(button_##name_##_pressed, button_pressed_props[variant_]);         \
    static LV_STYLE_CONST_INIT(button_##name_##_focus, button_focus_props[variant_]);             \
    static LV_STYLE_CONST_INIT(button_##name_##_disabled, button_disabled_props[variant_])

BUTTON_STYLES(filled, TRLV_BUTTON_FILLED);
BUTTON_STYLES(tonal, TRLV_BUTTON_TONAL);
BUTTON_STYLES(outlined, TRLV_BUTTON_OUTLINED);
BUTTON_STYLES(text, TRLV_BUTTON_TEXT);

/* Indexed by trlv_button_t, one list per selector. */
static const lv_style_t * const button_styles[] = {
    &button_filled, &button_tonal, &button_outlined, &button_text
};
static const lv_style_t * const button_pressed_styles[] = {
    &button_filled_pressed, &button_tonal_pressed, &button_outlined_pressed, &button_text_pressed
};
static const lv_style_t * const button_focus_styles[] = {
    &button_filled_focus, &button_tonal_focus, &button_outlined_focus, &button_text_focus
};
static const lv_style_t * const button_disabled_styles[] = {
    &button_filled_disabled, &button_tonal_disabled, &button_outlined_disabled, &button_text_disabled
};
TRLV_STATIC_ASSERT(sizeof(button_styles) / sizeof(button_styles[0]) == TRLV_BUTTON_COUNT,
                   button_styles_cover_the_enum);

static const trlv_style_group_t button_group = TRLV_STYLE_GROUP_LIST(button_styles);
static const trlv_style_group_t button_pressed_group = TRLV_STYLE_GROUP_LIST(button_pressed_styles);
static const trlv_style_group_t button_focus_group = TRLV_STYLE_GROUP_LIST(button_focus_styles);
static const trlv_style_group_t button_disabled_group = TRLV_STYLE_GROUP_LIST(button_disabled_styles);

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
 * trlv_apply_surface, trlv_apply_surface_custom and trlv_apply_card replace each other. */
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
static void build_card(trlv_card_t card, lv_color_t container, int32_t border_width,
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

/* One button variant at all four selectors. `opaque` is false for the
 * transparent variants; `border_width` is 0 except for outlined. */
static void build_button(trlv_button_t variant, bool opaque, lv_color_t container,
                         lv_color_t content, int32_t border_width, const trlv_color_tokens_t * color)
{
    const trlv_state_tokens_t * state = &color->state;
    lv_style_const_prop_t * entry;

    entry = button_props[variant];
    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, container);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, opaque ? LV_OPA_COVER : LV_OPA_TRANSP);
    trlv_prop_set_color(entry++, LV_STYLE_TEXT_COLOR, content);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_WIDTH, border_width);
    trlv_prop_set_color(entry++, LV_STYLE_BORDER_COLOR, color->outline);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_OPA, LV_OPA_COVER);
    trlv_prop_set_end(entry);

    put_state_layer(button_pressed_props[variant], opaque, container, content, state->pressed);
    put_state_layer(button_focus_props[variant], opaque, container, content, state->focus);

    /* MD3 disabled: content on_surface at 38 %; an opaque container, or
     * the outlined border, on_surface at 12 %. */
    entry = button_disabled_props[variant];
    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, opaque ? state->disabled_container : LV_OPA_TRANSP);
    trlv_prop_set_color(entry++, LV_STYLE_TEXT_COLOR, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_TEXT_OPA, state->disabled_content);
    trlv_prop_set_color(entry++, LV_STYLE_BORDER_COLOR, color->on_surface);
    trlv_prop_set_num(entry++, LV_STYLE_BORDER_OPA, state->disabled_container);
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

    build_button(TRLV_BUTTON_FILLED, true, color->primary, color->on_primary, 0, color);
    build_button(TRLV_BUTTON_TONAL, true, color->secondary_container, color->on_secondary_container, 0, color);
    build_button(TRLV_BUTTON_OUTLINED, false, color->primary, color->primary, layout->border_width, color);
    build_button(TRLV_BUTTON_TEXT, false, color->primary, color->primary, 0, color);

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
 * Modifiers
 *-------------------------------------------------------------------------*/

void trlv_apply_surface(lv_obj_t * obj, trlv_surface_t surface)
{
    LV_ASSERT_NULL(obj);

    if((uint32_t)surface >= TRLV_SURFACE_COUNT) {
        LV_LOG_WARN("trlv_apply_surface: %d is not a trlv_surface_t", (int)surface);
        return;
    }
    trlv_style_group_apply(obj, &fill_group, surface_styles[surface], LV_PART_MAIN);
}

void trlv_apply_card(lv_obj_t * obj, trlv_card_t card)
{
    LV_ASSERT_NULL(obj);

    if((uint32_t)card >= TRLV_CARD_COUNT) {
        LV_LOG_WARN("trlv_apply_card: %d is not a trlv_card_t", (int)card);
        return;
    }
    trlv_style_group_apply(obj, &fill_group, card_styles[card], LV_PART_MAIN);
}

void trlv_apply_button_type(lv_obj_t * btn, trlv_button_t variant)
{
    LV_ASSERT_NULL(btn);

    if((uint32_t)variant >= TRLV_BUTTON_COUNT) {
        LV_LOG_WARN("trlv_apply_button_type: %d is not a trlv_button_t", (int)variant);
        return;
    }
    trlv_style_group_apply(btn, &button_group, button_styles[variant], LV_PART_MAIN);
    trlv_style_group_apply(btn, &button_pressed_group, button_pressed_styles[variant],
                           LV_PART_MAIN | LV_STATE_PRESSED);
    trlv_style_group_apply(btn, &button_focus_group, button_focus_styles[variant],
                           LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    trlv_style_group_apply(btn, &button_disabled_group, button_disabled_styles[variant],
                           LV_PART_MAIN | LV_STATE_DISABLED);
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

    trlv_style_group_apply(obj, &fill_group,
                           custom_styles[index * TRLV_CUSTOM_SURFACE_COUNT + (uint32_t)surface],
                           LV_PART_MAIN);
#else
    LV_UNUSED(obj);
    LV_LOG_WARN("trlv_apply_surface_custom: index %d is at or beyond TRLV_CUSTOM_COLOR_MAX (0)", (int)index);
    LV_UNUSED(index);
#endif
}
