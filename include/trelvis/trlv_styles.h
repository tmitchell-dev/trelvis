/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_styles.h
 * Catalogue of the shared LVGL styles TreLVis creates (layout, spacing, surfaces, cards, text, buttons).
 *
 * The constructors and modifiers apply these same styles. A project can also
 * use them directly with lv_obj_add_style(obj, &trlv_style_..., 0).
 *
 * Provisional: the milestone 2 slice exposes the layout styles only. The full
 * catalogue is written in the milestone 3 API review. See docs/concepts.md
 * section 6.5.
 */

#ifndef TRLV_STYLES_H
#define TRLV_STYLES_H

#include "trlv_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Box reset (constant): transparent background, no border, no padding, and
 * hug (LV_SIZE_CONTENT) width and height. Applied by trlv_box.
 */
extern const lv_style_t trlv_style_box;

/** Horizontal flex flow (constant): Figma auto layout →, CSS
 *  `display: flex; flex-direction: row`. Applied by trlv_row. */
extern const lv_style_t trlv_style_row;

/** Vertical flex flow (constant): Figma auto layout ↓, CSS
 *  `display: flex; flex-direction: column`. Applied by trlv_column. */
extern const lv_style_t trlv_style_column;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_STYLES_H */
