/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo.h
 * TreLVis demo. Not part of the library: an example a host project builds
 * next to it and calls from its own simulator or target.
 *
 * The demo is a set of sections, one per area of the library, with a row of
 * buttons to move between them and a switch between the light and dark
 * colour modes. Each section is also a function of its own, so a host can
 * show one section on its own screen.
 *
 * Call trlv_init first. The demo is written for the token template, but any
 * complete set of tokens works:
 *
 *     trlv_init(disp, &tokens_light, &tokens_regular);
 *     trlv_demo_create(lv_display_get_screen_active(disp), &tokens_light, &tokens_dark);
 *
 * Keypad and encoder: the demo's buttons, switches, checkboxes, sliders,
 * arcs and fields join LVGL's default group as they are created, as any
 * LVGL widget does. To move through them with a keypad or encoder, create
 * a group, make it the default and give it to the input device before
 * calling trlv_demo_create:
 *
 *     lv_group_t * group = lv_group_create();
 *     lv_group_set_default(group);
 *     lv_indev_set_group(keypad, group);
 *
 * The focused widget shows the theme's focus ring, in both colour modes.
 *
 * All functions must be called from the LVGL thread.
 */

#ifndef TRLV_DEMO_H
#define TRLV_DEMO_H

#include <trelvis/trelvis.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Build the demo inside `parent`: a top bar with the title, a light/dark
 * switch and one button per section, and below it the selected section,
 * which scrolls vertically if it is too tall. The first section is shown
 * at the start.
 *
 * Only the selected section exists: choosing another deletes it and builds
 * the new one, so the demo needs memory for its largest section, not for
 * all of them.
 *
 * The switch calls trlv_theme_set_color_mode with `dark` when turned on and
 * `light` when turned off. It starts on if trlv_init was given `dark`.
 *
 * @param parent  usually a screen; any object with a fixed size works. The
 *                demo fills it.
 * @param light   the light colour tokens; must stay valid while the demo
 *                exists (const tokens in flash, as in the template)
 * @param dark    the dark colour tokens, likewise. If either is NULL the
 *                switch is left out.
 * @return        the demo's top-level object (a column)
 */
lv_obj_t * trlv_demo_create(lv_obj_t * parent, const trlv_color_tokens_t * light,
                            const trlv_color_tokens_t * dark);

/*---------------------------------------------------------------------------
 * Sections. Each builds a column that fills the width of `parent` and hugs
 * its content's height, so `parent` needs a width. Put it in a scrolling
 * container if it may be taller than the display.
 *-------------------------------------------------------------------------*/

/**
 * Readouts: the milestone 2 screen built from boxes, rows, columns and text:
 * a header, a list of readouts, wrapping body text and a row of buttons.
 */
lv_obj_t * trlv_demo_readouts_create(lv_obj_t * parent);

/**
 * Surfaces: every MD3 surface and container role, and the custom colours,
 * as swatches whose text takes the `on_` colour by inheritance.
 */
lv_obj_t * trlv_demo_surfaces_create(lv_obj_t * parent);

/**
 * Cards: a filled, an elevated and an outlined MD3 card, each with a title,
 * body text and its actions.
 */
lv_obj_t * trlv_demo_cards_create(lv_obj_t * parent);

/**
 * Controls: every button type and every widget the theme covers, in
 * their default, checked and disabled states where they apply.
 */
lv_obj_t * trlv_demo_controls_create(lv_obj_t * parent);

/**
 * List: a settings list built from rows and dividers, the replacement for
 * lv_list, with icons, supporting text and trailing switches and values.
 */
lv_obj_t * trlv_demo_list_create(lv_obj_t * parent);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_DEMO_H */
