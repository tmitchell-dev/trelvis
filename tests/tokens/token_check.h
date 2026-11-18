/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file token_check.h
 * A small check of a colour token set against what TreLVis assumes. It is
 * an aid, not a validator: it reports problems with printf and enforces
 * nothing. TreLVis runs it on its token template in CI; a project can
 * build token_check.c with its own tokens (see docs/building-a-token-set.md).
 *
 * It checks:
 *
 * - each `on_` colour against the colour it sits on, and lower-emphasis
 *   text (`on_surface_variant`) against every surface: at least 4.5:1, the
 *   WCAG contrast ratio for body text;
 * - `outline` and `focus` against `surface`: at least 3:1, the WCAG ratio
 *   for UI parts;
 * - each custom colour's two pairs at 4.5:1, and that no custom colour's
 *   strong colour is `error`, `primary` or `focus`;
 * - that two colour modes have the same number of custom colours.
 *
 * It can't check what it can't see: whether the custom colours are in the
 * same order in every mode, whether values meet a product's standard, or
 * whether a design reads well. Those stay the project's responsibility.
 *
 * Not part of the TreLVis library: test code, built on a host. Needs the
 * C maths library (pow).
 */

#ifndef TOKEN_CHECK_H
#define TOKEN_CHECK_H

#include <trelvis/trelvis.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Minimum contrast for text on its background (WCAG 2.x, normal text). */
#define TOKEN_CHECK_TEXT_CONTRAST 4.5

/** Minimum contrast for UI parts such as borders and focus rings (WCAG 2.x). */
#define TOKEN_CHECK_UI_CONTRAST 3.0

/**
 * The WCAG 2.x contrast ratio of two colours, from 1 (the same) to 21
 * (black on white).
 */
double token_check_contrast(lv_color_t a, lv_color_t b);

/**
 * Check one colour mode. Prints one line per problem, naming the mode and
 * the roles.
 *
 * @param mode_name  a name for the messages, such as "tokens_light"
 * @param color      the colour tokens of one mode
 * @return           the number of problems found
 */
uint32_t token_check_color_mode(const char * mode_name, const trlv_color_tokens_t * color);

/**
 * Check that two colour modes have the same custom colour count. Prints one
 * line if they differ.
 *
 * @return  1 if they differ, else 0
 */
uint32_t token_check_color_modes_match(const char * a_name, const trlv_color_tokens_t * a,
                                       const char * b_name, const trlv_color_tokens_t * b);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TOKEN_CHECK_H */
