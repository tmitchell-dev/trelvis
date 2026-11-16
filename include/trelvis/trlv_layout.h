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

/**
 * Create a row: Figma horizontal auto layout (→), CSS
 * `display: flex; flex-direction: row; gap: ...`.
 *
 * A box (see trlv_box: not scrollable, not clickable, no theme styles, hugs
 * its children) with trlv_style_row and the gap style. It replaces:
 *
 *     lv_obj_t * row = lv_obj_create(parent);
 *     lv_obj_remove_style_all(row);
 *     lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
 *     lv_obj_set_scrollable(row, false);
 *     lv_obj_set_clickable(row, false);
 *     lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
 *     lv_obj_set_style_pad_column(row, 8, 0);
 *     lv_obj_set_style_pad_row(row, 8, 0);
 *
 * Children are placed left to right. To wrap them onto more lines, use
 * lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP).
 *
 * @param parent  the parent object
 * @param gap     the gap between children; change it later with trlv_gap
 * @return        the new row
 */
lv_obj_t * trlv_row(lv_obj_t * parent, trlv_space_t gap);

/**
 * Create a column: Figma vertical auto layout (↓), CSS
 * `display: flex; flex-direction: column; gap: ...`.
 *
 * Like trlv_row, with children placed top to bottom.
 *
 * @param parent  the parent object
 * @param gap     the gap between children; change it later with trlv_gap
 * @return        the new column
 */
lv_obj_t * trlv_column(lv_obj_t * parent, trlv_space_t gap);

/**
 * Create a spacer: an empty box that takes the free space along its
 * parent's main axis. Figma: an empty frame set to "fill container"; CSS
 * `flex: 1` on an empty element.
 *
 * Use it to push the items after it to the far end, for example a title
 * left and a group of buttons right. To spread every item evenly, use
 * trlv_justify on the parent instead.
 *
 * It is a trlv_box with trlv_fill_width (in a row) or trlv_fill_height (in
 * a column) applied, and follows the same rules: the axis is chosen from the
 * parent's flex flow **at the time of the call**, so set the parent's
 * direction first. Across the axis it is 0 px and takes no room. Several
 * spacers in one parent share the free space equally.
 *
 * In a parent that hugs its main axis there is no free space, so the spacer
 * is 0 px; it logs the same LV_LOG_WARN as trlv_fill_*. In a parent with no
 * flex layout it has no size.
 *
 * @param parent  the parent object, normally a row or column
 * @return        the new spacer
 *
 * @note Not implemented yet (milestone 5).
 */
lv_obj_t * trlv_spacer(lv_obj_t * parent);

/**
 * Create a divider: an MD3 divider, a thin line in `outline_variant` that
 * separates groups of content. Figma: a 1 px line or rectangle; CSS `hr`.
 *
 * Its orientation follows the parent's flex flow **at the time of the
 * call**:
 *
 * - in a column, or a parent with no flex layout: a horizontal line,
 *   `border_width` tall, filling the parent's width;
 * - in a row: a vertical line, `border_width` wide, filling the row's
 *   height (it stretches to the tallest sibling when the row hugs).
 *
 * The colour is a shared token style, so it follows colour-mode switches.
 * For an MD3 inset divider, add a margin, which LVGL's flex layout honours:
 * lv_obj_set_style_margin_left(divider, trlv_layout()->space[TRLV_SPACE_LG], 0).
 *
 * @param parent  the parent object, normally a row or column
 * @return        the new divider
 *
 * @note Not implemented yet (milestone 5).
 */
lv_obj_t * trlv_divider(lv_obj_t * parent);

/*---------------------------------------------------------------------------
 * Modifiers. Each works on any object, including ones TreLVis did not
 * create, and replaces its own previous style rather than adding another.
 *-------------------------------------------------------------------------*/

/**
 * Set the gap between children: Figma auto layout "gap between items", CSS
 * `gap`. Sets both LVGL `pad_row` and `pad_column` to the token value.
 *
 * Applies a shared style (one per space step), so it costs no RAM per
 * object. Calling it again replaces the previous gap.
 *
 * @param obj    any object; the gap applies to its flex or grid children
 * @param space  a step of the spacing scale
 */
void trlv_gap(lv_obj_t * obj, trlv_space_t space);

/**
 * Set the padding on all four sides: Figma auto layout padding, CSS
 * `padding`. Sets LVGL `pad_top`, `pad_bottom`, `pad_left` and `pad_right`
 * to the token value. For one side only, use raw LVGL
 * (lv_obj_set_style_pad_left and so on), which adds a local style.
 *
 * Applies a shared style (one per space step), so it costs no RAM per
 * object. Calling it again replaces the previous padding.
 *
 * As in Figma, a border is drawn inside the padding box: it doesn't add to
 * the padding.
 *
 * @param obj    any object
 * @param space  a step of the spacing scale
 */
void trlv_pad(lv_obj_t * obj, trlv_space_t space);

/**
 * Hug the content's width: Figma "hug contents", CSS `width: fit-content`.
 * Sets LVGL `width` to LV_SIZE_CONTENT.
 *
 * Hug and fill on one axis replace each other: this clears an earlier
 * trlv_fill_width. A fixed size is set with lv_obj_set_width, which is a
 * local style and so takes precedence over hug and fill.
 *
 * @param obj  any object
 */
void trlv_hug_width(lv_obj_t * obj);

/** Hug the content's height. Like trlv_hug_width, for the height. */
void trlv_hug_height(lv_obj_t * obj);

/**
 * Fill the parent's width: Figma "fill container", CSS `flex: 1` in a row or
 * `align-self: stretch` in a column.
 *
 * LVGL has no single "fill", so this makes the choice from the parent's
 * layout **at the time of the call**:
 *
 * - parent is a row (width is its main axis): `flex_grow = 1`, so the object
 *   takes the free space, shared with any other growing siblings;
 * - parent is a column (cross axis), or has no flex layout: `width = 100 %`.
 *
 * Set the parent's direction first. If it changes later, call this again:
 * the choice stays as made, like any LVGL style.
 *
 * Inside a parent that hugs its width, TreLVis adds no behaviour of its own
 * and LVGL's result stands:
 *
 * - across a column (or without flex), the parent hugs its other children
 *   and the object stretches to the widest of them (CSS
 *   `align-self: stretch`). With no sized sibling, it is 0 wide.
 * - along a row, there is no free space, so the object collapses to 0
 *   wide. This is always a mistake: TreLVis logs an LV_LOG_WARN naming the
 *   object. Give the row a width first. The warning changes nothing and is
 *   compiled out when LVGL logging is off.
 *
 * Hug and fill on one axis replace each other: this clears an earlier
 * trlv_hug_width. A width set with lv_obj_set_width is a local style and
 * takes precedence. On an lv_label, a width makes the text wrap (Figma
 * "auto height").
 *
 * @param obj  any object
 */
void trlv_fill_width(lv_obj_t * obj);

/**
 * Fill the parent's height. Like trlv_fill_width, for the height: grows in
 * a column (main axis), 100 % in a row (cross axis) or without flex layout.
 *
 * @param obj  any object
 */
void trlv_fill_height(lv_obj_t * obj);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_LAYOUT_H */
