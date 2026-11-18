/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trelvis.h
 * TreLVis: a light and versatile layout layer for LVGL.
 *
 * Include this one header to use the whole library:
 *
 *     #include <trelvis/trelvis.h>
 *
 * All public functions must be called from the LVGL thread (the thread that
 * calls lv_timer_handler()), exactly like LVGL's own API.
 */

/**
 * @mainpage TreLVis API reference
 *
 * TreLVis is a small utility layer on LVGL 9.6: Figma and CSS style layout,
 * Material Design 3 tokens and an MD3 theme for LVGL's own widgets.
 * Everything takes or returns a plain lv_obj_t *, and raw LVGL calls stay
 * valid on every object.
 *
 * @section kinds Five kinds of function
 *
 * The verb in a name says which kind a call is, as in LVGL:
 *
 * | Kind                | Name                                    | What it does                                                                   |
 * |---------------------|-----------------------------------------|--------------------------------------------------------------------------------|
 * | @ref trlv_setup     | trlv_init, trlv_theme_set_color_mode    | Install the MD3 theme with the project's tokens; switch the colour mode.        |
 * | @ref trlv_create    | `trlv_<thing>_create(parent, ...)`      | Make a primitive and return it. Containers start with no gap or padding, fit content. |
 * | @ref trlv_set       | `trlv_set_<property>(obj, ...)`         | One row of Figma's auto layout panel. Never changes colour.                    |
 * | @ref trlv_apply     | `trlv_apply_<look>(obj, ...)`           | Figma's "apply a style": colour, shape, type. Never changes layout.            |
 * | @ref trlv_get       | `trlv_get_<what>_tokens()`              | The current tokens, for raw LVGL calls that need a token value.                |
 *
 * Setters and applies (the modifiers) work on any object, including LVGL
 * widgets and objects TreLVis didn't create, and each replaces its own
 * previous style rather than adding another, which is what `set` means in
 * LVGL. They use shared styles, so they cost no RAM per object.
 *
 * @section screen How a screen reads
 *
 * One block per Figma frame: the constructor, one setter per auto layout
 * property, then the looks it applies. An LVGL widget, such as the button
 * here, is created with LVGL's own call and styled by the theme.
 *
 *     lv_obj_t * card = trlv_column_create(parent);
 *     trlv_set_gap(card, TRLV_SPACE_SM);
 *     trlv_set_pad(card, TRLV_SPACE_LG);
 *     trlv_set_width_fill(card);
 *     trlv_apply_card(card, TRLV_CARD_FILLED);
 *
 *     lv_obj_t * btn = lv_button_create(card);
 *     trlv_apply_button_type(btn, TRLV_BUTTON_TONAL);
 *     trlv_text_create(btn, "Calibrate", TRLV_TEXT_LABEL_LARGE);
 *
 * The guide to each kind, with the reasons behind it, is "The API by
 * kind" in docs/concepts.md.
 */

/**
 * @defgroup trlv_setup Setup
 * Once-per-program calls that act on the whole library: trlv_init installs
 * the theme with the project's tokens, and trlv_theme_set_color_mode
 * switches light, dark or any other colour token set at run time.
 */

/**
 * @defgroup trlv_create Constructors
 * `trlv_<thing>_create(parent, ...)`: create a primitive and return a plain
 * lv_obj_t *. They take the parent plus only arguments whose type makes
 * them clear at the call site, never spacing. Box, row and column start
 * like a fresh Figma frame: no gap, no padding, fit content. LVGL widgets
 * need no TreLVis constructor: the theme styles them.
 */

/**
 * @defgroup trlv_set Layout setters
 * `trlv_set_<property>(obj, ...)`: the properties of Figma's auto layout
 * panel, one call each. They work on any object, replace the previous value
 * of their group, use shared styles, and never change colour.
 */

/**
 * @defgroup trlv_apply Appearance
 * `trlv_apply_<look>(obj, ...)`: Figma's "apply a style", for colour, shape
 * and type. They work on any object, replace the previous look of their
 * group, recolour on a colour-mode switch, and never change layout. On an
 * object a look makes no sense on, they log a warning and apply it anyway.
 */

/**
 * @defgroup trlv_get Getters
 * `trlv_get_<what>_tokens()`: the tokens in use, for raw LVGL calls and
 * widget APIs that need a token value. A value passed to LVGL this way is a
 * snapshot: it doesn't follow a later colour-mode switch.
 */

#ifndef TRELVIS_H
#define TRELVIS_H

#include "trlv_conf_default.h"
#include "trlv_version.h"
#include "trlv_tokens.h"
#include "trlv_theme.h"
#include "trlv_styles.h"
#include "trlv_layout.h"
#include "trlv_surface.h"
#include "trlv_text.h"

#endif /* TRELVIS_H */
