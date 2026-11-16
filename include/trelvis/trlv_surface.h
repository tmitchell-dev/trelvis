/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_surface.h
 * MD3 surfaces and cards: modifiers that give any object an MD3 container
 * colour, with its matching `on_` content colour inherited by everything
 * inside.
 *
 * Mapping: Figma / MD3 → CSS → TreLVis → LVGL mechanism
 *
 * | Figma / MD3                         | CSS                          | TreLVis            | LVGL mechanism                                          |
 * |-------------------------------------|------------------------------|--------------------|---------------------------------------------------------|
 * | Fill with a surface / container role | `background` + `color`      | trlv_surface       | shared style: `bg_color`, `bg_opa`, inherited `text_color` |
 * | MD3 card (filled, elevated, outlined) | `background`, `border-radius`, `border` / `box-shadow` | trlv_card | shared style: `bg_*`, `radius`, `border_*` / `shadow_*`, inherited `text_color` |
 *
 * Both are shared token styles, one per value, so they cost no RAM per
 * object and recolour on trlv_theme_set_color_mode. They form one style
 * group: trlv_surface and trlv_card replace each other, and calling either
 * again replaces the previous value.
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
    TRLV_SURFACE_SUCCESS_CONTAINER,   /**< success_container / on_success_container (extension) */
    TRLV_SURFACE_WARNING_CONTAINER,   /**< warning_container / on_warning_container (extension) */
    TRLV_SURFACE_INVERSE,             /**< inverse_surface / inverse_on_surface: snackbars, tooltips */
    TRLV_SURFACE_COUNT                /**< Number of surfaces; not a surface */
} trlv_surface_t;

/** The three MD3 card types. */
typedef enum {
    TRLV_CARD_FILLED,     /**< surface_container_highest, no border or shadow */
    TRLV_CARD_ELEVATED,   /**< surface_container_low with the level 1 shadow */
    TRLV_CARD_OUTLINED,   /**< surface with a border_width outline_variant border */
    TRLV_CARD_COUNT       /**< Number of card types; not a card type */
} trlv_card_t;

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
 * @param surface  the surface or container role
 *
 * @note Not implemented yet (milestone 5).
 */
void trlv_surface(lv_obj_t * obj, trlv_surface_t surface);

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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_SURFACE_H */
