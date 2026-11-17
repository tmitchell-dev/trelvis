/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_tokens_hmi.h
 * Example colour tokens for a process or instrument HMI: an alternative to
 * the template's colour collections, declared in trlv_tokens_hmi.c.
 *
 * It shows that the same token types serve an industrial product as well
 * as a brand-led one, with values only. The scheme follows the
 * high-performance HMI style (ISA-101): neutral greys, colour kept for
 * what needs attention, and a custom colour table of alarm priorities in
 * place of the template's success and warning.
 *
 * These are example values. Nothing here claims compliance with a
 * standard: a real product takes its alarm colours from its alarm
 * philosophy (ISA-18.2) or the applicable standard, as they are.
 *
 * Colour only: use it with a layout collection, such as the template's:
 *
 *     #include "trlv_tokens_hmi.h"
 *     #include "trlv_tokens_template.h"
 *
 *     trlv_init(disp, &tokens_hmi_light, &tokens_regular);
 *     trlv_surface_custom(banner, TOKEN_ALARM_HIGH, TRLV_CUSTOM_SURFACE_COLOR);
 *
 * Copy it into your project the same way as the template (see
 * trlv_tokens_template.h), or take what you need from it.
 */

#ifndef TOKENS_HMI_H
#define TOKENS_HMI_H

#include <trelvis/trelvis.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * The example's custom colours: alarm priorities, the index into
 * trlv_color_tokens_t::custom and the argument of trlv_surface_custom.
 * A site's alarm philosophy sets how many priorities there are and what
 * they look like.
 */
typedef enum {
    TOKEN_ALARM_HIGH,     /**< Example: high-priority alarm */
    TOKEN_ALARM_MEDIUM,   /**< Example: medium-priority alarm */
    TOKEN_ALARM_LOW,      /**< Example: low-priority alarm */
    TOKEN_ALARM_COUNT     /**< Number of alarm colours; not a colour */
} token_alarm_t;

/** Color collection, light mode: a light-grey HMI scheme. */
extern const trlv_color_tokens_t tokens_hmi_light;

/** Color collection, dark mode: a dark-grey scheme for dimmed control rooms. */
extern const trlv_color_tokens_t tokens_hmi_dark;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TOKENS_HMI_H */
