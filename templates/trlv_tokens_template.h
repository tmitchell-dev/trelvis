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
 *     If you rename the include guard too, give it a project name such
 *     as TOKENS_H: the TRLV_ prefix is the library's, and TRLV_TOKENS_H
 *     is the guard of <trelvis/trlv_tokens.h>.
 *  2. In the .c file, change the #include of this header to the new name.
 *  3. Build the .c file with your own sources.
 *  4. Pass the tokens to trlv_init:
 *
 *         #include "tokens.h"
 *         trlv_init(disp, &tokens_light, &tokens_regular);
 *
 * The template works unedited. Then make it your product's:
 *
 *  - regenerate the colours from your seed colour (Material Theme Builder),
 *    or choose them yourself;
 *  - replace the example status colours with your own (alarm priorities,
 *    status colours, extra brand accents), or delete them;
 *  - give each text role a real font and its Figma line height;
 *  - delete what you don't use: the second colour mode, or the layout
 *    mode you don't ship (Regular or Compact).
 *
 * docs/building-a-token-set.md says what each token is used for and what
 * TreLVis assumes about it; docs/concepts.md section 4 has the token types
 * and reference values.
 */

#ifndef TOKENS_TEMPLATE_H
#define TOKENS_TEMPLATE_H

#include <trelvis/trelvis.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The project's custom colours: the index into trlv_color_tokens_t::custom and
 * the argument of trlv_apply_surface_custom. The four entries are examples
 * of status colours: rename them, replace them or delete them. If you need
 * more than TRLV_CUSTOM_COLOR_MAX (default 4) with
 * trlv_apply_surface_custom, raise it in trlv_conf.h.
 *
 * There is no "error" entry on purpose: MD3's `error` role is for input
 * and action errors (an invalid field, a failed save), and a process fault
 * should never look like one.
 */
typedef enum {
    TOKEN_CUSTOM_FAULT,     /**< Example: fault, or a high-priority alarm */
    TOKEN_CUSTOM_WARNING,   /**< Example: warning, or a medium-priority alarm */
    TOKEN_CUSTOM_ADVISORY,  /**< Example: advisory, or a low-priority alarm */
    TOKEN_CUSTOM_BAD_DATA,  /**< Example: bad or uncertain data quality */
    TOKEN_CUSTOM_COUNT      /**< Number of custom colours; not a colour */
} token_custom_color_t;

/** Color collection, light mode: the MD3 baseline light scheme. */
extern const trlv_color_tokens_t tokens_light;

/** Color collection, dark mode: the MD3 baseline dark scheme. */
extern const trlv_color_tokens_t tokens_dark;

/** Layout collection, Regular mode: the MD3 reference values. */
extern const trlv_layout_tokens_t tokens_regular;

/** Layout collection, Compact mode: denser, tone-only elevation. */
extern const trlv_layout_tokens_t tokens_compact;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TOKENS_TEMPLATE_H */
