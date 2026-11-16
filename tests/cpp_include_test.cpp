/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Compile-only test: the public API must be usable from C++17, warning-free.
 */

#include <trelvis/trelvis.h>

static_assert(TRLV_VERSION_MAJOR >= 0, "TreLVis headers must be usable from C++");

/* Token types are usable as C++ types (arrays sized by the scale enums). */
static_assert(sizeof(trlv_layout_tokens_t) > 0, "layout tokens");
static_assert(sizeof(trlv_color_tokens_t) > 0, "color tokens");
static_assert(TRLV_TEXT_COUNT == 15, "MD3 type scale has 15 roles");

/* The token template's header is included from C++ projects too. */
#include "trlv_tokens_template.h"

static const trlv_color_tokens_t * const template_light = &tokens_light;
static const trlv_layout_tokens_t * const template_regular = &tokens_regular;
static_assert(sizeof(template_light) > 0 && sizeof(template_regular) > 0, "template tokens");
