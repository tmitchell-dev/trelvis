/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_internal.h
 * Internal: how trlv_init reaches each module. Not part of the public API.
 *
 * Each module that has token-dependent shared styles provides a build
 * function, and trlv_init calls them all. A build function may be called
 * more than once (trlv_init again, and later colour-mode switches), so it
 * must follow the memory rules in docs/concepts.md section 8:
 *
 *  - call lv_style_init() on a style only the first time (it would drop the
 *    style's existing properties without freeing them);
 *  - then set every property with lv_style_set_*(), which overwrites a
 *    property that is already there instead of allocating again;
 *  - set the same properties on every call, so the allocation stays bounded.
 */

#ifndef TRLV_INTERNAL_H
#define TRLV_INTERNAL_H

#include "trelvis/trlv_tokens.h"

/** Build the gap and pad styles (trlv_layout.c). */
void trlv_layout_styles_build(const trlv_layout_tokens_t * layout);

/** Build the text role styles (trlv_text.c). */
void trlv_text_styles_build(const trlv_layout_tokens_t * layout);

#endif /* TRLV_INTERNAL_H */
