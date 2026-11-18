/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_text.h
 * Text primitive and MD3 type-scale roles.
 *
 * Mapping: Figma / MD3 → CSS → TreLVis → LVGL mechanism
 *
 * | Figma / MD3                       | CSS                               | TreLVis                   | LVGL mechanism                                                                         |
 * |-----------------------------------|-----------------------------------|---------------------------|----------------------------------------------------------------------------------------|
 * | Text layer with an MD3 text style | `p`, `h1`, ... with `font`        | trlv_text_create          | lv_label + shared role style                                                           |
 * | Text style on a frame             | `font` on a container (inherited) | trlv_apply_text_role      | shared role style: `text_font`, `text_line_space`, `text_letter_space` (all inherited) |
 * | Line height (px)                  | `line-height`                     | role token                | `text_line_space = line_height - font line height`                                     |
 * | Letter spacing                    | `letter-spacing`                  | role token                | `text_letter_space`                                                                    |
 * | Text colour from a colour role    | `color` (inherited)               | trlv_apply_surface, theme | inherited `text_color`                                                                 |
 *
 * Left to LVGL:
 *
 * | Figma / MD3                   | CSS                       | LVGL                                                        |
 * |-------------------------------|---------------------------|-------------------------------------------------------------|
 * | Auto width (hug)              | `width: fit-content`      | the label default                                           |
 * | Auto height (wrap at a width) | `width` + normal wrapping | trlv_set_width_fill, or lv_obj_set_width                    |
 * | Truncate                      | `text-overflow: ellipsis` | lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS)      |
 * | Text alignment                | `text-align`              | lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0) |
 * | Change the text               | —                         | lv_label_set_text, lv_label_set_text_fmt                    |
 *
 * All functions must be called from the LVGL thread.
 */

#ifndef TRLV_TEXT_H
#define TRLV_TEXT_H

#include "trlv_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a text: a Figma text layer with an MD3 text style, CSS `p`, `h1`
 * and so on with `font`, `line-height` and `letter-spacing`.
 *
 * An lv_label with the role's shared style, which sets the font, line
 * spacing and letter spacing from the layout tokens. It replaces:
 *
 *     lv_obj_t * label = lv_label_create(parent);
 *     lv_label_set_text(label, "Temperature");
 *     lv_obj_set_style_text_font(label, &font_title_medium, 0);
 *     lv_obj_set_style_text_line_space(label, 24 - lv_font_get_line_height(&font_title_medium), 0);
 *     lv_obj_set_style_text_letter_space(label, 0, 0);
 *
 * Behaves like any LVGL label:
 *
 * - **Hugs its text** (Figma "auto width"): it is as wide as its longest
 *   line and doesn't wrap. Give it a width, for example with
 *   trlv_set_width_fill or lv_obj_set_width, and it wraps onto more lines
 *   (Figma "auto height"). To cut the text off instead, use
 *   lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS).
 * - **Inherits its colour** from the enclosing object; the role sets no
 *   colour.
 * - The text is copied, as with lv_label_set_text. Change it later with
 *   lv_label_set_text.
 *
 * Line height: the role's `line_height` token is a Figma line height in px.
 * LVGL has line *spacing* instead, so the style sets
 * `text_line_space = line_height - lv_font_get_line_height(font)`. A
 * `line_height` of 0 keeps the font's own line height. As in LVGL, the
 * spacing goes between lines only: a one-line text is as tall as the font's
 * line height, not the token's.
 *
 * The role style is a shared style (one per role), so it costs no RAM per
 * object.
 *
 * @param parent  the parent object
 * @param text    the text, copied into the label
 * @param role    a role of the MD3 type scale, such as TRLV_TEXT_BODY_MEDIUM
 * @return        the new label
 *
 * @ingroup trlv_create
 */
lv_obj_t * trlv_text_create(lv_obj_t * parent, const char * text, trlv_text_role_t role);

/**
 * Apply an MD3 text role to any object. Figma: apply a text style; CSS
 * set `font`, `line-height` and `letter-spacing`.
 *
 * Applies the same shared role style trlv_text_create uses. On a label it
 * changes the label's role. On a container it sets the role for every label
 * inside, at any depth, because LVGL's font, line spacing and letter spacing
 * inherit like CSS `font`. Labels with a role of their own (from
 * trlv_text_create or trlv_apply_text_role) keep it.
 *
 * Inheritance works because the theme sets roles on screens and buttons,
 * never on plain labels: the screen gives Body Medium, a button Label Large.
 *
 * Calling it again, or calling it on a label made with trlv_text_create,
 * replaces the previous role. The style is shared (one per role), so it costs
 * no RAM per object.
 *
 * @param obj   any object
 * @param role  a role of the MD3 type scale, such as TRLV_TEXT_LABEL_LARGE;
 *              any other value logs a warning and changes nothing
 *
 * @ingroup trlv_apply
 */
void trlv_apply_text_role(lv_obj_t * obj, trlv_text_role_t role);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_TEXT_H */
