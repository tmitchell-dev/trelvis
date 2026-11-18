/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_styles.h
 * The shared LVGL styles a project can add to an object directly.
 *
 * TreLVis exposes a style only when no modifier can apply it to an object
 * that already exists. trlv_box_create, trlv_row_create and trlv_column_create create new
 * objects, so their styles are here: with them, a project can turn an
 * existing object, such as a plain lv_obj used as a list item, into a row
 * or column while it keeps its theme look, or reset its theme look:
 *
 *     lv_obj_t * item = lv_obj_create(list);
 *     lv_obj_add_style(item, &trlv_style_row, 0);
 *     trlv_set_gap(item, TRLV_SPACE_SM);
 *     lv_image_set_src(lv_image_create(item), &icon_play);
 *     trlv_text_create(item, "Start", TRLV_TEXT_BODY_LARGE);
 *
 * (An lv_button needs none of this: the theme already lays it out as a
 * centred row.)
 *
 * Every other TreLVis style (gap, pad, justify, align items, hug and fill,
 * text roles, surfaces, cards, button variants, divider) stays internal:
 * its modifier works on any object, including inside a project's own theme
 * apply callback, and also replaces the group's previous style, which
 * lv_obj_add_style does not.
 *
 * These styles are constant (LV_STYLE_CONST_INIT): they live in flash, use
 * no RAM and are valid before trlv_init.
 *
 * Style precedence, lowest to highest (LVGL's own rules):
 *
 * 1. theme styles
 * 2. TreLVis styles, from modifiers or added from this header
 * 3. project styles added later with lv_obj_add_style
 * 4. local style properties (lv_obj_set_style_*)
 *
 * Within one level, a style added later wins.
 *
 * Mapping: Figma / MD3 → CSS → TreLVis → LVGL mechanism
 *
 * | Figma / MD3                    | CSS                                       | TreLVis            | LVGL properties                                    |
 * |--------------------------------|-------------------------------------------|--------------------|----------------------------------------------------|
 * | Frame, no fill, stroke or padding | `div` with no styles                  | trlv_style_box     | `bg_opa`, `border_width`, `pad_*` = 0; size LV_SIZE_CONTENT |
 * | Auto layout →                  | `display: flex; flex-direction: row`      | trlv_style_row     | `layout = LV_LAYOUT_FLEX`, `flex_flow = ROW`       |
 * | Auto layout ↓                  | `display: flex; flex-direction: column`   | trlv_style_column  | `layout = LV_LAYOUT_FLEX`, `flex_flow = COLUMN`    |
 *
 * All functions using these styles must be called from the LVGL thread.
 */

#ifndef TRLV_STYLES_H
#define TRLV_STYLES_H

#include "trlv_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Box reset (constant): transparent background, no border, no padding, and
 * hug (LV_SIZE_CONTENT) width and height. Figma: a frame with no fill,
 * stroke or padding; CSS: a `div` with no styles. Applied by trlv_box_create.
 *
 * Added to an existing widget, it overrides the theme's background, border,
 * padding and size, but not its other styles or the widget's flags: an
 * lv_obj stays scrollable and clickable (trlv_box_create turns those off).
 */
extern const lv_style_t trlv_style_box;

/**
 * Horizontal flex flow (constant): Figma auto layout →, CSS
 * `display: flex; flex-direction: row`. Applied by trlv_row_create.
 *
 * Sets only the layout and flow: the object keeps its size, padding and
 * look. Add a gap with trlv_set_gap. trlv_style_row and trlv_style_column are
 * not a group: to change the direction later, remove one with
 * lv_obj_remove_style before adding the other, or use
 * lv_obj_set_flex_flow, which adds a local style and wins.
 */
extern const lv_style_t trlv_style_row;

/**
 * Vertical flex flow (constant): Figma auto layout ↓, CSS
 * `display: flex; flex-direction: column`. Applied by trlv_column_create.
 *
 * Like trlv_style_row, with children placed top to bottom.
 */
extern const lv_style_t trlv_style_column;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_STYLES_H */
