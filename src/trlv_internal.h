/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_internal.h
 * Internal: token styles, and how trlv_init reaches each module. Not part of
 * the public API.
 *
 * Token styles (gap, pad, text roles, later surfaces and cards) are
 * LV_STYLE_CONST_INIT styles, like the constant styles, except that their
 * property array is a static, writable array in RAM rather than a `const`
 * one in flash:
 *
 *     static lv_style_const_prop_t gap_md_props[3];
 *     static LV_STYLE_CONST_INIT(gap_md, gap_md_props);
 *
 * The lv_style_t itself stays in flash. Each module's build function writes
 * the property ids and token values into the arrays; it never calls
 * lv_style_init or lv_style_set_*. So:
 *
 *  - TreLVis never allocates for styles, not even through LVGL;
 *  - building again (trlv_init again, later colour-mode switches) overwrites
 *    the same arrays in place, so memory use is fixed at compile time;
 *  - nothing points into LVGL's heap, so lv_deinit followed by lv_init and
 *    trlv_init leaves nothing stale;
 *  - before trlv_init the arrays are all zero, which is LVGL's end marker
 *    (LV_STYLE_PROP_INV), so a token style applied that early is empty and
 *    takes its values when trlv_init runs.
 *
 * A build function must write every entry, including the end marker, and the
 * caller reports the change to LVGL (lv_obj_report_style_change) afterwards.
 */

#ifndef TRLV_INTERNAL_H
#define TRLV_INTERNAL_H

#include "trelvis/trlv_tokens.h"

/** Compile-time check, usable at file scope (C99 has no static_assert). */
#define TRLV_STATIC_ASSERT(cond, name) typedef char trlv_static_assert_##name[(cond) ? 1 : -1]

/** Write one numeric property into a token style's property array. */
static inline void trlv_prop_set_num(lv_style_const_prop_t * entry, lv_style_prop_t prop, int32_t num)
{
    entry->prop = prop;
    entry->value.num = num;
}

/** Write one pointer property (such as a font) into a token style's property array. */
static inline void trlv_prop_set_ptr(lv_style_const_prop_t * entry, lv_style_prop_t prop, const void * ptr)
{
    entry->prop = prop;
    entry->value.ptr = ptr;
}

/** Write one colour property into a token style's property array. */
static inline void trlv_prop_set_color(lv_style_const_prop_t * entry, lv_style_prop_t prop, lv_color_t color)
{
    entry->prop = prop;
    entry->value.color = color;
}

/**
 * An MD3 state layer, pre-mixed: the content colour over the container at
 * the state's opacity (`state.pressed`, `state.focus`). Used as the
 * container colour of a state style, so the content keeps its own colour
 * (an LVGL colour filter or recolour would tint the content too).
 */
static inline lv_color_t trlv_state_layer(lv_color_t container, lv_color_t content, lv_opa_t opa)
{
    return lv_color_mix(content, container, opa);
}

/** Write the end marker that closes a token style's property array. */
static inline void trlv_prop_set_end(lv_style_const_prop_t * entry)
{
    entry->prop = LV_STYLE_PROP_INV;
    entry->value.num = 0;
}

/** Build the gap and pad styles (trlv_layout.c). */
void trlv_layout_styles_build(const trlv_layout_tokens_t * layout);

/** Build the text role styles (trlv_text.c). */
void trlv_text_styles_build(const trlv_layout_tokens_t * layout);

/** Build the surface, custom colour, card, button variant and divider styles
 *  (trlv_surface.c). Called by trlv_init and on each colour-mode switch. */
void trlv_surface_styles_build(const trlv_color_tokens_t * color, const trlv_layout_tokens_t * layout);

/** The divider's line style, for trlv_divider (trlv_surface.c). */
const lv_style_t * trlv_divider_style(bool vertical);

/** The shared style of a text role, for the theme (trlv_text.c). NULL for
 *  a value that is not a role. */
const lv_style_t * trlv_text_role_style(trlv_text_role_t role);

#endif /* TRLV_INTERNAL_H */
