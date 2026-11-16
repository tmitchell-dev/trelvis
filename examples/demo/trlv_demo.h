/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo.h
 * TreLVis demo screen. Not part of the library: an example a host project
 * builds next to it and calls from its own simulator or target.
 *
 * Call from the LVGL thread.
 */

#ifndef TRLV_DEMO_H
#define TRLV_DEMO_H

#include <trelvis/trelvis.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Build the demo screen inside `parent`: a header, a list of readouts, a
 * section of wrapping body text and a row of buttons. It fills `parent`
 * and scrolls vertically if `parent` is too short for it.
 *
 * Call trlv_init first, with the unedited token template:
 *
 *     trlv_init(disp, &tokens_light, &tokens_regular);
 *     trlv_demo_create(lv_display_get_screen_active(disp));
 *
 * @param parent  usually a screen; any object with a fixed size works
 * @return        the demo's top-level object (a column)
 */
lv_obj_t * trlv_demo_create(lv_obj_t * parent);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_DEMO_H */
