/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_theme.h
 * Initialisation, the MD3 theme for built-in LVGL widgets, and runtime colour-mode switching.
 *
 * Provisional: the milestone 2 slice has initialisation and token access but
 * no theme yet. The full API, with the theme and colour-mode switching, is
 * written in the milestone 3 API review. See docs/concepts.md section 6.3.
 */

#ifndef TRLV_THEME_H
#define TRLV_THEME_H

#include "trlv_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise TreLVis with a project's tokens. Call once after creating the
 * display and before using any other TreLVis function. LVGL thread only.
 *
 * Builds the shared styles that depend on tokens (gap, pad, text roles).
 * The token structs are not copied: they must stay valid for as long as
 * TreLVis is used, which `const` globals from the token template do.
 *
 * Calling it again with other tokens re-sets the existing shared styles in
 * place, so LVGL's style memory does not grow.
 *
 * The milestone 2 slice installs no theme yet; `disp` is checked but not
 * otherwise used.
 *
 * @param disp    the display to theme. Must not be NULL: LVGL v10 no longer
 *                accepts NULL for "the default display". A NULL display,
 *                colour or layout is rejected with LV_LOG_WARN and nothing
 *                changes.
 * @param color   colour collection for the starting colour mode (e.g. light)
 * @param layout  layout collection (e.g. Regular); chosen once, at start-up
 */
void trlv_init(lv_display_t * disp, const trlv_color_tokens_t * color,
               const trlv_layout_tokens_t * layout);

/** The colour tokens in use: the Color collection's current mode.
 *  NULL before trlv_init. */
const trlv_color_tokens_t * trlv_color(void);

/** The layout tokens in use: the Layout collection's mode. NULL before trlv_init. */
const trlv_layout_tokens_t * trlv_layout(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_THEME_H */
