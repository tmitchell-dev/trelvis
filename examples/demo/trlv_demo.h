/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo.h
 * TreLVis demo. Not part of the library: an example a host project builds
 * next to it and calls from its own simulator or target.
 *
 * The demo is a set of sections, one per area of the library, with a row of
 * buttons to move between them. Each section is also a function of its own,
 * so a host can show one section on its own screen.
 *
 * Call trlv_init first. The demo is written for the token template, but any
 * complete set of tokens works:
 *
 *     trlv_init(disp, &tokens_light, &tokens_regular);
 *     trlv_demo_create(lv_display_get_screen_active(disp));
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
 * Build the demo inside `parent`: a top bar with the title and one button
 * per section, and below it the selected section, which scrolls vertically
 * if it is too tall. The first section is shown at the start.
 *
 * Only the selected section exists: choosing another deletes it and builds
 * the new one, so the demo needs memory for its largest section, not for
 * all of them.
 *
 * @param parent  usually a screen; any object with a fixed size works. The
 *                demo fills it.
 * @return        the demo's top-level object (a column)
 */
lv_obj_t * trlv_demo_create(lv_obj_t * parent);

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

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_DEMO_H */
