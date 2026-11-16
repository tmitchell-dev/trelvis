/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_text.h
 * Text primitive and MD3 type-scale roles.
 *
 * Provisional: the milestone 2 slice. The full API, with doc comments and the
 * mapping table, is written in the milestone 3 API review. See
 * docs/concepts.md sections 4.2, 7.1 and 9 for the design.
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
 *   trlv_fill_width or lv_obj_set_width, and it wraps onto more lines
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
 */
lv_obj_t * trlv_text(lv_obj_t * parent, const char * text, trlv_text_role_t role);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_TEXT_H */
