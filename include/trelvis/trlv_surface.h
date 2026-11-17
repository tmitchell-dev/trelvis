/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_surface.h
 * MD3 surfaces, cards and button variants: modifiers that give any object
 * an MD3 container colour, with its matching `on_` content colour inherited
 * by everything inside.
 *
 * Mapping: Figma / MD3 → CSS → TreLVis → LVGL mechanism
 *
 * | Figma / MD3                         | CSS                          | TreLVis            | LVGL mechanism                                          |
 * |-------------------------------------|------------------------------|--------------------|---------------------------------------------------------|
 * | Fill with a surface / container role | `background` + `color`      | trlv_surface       | shared style: `bg_color`, `bg_opa`, inherited `text_color` |
 * | Fill with a custom colour or its container | `background` + `color` | trlv_surface_custom | shared styles per custom colour: `bg_color`, `bg_opa`, inherited `text_color` |
 * | MD3 card (filled, elevated, outlined) | `background`, `border-radius`, `border` / `box-shadow` | trlv_card | shared style: `bg_*`, `radius`, `border_*` / `shadow_*`, inherited `text_color` |
 * | MD3 button type (filled, tonal, outlined, text) | button classes with `:active`, `:focus-visible`, `:disabled` | trlv_button_variant | shared styles with LV_STATE_PRESSED / FOCUS_KEY / DISABLED selectors |
 *
 * All are shared token styles, one per value, so they cost no RAM per
 * object and recolour on trlv_theme_set_color_mode. trlv_surface and
 * trlv_card form one style group and replace each other; button variants
 * are a group of their own. Calling a modifier again replaces its previous
 * value.
 *
 * All functions must be called from the LVGL thread.
 */

#ifndef TRLV_SURFACE_H
#define TRLV_SURFACE_H

#include "trlv_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * Types
 *-------------------------------------------------------------------------*/

/**
 * An MD3 surface or container colour role, with the content role that goes
 * on it. Named after the container role.
 */
typedef enum {
    TRLV_SURFACE_SURFACE,             /**< surface / on_surface: the screen background */
    TRLV_SURFACE_CONTAINER_LOWEST,    /**< surface_container_lowest / on_surface */
    TRLV_SURFACE_CONTAINER_LOW,       /**< surface_container_low / on_surface */
    TRLV_SURFACE_CONTAINER,           /**< surface_container / on_surface */
    TRLV_SURFACE_CONTAINER_HIGH,      /**< surface_container_high / on_surface */
    TRLV_SURFACE_CONTAINER_HIGHEST,   /**< surface_container_highest / on_surface */
    TRLV_SURFACE_PRIMARY_CONTAINER,   /**< primary_container / on_primary_container */
    TRLV_SURFACE_SECONDARY_CONTAINER, /**< secondary_container / on_secondary_container */
    TRLV_SURFACE_TERTIARY_CONTAINER,  /**< tertiary_container / on_tertiary_container */
    TRLV_SURFACE_ERROR_CONTAINER,     /**< error_container / on_error_container */
    TRLV_SURFACE_INVERSE,            /**< inverse_surface / inverse_on_surface: snackbars, tooltips */
    TRLV_SURFACE_COUNT                /**< Number of surfaces; not a surface */
} trlv_surface_t;

/**
 * Which pair of a custom colour (trlv_custom_color_t) fills a surface.
 */
typedef enum {
    TRLV_CUSTOM_SURFACE_COLOR,      /**< color / on_color: a strong fill, such as an
                                         alarm banner */
    TRLV_CUSTOM_SURFACE_CONTAINER,  /**< color_container / on_color_container: lower
                                         emphasis, as the MD3 container roles */
    TRLV_CUSTOM_SURFACE_COUNT       /**< Number of pairs; not a pair */
} trlv_custom_surface_t;

/** The three MD3 card types. */
typedef enum {
    TRLV_CARD_FILLED,     /**< surface_container_highest, no border or shadow */
    TRLV_CARD_ELEVATED,   /**< surface_container_low with the level 1 shadow */
    TRLV_CARD_OUTLINED,   /**< surface with a border_width outline_variant border */
    TRLV_CARD_COUNT       /**< Number of card types; not a card type */
} trlv_card_t;

/** The MD3 common button types (v1 has no elevated button). */
typedef enum {
    TRLV_BUTTON_FILLED,   /**< primary / on_primary: the theme's default */
    TRLV_BUTTON_TONAL,    /**< secondary_container / on_secondary_container */
    TRLV_BUTTON_OUTLINED, /**< transparent, border_width outline border, primary content */
    TRLV_BUTTON_TEXT,     /**< transparent, no border, primary content */
    TRLV_BUTTON_COUNT     /**< Number of button types; not a button type */
} trlv_button_t;

/*---------------------------------------------------------------------------
 * Modifiers
 *-------------------------------------------------------------------------*/

/**
 * Fill an object with an MD3 surface or container colour, and set the
 * matching `on_` colour for its content. Figma: a fill bound to a colour
 * variable such as *Surface Container High*; CSS `background` plus `color`.
 *
 * Sets `bg_color` (opaque) and `text_color`. LVGL text colour inherits like
 * CSS `color`, so every label inside, at any depth, gets the right MD3
 * pairing without being styled. A label with its own text colour (a local
 * style, or a project style) keeps it.
 *
 * Tone only: no radius, border, shadow or padding. Add a radius with
 * lv_obj_set_style_radius(obj, trlv_layout()->radius[TRLV_RADIUS_MD], 0),
 * or use trlv_card.
 *
 * trlv_surface and trlv_card form one group, so this replaces an earlier
 * trlv_card or trlv_surface on the object. Works on any object; meant for
 * boxes, rows and columns. LVGL widgets have their colours from the theme.
 *
 * @param obj      any object
 * @param surface  the surface or container role; any other value logs a
 *                 warning and changes nothing
 */
void trlv_surface(lv_obj_t * obj, trlv_surface_t surface);

/**
 * Fill an object with one of the project's custom colours, and set the
 * matching content colour. Figma: a fill bound to a Material Theme Builder
 * custom colour variable; CSS `background` plus `color`.
 *
 * Works as trlv_surface does, with a pair from
 * `trlv_color()->custom[index]`: tone only, the content colour inherited,
 * and recoloured on trlv_theme_set_color_mode. `surface` chooses the pair:
 * the colour itself with `on_color` (a strong fill: an alarm banner, a
 * status badge) or its container with `on_color_container`. It is in the
 * same group as trlv_surface and trlv_card, so they replace each other.
 *
 * TreLVis only colours the object. What a custom colour means, and alarm
 * behaviour such as flashing or acknowledgement, belong to the project.
 *
 * Each index has one shared style per pair, and TRLV_CUSTOM_COLOR_MAX sets
 * how many indexes have them. An index at or beyond that, or beyond the
 * tokens' `custom_count`, logs a warning and leaves the object unchanged.
 *
 * @param obj      any object
 * @param index    index into trlv_color_tokens_t::custom, normally a value
 *                 of the project's own enum
 * @param surface  which pair of the custom colour to use; any other value
 *                 logs a warning and changes nothing
 */
void trlv_surface_custom(lv_obj_t * obj, uint32_t index, trlv_custom_surface_t surface);

/**
 * Style an object as an MD3 card. Figma: an MD3 card component; CSS a
 * `background`, `border-radius` and `border` or `box-shadow`.
 *
 * Sets the card's container colour and inherited `on_surface` content
 * colour (as trlv_surface does), the TRLV_RADIUS_MD corner radius, and:
 *
 * - TRLV_CARD_FILLED: nothing more;
 * - TRLV_CARD_ELEVATED: the TRLV_ELEVATION_1 shadow in the `shadow` colour;
 * - TRLV_CARD_OUTLINED: a `border_width` border in `outline_variant`. As in
 *   Figma with an inside stroke, the border is drawn inside the padding box.
 *
 * Padding stays explicit, as in Figma: use trlv_pad, typically
 * TRLV_SPACE_LG. Children are not clipped to the rounded corners; if a
 * child's background reaches a corner, add
 * lv_obj_set_style_clip_corner(card, true, 0) (it costs draw time).
 *
 * MD3 cards use surface tones only. trlv_surface and trlv_card form one
 * group, so a later trlv_surface replaces the card. A card has no pressed
 * or focus state: clickable MD3 cards, with state layers, are not part of
 * v1.
 *
 * @param obj   any object, usually a column
 * @param card  the card type
 *
 * @note Not implemented yet (milestone 5).
 */
void trlv_card(lv_obj_t * obj, trlv_card_t card);

/**
 * Restyle an lv_button as another MD3 button type. Figma: the MD3 button
 * component's "Style" property; CSS a button class.
 *
 * With the TreLVis theme, every lv_button is already an MD3 filled button:
 * radius FULL, height control_height[TRLV_SIZE_MD], Label Large text, and
 * pressed, focus and disabled states. This changes only what differs
 * between the types: the container colour, the border and the content
 * colour (inherited by the button's label and icons), each with its state
 * styles:
 *
 * - pressed: the MD3 state layer, the content colour over the container
 *   at `state.pressed` opacity, pre-mixed at init and on each colour-mode
 *   switch;
 * - focused by keypad or encoder: the same with `state.focus`, plus the
 *   theme's focus ring;
 * - disabled: content in on_surface at `state.disabled_content`, container
 *   (or the outlined border) in on_surface at `state.disabled_container`.
 *
 * TRLV_BUTTON_FILLED returns the button to the theme's default. Shape,
 * height and text role stay the theme's; change the height with
 * lv_obj_set_height(btn, trlv_layout()->control_height[TRLV_SIZE_LG]).
 * Calling it again replaces the previous variant.
 *
 * With TRLV_USE_THEME 0 the variant still sets its colours and states, but
 * the shape, height and text role come from whatever theme is installed.
 *
 * @param btn      an lv_button (it works on any object, but the states are
 *                 only meaningful on a clickable one)
 * @param variant  the button type
 *
 * @note Not implemented yet (milestone 5).
 */
void trlv_button_variant(lv_obj_t * btn, trlv_button_t variant);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_SURFACE_H */
