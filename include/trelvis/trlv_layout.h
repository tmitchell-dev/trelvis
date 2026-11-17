/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_layout.h
 * Layout primitives (box, row, column, spacer, divider) and layout
 * modifiers: Figma auto layout, CSS flexbox.
 *
 * Constructors are nouns that create an object and return a plain
 * lv_obj_t *. Modifiers are verbs or properties that work on any object,
 * including ones TreLVis didn't create; each replaces its own previous
 * style rather than adding another. Raw LVGL calls stay valid on every
 * object.
 *
 * Mapping: Figma / MD3 → CSS → TreLVis → LVGL mechanism
 *
 * | Figma / MD3                    | CSS                                   | TreLVis                         | LVGL mechanism                               |
 * |--------------------------------|---------------------------------------|---------------------------------|----------------------------------------------|
 * | Frame, no auto layout          | `div`                                 | trlv_box                        | lv_obj + reset style, scroll and click off   |
 * | Auto layout → / ↓              | `display: flex; flex-direction: ...`  | trlv_row / trlv_column          | `layout = LV_LAYOUT_FLEX`, `flex_flow`       |
 * | Empty frame, fill container    | `flex: 1`                             | trlv_spacer                     | `flex_grow = 1`                              |
 * | Divider (MD3)                  | `hr`                                  | trlv_divider                    | box + `bg_color = outline_variant`           |
 * | Gap between items              | `gap`                                 | trlv_gap                        | `pad_row`, `pad_column`                      |
 * | Padding                        | `padding`                             | trlv_pad                        | `pad_top/bottom/left/right`                  |
 * | Alignment, space between       | `justify-content`                     | trlv_justify                    | `flex_main_place`                            |
 * | Alignment (cross axis)         | `align-items`                         | trlv_align_items                | `flex_cross_place` + `flex_track_place`      |
 * | Hug contents                   | `width: fit-content`                  | trlv_hug_width / _height        | LV_SIZE_CONTENT                              |
 * | Fill container                 | `flex: 1` / `align-self: stretch`     | trlv_fill_width / _height       | `flex_grow = 1` or LV_PCT(100)               |
 *
 * Left to LVGL: these are already one clear call, so TreLVis adds no
 * function. Setters named lv_obj_set_style_* add a local style to the
 * object; read token values with trlv_layout().
 *
 * | Figma / MD3                    | CSS                                   | LVGL                                                                  |
 * |--------------------------------|---------------------------------------|-----------------------------------------------------------------------|
 * | Horizontal / vertical gap      | `column-gap`, `row-gap`               | lv_obj_set_style_pad_column(obj, trlv_layout()->space[...], 0), `..._pad_row` |
 * | Horizontal / vertical padding  | `padding: y x`                        | lv_obj_set_style_pad_hor(obj, trlv_layout()->space[...], 0), `..._pad_ver`    |
 * | Padding on one side            | `padding-left`, ...                   | lv_obj_set_style_pad_left(obj, trlv_layout()->space[...], 0), ...    |
 * | Wrap                           | `flex-wrap: wrap`                     | lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP)                      |
 * | Line placement when wrapping   | `align-content`                       | lv_obj_set_style_flex_track_place(obj, LV_FLEX_ALIGN_..., 0)          |
 * | Fixed size                     | `width`, `height`                     | lv_obj_set_size, lv_obj_set_width, lv_obj_set_height                 |
 * | Min / max size                 | `min-width`, `max-width`, ...         | lv_obj_set_style_min_width(obj, px, 0), ...                           |
 * | Absolute position              | `position: absolute`                  | lv_obj_set_floating(obj, true), then lv_obj_align(obj, LV_ALIGN_..., x, y) |
 * | Centre in the parent           | `place-self: center`                  | lv_obj_center                                                         |
 * | Grid auto layout               | `display: grid`                       | lv_obj_set_grid_dsc_array on a trlv_box, lv_obj_set_grid_cell         |
 * | Clip content                   | `overflow: hidden`                    | LVGL default; lv_obj_set_style_clip_corner for rounded corners       |
 * | Scroll                         | `overflow: auto`                      | lv_obj_set_scrollable(obj, true), lv_obj_set_scroll_dir               |
 * | Hidden                         | `display: none`                       | lv_obj_set_hidden(obj, true); hidden objects leave the layout         |
 *
 * For an absolute position, lv_obj_set_floating matches Figma: the object
 * is left out of the parent's auto layout *and* of its hug size. (The
 * ignore-layout flag also skips the layout, but still counts towards the
 * parent's hug size.) A floating object doesn't scroll with its parent.
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
 * Types
 *-------------------------------------------------------------------------*/

/**
 * How children are distributed along the main axis: Figma auto layout
 * alignment and "space between", CSS `justify-content`. The values equal
 * LVGL's lv_flex_align_t.
 */
typedef enum {
    TRLV_JUSTIFY_START         = LV_FLEX_ALIGN_START,         /**< Packed at the start (LVGL default) */
    TRLV_JUSTIFY_CENTER        = LV_FLEX_ALIGN_CENTER,        /**< Packed in the centre */
    TRLV_JUSTIFY_END           = LV_FLEX_ALIGN_END,           /**< Packed at the end */
    TRLV_JUSTIFY_SPACE_BETWEEN = LV_FLEX_ALIGN_SPACE_BETWEEN, /**< First and last at the edges,
                                                                   equal space between (Figma
                                                                   "space between") */
    TRLV_JUSTIFY_SPACE_AROUND  = LV_FLEX_ALIGN_SPACE_AROUND,  /**< Equal space around each child */
    TRLV_JUSTIFY_SPACE_EVENLY  = LV_FLEX_ALIGN_SPACE_EVENLY   /**< Equal space between and at the edges */
} trlv_justify_t;

/**
 * How children are placed across the main axis: Figma auto layout
 * alignment, CSS `align-items`. The values equal LVGL's lv_flex_align_t.
 *
 * Named ALIGN_ITEMS, not ALIGN, so they can't be mistaken for LVGL's
 * lv_align_t (LV_ALIGN_CENTER and so on), which positions an object.
 * There is no STRETCH (LVGL flex has none): use trlv_fill_width or
 * trlv_fill_height on the children. There is no BASELINE either; CENTER is
 * the closest for texts of different sizes.
 */
typedef enum {
    TRLV_ALIGN_ITEMS_START  = LV_FLEX_ALIGN_START,  /**< Top of a row, left of a column (LVGL default) */
    TRLV_ALIGN_ITEMS_CENTER = LV_FLEX_ALIGN_CENTER, /**< Centred across the axis */
    TRLV_ALIGN_ITEMS_END    = LV_FLEX_ALIGN_END     /**< Bottom of a row, right of a column */
} trlv_align_items_t;

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
 * is 0 px; it logs an LV_LOG_WARN naming trlv_spacer, like trlv_fill_*.
 * In a parent with no flex layout it has no size.
 *
 * @param parent  the parent object, normally a row or column
 * @return        the new spacer
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
 * object. Calling it again replaces the previous gap. For different
 * horizontal and vertical gaps, use raw LVGL (see the table above).
 *
 * @param obj    any object; the gap applies to its flex or grid children
 * @param space  a step of the spacing scale
 */
void trlv_gap(lv_obj_t * obj, trlv_space_t space);

/**
 * Set the padding on all four sides: Figma auto layout padding, CSS
 * `padding`. Sets LVGL `pad_top`, `pad_bottom`, `pad_left` and `pad_right`
 * to the token value. For one side, or different horizontal and vertical
 * padding, use raw LVGL (see the table above), which adds a local style.
 *
 * Applies a shared style (one per space step), so it costs no RAM per
 * object. Calling it again replaces the previous padding.
 *
 * As in Figma with an inside stroke included in layout (CSS
 * `box-sizing: border-box`), a border sits inside the object's size and the
 * padding is measured from it: children are inset by the border width plus
 * the padding.
 *
 * @param obj    any object
 * @param space  a step of the spacing scale
 */
void trlv_pad(lv_obj_t * obj, trlv_space_t space);

/**
 * Distribute the children along the main axis: Figma auto layout alignment
 * and "space between", CSS `justify-content`. Sets LVGL `flex_main_place`.
 *
 * LVGL's lv_obj_set_flex_align sets the main, cross and track placement in
 * one call, so changing one means repeating the other two, and it adds a
 * local style. trlv_justify and trlv_align_items set one each, by the CSS
 * name, with a shared constant style (one per value) that costs no RAM per
 * object. Calling it again replaces the previous value.
 *
 * Spacing in a SPACE_* mode comes on top of the gap. With
 * TRLV_JUSTIFY_SPACE_BETWEEN the children need a parent wider than their
 * total (a fixed or filled size); in a parent that hugs there is no free
 * space and it looks like START.
 *
 * @param obj      any object with a flex layout, such as a row or column
 * @param justify  how to distribute the children; any other value logs a
 *                 warning and changes nothing
 */
void trlv_justify(lv_obj_t * obj, trlv_justify_t justify);

/**
 * Place the children across the main axis: Figma auto layout alignment,
 * CSS `align-items`. Centre a row of texts of different sizes with
 * TRLV_ALIGN_ITEMS_CENTER.
 *
 * Deliberate difference from LVGL: this sets **both** `flex_cross_place`
 * and `flex_track_place` to the value. LVGL places items within their line
 * (track), which is only as tall as the tallest item and sits at the top of
 * the container, so `flex_cross_place` alone centres a short item against
 * its tallest sibling, not in the row. Placing the line too gives the Figma
 * and CSS result: the items are centred (or at the end) of the container.
 *
 * In a wrapping row, the lines are packed together at the same place, as in
 * Figma. For CSS `align-content`, set the line placement afterwards with
 * lv_obj_set_style_flex_track_place, a local style, which takes precedence.
 *
 * Applies a shared constant style (one per value), so it costs no RAM per
 * object. Calling it again replaces the previous value. See trlv_justify
 * for why these are separate calls.
 *
 * @param obj    any object with a flex layout, such as a row or column
 * @param align  where to place the children across the axis; any other
 *               value logs a warning and changes nothing
 */
void trlv_align_items(lv_obj_t * obj, trlv_align_items_t align);

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
