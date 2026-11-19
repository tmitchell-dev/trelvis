/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_surface.h
 * MD3 surfaces, cards and button types: appearance functions that give an
 * object an MD3 container colour, with its matching `on_` content colour
 * inherited by everything inside.
 *
 * Appearance functions are named `trlv_apply_*`, as in Figma, where you
 * apply a colour or text style; layout properties are `trlv_set_*`, as
 * in Figma's auto layout panel. An apply never changes layout and a set
 * never changes colour, so a card's padding is a separate trlv_set_pad:
 *
 *     lv_obj_t * card = trlv_column_create(parent);
 *     trlv_set_pad(card, TRLV_SPACE_LG);
 *     trlv_apply_card(card, TRLV_CARD_FILLED);
 *
 * Each function works on any object, as LVGL styles do, and logs a
 * warning (LV_LOG_WARN, which changes nothing) for an object it makes no
 * sense on; see each function.
 *
 * Mapping: Figma / MD3 → CSS → TreLVis → LVGL mechanism
 *
 * | Figma / MD3                                     | CSS                                                          | TreLVis                   | LVGL mechanism                                                                  |
 * |-------------------------------------------------|--------------------------------------------------------------|---------------------------|---------------------------------------------------------------------------------|
 * | Fill with a surface / container role            | `background` + `color`                                       | trlv_apply_surface        | shared style: `bg_color`, `bg_opa`, inherited `text_color`                      |
 * | Fill with a custom colour or its container      | `background` + `color`                                       | trlv_apply_surface_custom | shared styles per custom colour: `bg_color`, `bg_opa`, inherited `text_color`   |
 * | MD3 card (filled, elevated, outlined)           | `background`, `border-radius`, `border` / `box-shadow`       | trlv_apply_card           | shared style: `bg_*`, `radius`, `border_*` / `shadow_*`, inherited `text_color` |
 * | MD3 button type (filled, tonal, outlined, text) | button classes with `:active`, `:focus-visible`, `:disabled` | trlv_apply_button_type    | shared styles with LV_STATE_PRESSED / FOCUS_KEY / DISABLED selectors            |
 * | MD3 toggle button (filled, tonal, outlined)     | the same, with `:checked`                                    | trlv_apply_button_type    | the same, plus LV_STATE_CHECKED combined with each                              |
 *
 * All are shared token styles, one per value, so they cost no RAM per
 * object and recolour on trlv_theme_set_color_mode. trlv_apply_surface,
 * trlv_apply_surface_custom and trlv_apply_card form one style group and
 * replace each other; button types are a group of their own. Applying
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
 * A surface or container colour role (MD3's names), with the content role
 * that goes on it. Named after the container role. Nesting goes one step
 * up the container ladder per level; docs/designing-a-screen.md says which
 * role to use for what.
 */
typedef enum {
    TRLV_SURFACE_SURFACE,             /**< surface / on_surface: the page or screen background */
    TRLV_SURFACE_CONTAINER_LOWEST,    /**< surface_container_lowest / on_surface: a panel
                                           lighter than the page */
    TRLV_SURFACE_CONTAINER_LOW,       /**< surface_container_low / on_surface: a panel on the page */
    TRLV_SURFACE_CONTAINER,           /**< surface_container / on_surface: a menu or popup, a
                                           nested panel */
    TRLV_SURFACE_CONTAINER_HIGH,      /**< surface_container_high / on_surface: a header or
                                           footer bar, a dialog */
    TRLV_SURFACE_CONTAINER_HIGHEST,   /**< surface_container_highest / on_surface: the highest
                                           step; a recessed track or well */
    TRLV_SURFACE_PRIMARY_CONTAINER,   /**< primary_container / on_primary_container: an
                                           emphasised area */
    TRLV_SURFACE_SECONDARY_CONTAINER, /**< secondary_container / on_secondary_container: a
                                           selected item, a chip */
    TRLV_SURFACE_TERTIARY_CONTAINER,  /**< tertiary_container / on_tertiary_container: a
                                           contrasting accent */
    TRLV_SURFACE_ERROR_CONTAINER,     /**< error_container / on_error_container: an input or
                                           action error (not a process alarm) */
    TRLV_SURFACE_INVERSE,            /**< inverse_surface / inverse_on_surface: snackbars, tooltips */
    TRLV_SURFACE_DIM,                 /**< surface_dim / on_surface: the dimmest surface. In MD3's
                                           dark schemes it equals `surface`, so it is not darker
                                           than the page in both modes */
    TRLV_SURFACE_BRIGHT,              /**< surface_bright / on_surface: the brightest surface. In
                                           MD3's light schemes it equals `surface` */
    TRLV_SURFACE_COUNT                /**< Number of surfaces; not a surface */
} trlv_surface_t;

/**
 * Which pair of a custom colour (trlv_custom_color_t) fills a surface.
 */
typedef enum {
    TRLV_CUSTOM_SURFACE_COLOR,      /**< color / on_color: the strong fill, such as an
                                         active alarm banner */
    TRLV_CUSTOM_SURFACE_CONTAINER,  /**< color_container / on_color_container: the calm
                                         fill, such as an acknowledged alarm */
    TRLV_CUSTOM_SURFACE_COUNT       /**< Number of pairs; not a pair */
} trlv_custom_surface_t;

/** The three MD3 card types. */
typedef enum {
    TRLV_CARD_FILLED,     /**< surface_container_highest, no border or shadow */
    TRLV_CARD_ELEVATED,   /**< surface_container_low with the level 1 shadow */
    TRLV_CARD_OUTLINED,   /**< surface with a border_width outline_variant border */
    TRLV_CARD_COUNT       /**< Number of card types; not a card type */
} trlv_card_type_t;

/**
 * The MD3 common button types, and the toggle buttons of MD3's button
 * spec (v1 has no elevated button).
 *
 * A toggle type is for a checkable button (lv_obj_set_checkable): it
 * has an unselected look and a selected look (LV_STATE_CHECKED), each
 * listed here as unselected → selected. MD3 has no text toggle button.
 */
typedef enum {
    TRLV_BUTTON_FILLED,          /**< primary / on_primary: the theme's default */
    TRLV_BUTTON_TONAL,           /**< secondary_container / on_secondary_container */
    TRLV_BUTTON_OUTLINED,        /**< transparent, border_width outline border, primary content */
    TRLV_BUTTON_TEXT,            /**< transparent, no border, primary content */
    TRLV_BUTTON_FILLED_TOGGLE,   /**< surface_container / on_surface_variant →
                                      primary / on_primary */
    TRLV_BUTTON_TONAL_TOGGLE,    /**< secondary_container / on_secondary_container →
                                      secondary / on_secondary */
    TRLV_BUTTON_OUTLINED_TOGGLE, /**< transparent, border_width outline_variant border,
                                      on_surface_variant content →
                                      inverse_surface / inverse_on_surface, no border */
    TRLV_BUTTON_COUNT            /**< Number of button types; not a button type */
} trlv_button_type_t;

/*---------------------------------------------------------------------------
 * Appearance. Each applies a shared look to any object, replaces the look
 * its group had, and never changes layout (size, padding, gap).
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
 *
 *     lv_obj_set_style_radius(obj, trlv_get_radius(TRLV_RADIUS_MD), 0);
 *
 * or use trlv_apply_card.
 *
 * trlv_apply_surface, trlv_apply_surface_custom and trlv_apply_card form
 * one group, so this replaces whichever of them the object had.
 *
 * Meant for containers (boxes, rows, columns) and labels, such as a status
 * badge. LVGL widgets have their colours from the theme. On an lv_button
 * it changes only the colour at rest: pressed and focus keep the button
 * type's colours, so it logs an LV_LOG_WARN, then applies the surface
 * anyway. Use trlv_apply_button_type for a button.
 *
 * @param obj      any object
 * @param surface  the surface or container role; any other value logs a
 *                 warning and changes nothing
 *
 * @ingroup trlv_apply
 */
void trlv_apply_surface(lv_obj_t * obj, trlv_surface_t surface);

/**
 * Fill an object with one of the project's custom colours, and set the
 * matching content colour. Figma: a fill bound to a Material Theme Builder
 * custom colour variable; CSS `background` plus `color`.
 *
 * Works as trlv_apply_surface does, with a pair from
 * `trlv_get_color_tokens()->custom[index]`: tone only, the content colour
 * inherited, and recoloured on trlv_theme_set_color_mode. `surface` chooses the
 * pair: the colour itself with `on_color` (a strong fill: an alarm banner, a
 * status badge) or its container with `on_color_container`. It is in the same
 * group as trlv_apply_surface and trlv_apply_card, so they replace each other.
 * Like trlv_apply_surface, it logs a warning on an lv_button, where pressed
 * and focus keep the button type's colours: a custom-coloured button (a red
 * Stop button) is not part of v1.
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
 *
 * @ingroup trlv_apply
 */
void trlv_apply_surface_custom(lv_obj_t * obj, uint32_t index, trlv_custom_surface_t surface);

/**
 * Style an object as an MD3 card. Figma: an MD3 card component; CSS a
 * `background`, `border-radius` and `border` or `box-shadow`.
 *
 * Sets the card's container colour and inherited `on_surface` content
 * colour (as trlv_apply_surface does), the TRLV_RADIUS_MD corner radius, and:
 *
 * - TRLV_CARD_FILLED: nothing more;
 * - TRLV_CARD_ELEVATED: the TRLV_ELEVATION_1 shadow in the `shadow` colour;
 * - TRLV_CARD_OUTLINED: a `border_width` border in `outline_variant`. As in
 *   Figma with an inside stroke included in layout, the border sits inside
 *   the card's size and insets the content by its width, on top of the
 *   padding.
 *
 * Padding is layout, so it is set separately with trlv_set_pad: MD3 cards
 * use TRLV_SPACE_LG. A card's header, media and actions are children,
 * all optional as in MD3; a card with fixed content is a project
 * component (`stat_card_create(parent, …)`).
 *
 * Children are not clipped to the rounded corners; if a child's background
 * reaches a corner, add lv_obj_set_style_clip_corner(card, true, 0) (it
 * costs draw time).
 *
 * MD3 cards use surface tones only. trlv_apply_surface and
 * trlv_apply_card form one group, so a later trlv_apply_surface replaces
 * the card. A card has no pressed or focus state: clickable MD3 cards,
 * with state layers, are not part of v1.
 *
 * Every card type sets the border and shadow properties (to none where the
 * type has none), so on a plain lv_obj it also replaces the theme's border.
 *
 * Meant for a plain container: a row, column, box or lv_obj. On an LVGL
 * widget (a button, a label, a slider) the card's look is mixed with the
 * widget's own parts and states, so it logs an LV_LOG_WARN, then applies
 * the card anyway.
 *
 * @param obj   a row, column, box or plain lv_obj
 * @param card  the card type; any other value logs a warning and changes
 *              nothing
 *
 * @ingroup trlv_apply
 */
void trlv_apply_card(lv_obj_t * obj, trlv_card_type_t card);

/**
 * Restyle an lv_button as another MD3 button type: filled, tonal,
 * outlined or text, or a filled, tonal or outlined toggle button. Figma:
 * the MD3 button component's "Style" property (and its selected variant);
 * CSS a button class (with `:checked`).
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
 *   switch. Outlined and text buttons have a transparent container, so
 *   their layer is the content colour at that opacity, drawn over whatever
 *   is behind the button;
 * - focused by keypad or encoder: the same with `state.focus`, plus the
 *   theme's focus ring;
 * - disabled: content in on_surface at `state.disabled_content`, container
 *   (or the outlined border) in on_surface at `state.disabled_container`.
 *
 * TRLV_BUTTON_FILLED returns the button to the theme's default. Shape, height
 * and text role stay the theme's; change the height with lv_obj_set_height(btn,
 * trlv_get_control_height(TRLV_SIZE_LG)). Calling it again
 * replaces the previous type.
 *
 * With TRLV_USE_THEME 0 the type still sets its colours and states, but
 * the shape, height and text role come from whatever theme is installed,
 * and there is no focus ring.
 *
 * The type adds four shared styles (default, pressed, focus and
 * disabled), each one group, so a second call replaces all four in place.
 *
 * **Toggle buttons.** A toggle type (TRLV_BUTTON_FILLED_TOGGLE,
 * TRLV_BUTTON_TONAL_TOGGLE, TRLV_BUTTON_OUTLINED_TOGGLE) is for a
 * checkable button, an MD3 toggle button: the type's unselected look at
 * rest, and its selected look when LV_STATE_CHECKED is set, each with its
 * own pressed and focus state layer. Disabled is the same as for the other
 * types, except that a selected outlined toggle keeps an `on_surface`
 * container at `state.disabled_container`. Make the button checkable
 * first; a toggle type on an object that isn't checkable logs an
 * LV_LOG_WARN, then applies anyway:
 *
 *     lv_obj_t * btn = lv_button_create(row);
 *     lv_obj_set_checkable(btn, true);
 *     trlv_apply_button_type(btn, TRLV_BUTTON_TONAL_TOGGLE);
 *     trlv_text_create(btn, "Auto", TRLV_TEXT_LABEL_LARGE);
 *
 * A toggle type adds four more shared styles (selected, and selected while
 * pressed, focused or disabled), again one group each; any other type
 * removes them. The other types have no selected look: with the TreLVis
 * theme, a checkable button of another type looks the same checked or not
 * (the theme removes the parent theme's checked colour from lv_button).
 *
 * It works on any clickable object, for example a clickable row used as a
 * list item: the colours and states apply, but the shape, height and
 * text role are the theme's for lv_button only. On an object that is not
 * clickable the states never show, so it logs an LV_LOG_WARN, then
 * applies the type anyway.
 *
 * @param btn   an lv_button, or another clickable object
 * @param type  the button type; any other value logs a warning and changes
 *              nothing
 *
 * @ingroup trlv_apply
 */
void trlv_apply_button_type(lv_obj_t * btn, trlv_button_type_t type);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_SURFACE_H */
