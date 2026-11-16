/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_layout.h
 * Layout primitives (box, row, column, spacer, divider) and layout modifiers (Figma auto layout / CSS flexbox).
 *
 * Provisional: the milestone 2 slice. The full API, with doc comments and the
 * mapping table, is written in the milestone 3 API review. See
 * docs/concepts.md section 7 for the design.
 *
 * All functions must be called from the LVGL thread.
 */

#ifndef TRLV_LAYOUT_H
#define TRLV_LAYOUT_H

#include "trlv_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * Constructors
 *-------------------------------------------------------------------------*/

/**
 * Create a box: a Figma frame without auto layout, a CSS `div`.
 *
 * An lv_obj with the theme's styles removed and trlv_style_box added. It is
 * transparent, has no border or padding, and hugs its children.
 *
 * Deliberate differences from lv_obj_create: a box is **not scrollable** and
 * **not clickable**, and has **no theme background, border or padding**.
 * Turn scrolling back on with lv_obj_set_scrollable(box, true) and clicks with
 * lv_obj_set_clickable(box, true).
 *
 * @param parent  the parent object
 * @return        the new box
 */
lv_obj_t * trlv_box(lv_obj_t * parent);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_LAYOUT_H */
