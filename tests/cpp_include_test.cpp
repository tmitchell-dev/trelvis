/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Compile-only test: the public API must be usable from C++17, warning-free,
 * and keep the signatures approved in the API review.
 */

#include <trelvis/trelvis.h>

#include <type_traits>

static_assert(TRLV_VERSION_MAJOR >= 0, "TreLVis headers must be usable from C++");

/* Token types are usable as C++ types (arrays sized by the scale enums). */
static_assert(sizeof(trlv_layout_tokens_t) > 0, "layout tokens");
static_assert(sizeof(trlv_color_tokens_t) > 0, "color tokens");
static_assert(TRLV_TEXT_COUNT == 15, "MD3 type scale has 15 roles");

/* Custom colours: a project-defined table, the same struct shape whatever
 * the configuration. */
static_assert(std::is_same<decltype(trlv_color_tokens_t::custom), const trlv_custom_color_t *>::value,
              "custom colour table");
static_assert(std::is_same<decltype(trlv_color_tokens_t::custom_count), uint32_t>::value,
              "custom colour count");
static_assert(std::is_same<decltype(trlv_custom_color_t::on_color_container), lv_color_t>::value,
              "custom colour fields");
static_assert(TRLV_CUSTOM_COLOR_MAX >= 0, "custom colour style count");

/* The token template's header is included from C++ projects too, and
 * declares the collections with the token types. */
#include "trlv_tokens_template.h"

static_assert(TOKEN_CUSTOM_COUNT <= TRLV_CUSTOM_COLOR_MAX,
              "the template's custom colours all work with trlv_surface_custom");

static_assert(std::is_same<decltype(tokens_light), const trlv_color_tokens_t>::value,
              "template light colours");
static_assert(std::is_same<decltype(tokens_dark), const trlv_color_tokens_t>::value,
              "template dark colours");
static_assert(std::is_same<decltype(tokens_regular), const trlv_layout_tokens_t>::value,
              "template regular layout");

/*
 * The v1 API as approved in the milestone 3 review. Each check pins one
 * public function's exact signature, so a change to the API fails here and
 * has to be made on purpose (with the docs and the changelog).
 */
#define TRLV_CHECK_SIGNATURE(fn, type) \
    static_assert(std::is_same<decltype(&fn), type>::value, #fn " signature")

/* trlv_theme.h: initialisation and token access */
TRLV_CHECK_SIGNATURE(trlv_init, void (*)(lv_display_t *, const trlv_color_tokens_t *, const trlv_layout_tokens_t *));
TRLV_CHECK_SIGNATURE(trlv_theme_set_color_mode, void (*)(const trlv_color_tokens_t *));
TRLV_CHECK_SIGNATURE(trlv_color, const trlv_color_tokens_t * (*)(void));
TRLV_CHECK_SIGNATURE(trlv_layout, const trlv_layout_tokens_t * (*)(void));

/* trlv_layout.h: constructors */
TRLV_CHECK_SIGNATURE(trlv_box, lv_obj_t * (*)(lv_obj_t *));
TRLV_CHECK_SIGNATURE(trlv_row, lv_obj_t * (*)(lv_obj_t *, trlv_space_t));
TRLV_CHECK_SIGNATURE(trlv_column, lv_obj_t * (*)(lv_obj_t *, trlv_space_t));
TRLV_CHECK_SIGNATURE(trlv_spacer, lv_obj_t * (*)(lv_obj_t *));
TRLV_CHECK_SIGNATURE(trlv_divider, lv_obj_t * (*)(lv_obj_t *));

/* trlv_layout.h: modifiers */
TRLV_CHECK_SIGNATURE(trlv_gap, void (*)(lv_obj_t *, trlv_space_t));
TRLV_CHECK_SIGNATURE(trlv_pad, void (*)(lv_obj_t *, trlv_space_t));
TRLV_CHECK_SIGNATURE(trlv_justify, void (*)(lv_obj_t *, trlv_justify_t));
TRLV_CHECK_SIGNATURE(trlv_align_items, void (*)(lv_obj_t *, trlv_align_items_t));
TRLV_CHECK_SIGNATURE(trlv_hug_width, void (*)(lv_obj_t *));
TRLV_CHECK_SIGNATURE(trlv_hug_height, void (*)(lv_obj_t *));
TRLV_CHECK_SIGNATURE(trlv_fill_width, void (*)(lv_obj_t *));
TRLV_CHECK_SIGNATURE(trlv_fill_height, void (*)(lv_obj_t *));

/* trlv_text.h */
TRLV_CHECK_SIGNATURE(trlv_text, lv_obj_t * (*)(lv_obj_t *, const char *, trlv_text_role_t));
TRLV_CHECK_SIGNATURE(trlv_text_role, void (*)(lv_obj_t *, trlv_text_role_t));

/* trlv_surface.h */
TRLV_CHECK_SIGNATURE(trlv_surface, void (*)(lv_obj_t *, trlv_surface_t));
TRLV_CHECK_SIGNATURE(trlv_surface_custom, void (*)(lv_obj_t *, uint32_t, trlv_custom_surface_t));
TRLV_CHECK_SIGNATURE(trlv_card, void (*)(lv_obj_t *, trlv_card_t));
TRLV_CHECK_SIGNATURE(trlv_button_variant, void (*)(lv_obj_t *, trlv_button_t));

/* trlv_styles.h */
static_assert(std::is_same<decltype(trlv_style_box), const lv_style_t>::value, "trlv_style_box");
static_assert(std::is_same<decltype(trlv_style_row), const lv_style_t>::value, "trlv_style_row");
static_assert(std::is_same<decltype(trlv_style_column), const lv_style_t>::value, "trlv_style_column");

/* Enum sizes that the token arrays and the docs rely on. */
static_assert(TRLV_SPACE_COUNT == 9, "spacing scale NONE..3XL");
static_assert(TRLV_SURFACE_COUNT == 11, "surface roles");
static_assert(TRLV_CUSTOM_SURFACE_COUNT == 2, "custom colour pairs");
static_assert(TRLV_CARD_COUNT == 3, "MD3 card types");
static_assert(TRLV_BUTTON_COUNT == 4, "v1 button types");
static_assert(static_cast<int>(TRLV_JUSTIFY_SPACE_EVENLY) == static_cast<int>(LV_FLEX_ALIGN_SPACE_EVENLY),
              "justify values equal lv_flex_align_t");
static_assert(static_cast<int>(TRLV_ALIGN_ITEMS_END) == static_cast<int>(LV_FLEX_ALIGN_END),
              "align-items values equal lv_flex_align_t");
