/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_tokens_template.h
 * Token template: declarations for the token values in
 * trlv_tokens_template.c.
 *
 * TreLVis compiles in no token values. Your project owns them, starting from
 * this template (the same way lv_conf_template.h works in LVGL):
 *
 *  1. Copy trlv_tokens_template.h and trlv_tokens_template.c into your
 *     project and rename them, for example to tokens.h and tokens.c.
 *  2. In the .c file, change the #include of this header to the new name.
 *  3. Build the .c file with your own sources.
 *  4. Pass the tokens to trlv_init:
 *
 *         #include "tokens.h"
 *         trlv_init(disp, &tokens_light, &tokens_regular);
 *
 * The template works unedited. Then make it your product's:
 *
 *  - regenerate the colours from your seed colour (Material Theme Builder);
 *  - give each text role a real font and its Figma line height;
 *  - delete what you don't use, and add a Compact layout if you need one.
 *
 * See docs/concepts.md section 4 for the token types and reference values.
 */

#ifndef TRLV_TOKENS_TEMPLATE_H
#define TRLV_TOKENS_TEMPLATE_H

#include <trelvis/trelvis.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Color collection, light mode: the MD3 baseline light scheme. */
extern const trlv_color_tokens_t tokens_light;

/** Color collection, dark mode: the MD3 baseline dark scheme. */
extern const trlv_color_tokens_t tokens_dark;

/** Layout collection, Regular mode. */
extern const trlv_layout_tokens_t tokens_regular;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_TOKENS_TEMPLATE_H */
