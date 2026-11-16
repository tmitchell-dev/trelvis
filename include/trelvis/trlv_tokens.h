/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_tokens.h
 * MD3 design-token types: the colour and layout collections and the scales
 * they index. TreLVis defines only the types; a project supplies the values
 * by copying the token template (see templates/).
 *
 * Provisional: these are the draft types from docs/concepts.md section 4.1,
 * used by the milestone 2 slice. They are finalised, with full doc comments
 * and the mapping table, in the milestone 3 API review.
 */

#ifndef TRLV_TOKENS_H
#define TRLV_TOKENS_H

#include "trlv_version.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * Scales. Each value is an array index into trlv_layout_tokens_t; the names
 * mirror MD3 / Figma variable names.
 *-------------------------------------------------------------------------*/

/** Spacing scale (Figma `space/sm`, `space/md`, ...): padding and gaps. */
typedef enum {
    TRLV_SPACE_NONE,
    TRLV_SPACE_2XS,
    TRLV_SPACE_XS,
    TRLV_SPACE_SM,
    TRLV_SPACE_MD,
    TRLV_SPACE_LG,
    TRLV_SPACE_XL,
    TRLV_SPACE_2XL,
    TRLV_SPACE_3XL,
    TRLV_SPACE_COUNT
} trlv_space_t;

/** MD3 shape (corner radius) scale. */
typedef enum {
    TRLV_RADIUS_NONE,
    TRLV_RADIUS_XS,
    TRLV_RADIUS_SM,
    TRLV_RADIUS_MD,
    TRLV_RADIUS_LG,
    TRLV_RADIUS_XL,
    TRLV_RADIUS_FULL,       /**< Pill / circle; the template uses LV_RADIUS_CIRCLE */
    TRLV_RADIUS_COUNT
} trlv_radius_t;

/** Control size (e.g. button height). */
typedef enum {
    TRLV_SIZE_SM,
    TRLV_SIZE_MD,
    TRLV_SIZE_LG,
    TRLV_SIZE_COUNT
} trlv_size_t;

/** MD3 elevation levels 0-5. */
typedef enum {
    TRLV_ELEVATION_0,
    TRLV_ELEVATION_1,
    TRLV_ELEVATION_2,
    TRLV_ELEVATION_3,
    TRLV_ELEVATION_4,
    TRLV_ELEVATION_5,
    TRLV_ELEVATION_COUNT
} trlv_elevation_t;

/** MD3 type scale roles. */
typedef enum {
    TRLV_TEXT_DISPLAY_LARGE,
    TRLV_TEXT_DISPLAY_MEDIUM,
    TRLV_TEXT_DISPLAY_SMALL,
    TRLV_TEXT_HEADLINE_LARGE,
    TRLV_TEXT_HEADLINE_MEDIUM,
    TRLV_TEXT_HEADLINE_SMALL,
    TRLV_TEXT_TITLE_LARGE,
    TRLV_TEXT_TITLE_MEDIUM,
    TRLV_TEXT_TITLE_SMALL,
    TRLV_TEXT_BODY_LARGE,
    TRLV_TEXT_BODY_MEDIUM,
    TRLV_TEXT_BODY_SMALL,
    TRLV_TEXT_LABEL_LARGE,
    TRLV_TEXT_LABEL_MEDIUM,
    TRLV_TEXT_LABEL_SMALL,
    TRLV_TEXT_COUNT
} trlv_text_role_t;

/*---------------------------------------------------------------------------
 * Composite tokens
 *-------------------------------------------------------------------------*/

/** One MD3 text style (Figma text style). */
typedef struct {
    const lv_font_t * font;     /**< Font for this role (family + weight + size) */
    int32_t line_height;        /**< Figma / MD3 line height in px; converted to LVGL
                                     line spacing. 0 = the font's own line height. */
    int32_t letter_spacing;     /**< Letter spacing in px */
} trlv_text_style_t;

/** Drop shadow approximating one MD3 elevation level. The colour comes from
 *  trlv_color_tokens_t::shadow. */
typedef struct {
    int32_t blur;
    int32_t spread;
    int32_t offset_x;
    int32_t offset_y;
    lv_opa_t opa;
} trlv_shadow_t;

/** MD3 state-layer and disabled opacities. */
typedef struct {
    lv_opa_t focus;              /**< Focus state layer (MD3 10 %) */
    lv_opa_t pressed;            /**< Pressed state layer (MD3 10 %) */
    lv_opa_t disabled_content;   /**< Disabled content, on on_surface (MD3 38 %) */
    lv_opa_t disabled_container; /**< Disabled container, on on_surface (MD3 12 %) */
} trlv_state_tokens_t;

/*---------------------------------------------------------------------------
 * Collections (Figma variable collections)
 *-------------------------------------------------------------------------*/

/** Color collection: MD3 colour roles for one mode (light, dark, ...).
 *  Selected at runtime. Field names are MD3 role names in snake_case. */
typedef struct {
    lv_color_t primary;
    lv_color_t on_primary;
    lv_color_t primary_container;
    lv_color_t on_primary_container;

    lv_color_t secondary;
    lv_color_t on_secondary;
    lv_color_t secondary_container;
    lv_color_t on_secondary_container;

    lv_color_t tertiary;
    lv_color_t on_tertiary;
    lv_color_t tertiary_container;
    lv_color_t on_tertiary_container;

    lv_color_t error;
    lv_color_t on_error;
    lv_color_t error_container;
    lv_color_t on_error_container;

    lv_color_t surface;
    lv_color_t on_surface;
    lv_color_t on_surface_variant;
    lv_color_t surface_dim;
    lv_color_t surface_bright;
    lv_color_t surface_container_lowest;
    lv_color_t surface_container_low;
    lv_color_t surface_container;
    lv_color_t surface_container_high;
    lv_color_t surface_container_highest;

    lv_color_t inverse_surface;
    lv_color_t inverse_on_surface;
    lv_color_t inverse_primary;

    lv_color_t outline;
    lv_color_t outline_variant;
    lv_color_t scrim;
    lv_color_t shadow;

    /* Extensions, named like Material Theme Builder custom colours */
    lv_color_t success;
    lv_color_t on_success;
    lv_color_t success_container;
    lv_color_t on_success_container;

    lv_color_t warning;
    lv_color_t on_warning;
    lv_color_t warning_container;
    lv_color_t on_warning_container;

    lv_color_t focus;            /**< Focus ring; typically the same as secondary */

    trlv_state_tokens_t state;
} trlv_color_tokens_t;

/** Layout collection: one layout mode (compact, regular, ...). Selected at
 *  start-up. Arrays are indexed by the scale enums above. */
typedef struct {
    int32_t space[TRLV_SPACE_COUNT];
    int32_t radius[TRLV_RADIUS_COUNT];
    int32_t border_width;
    int32_t focus_width;
    int32_t control_height[TRLV_SIZE_COUNT];
    trlv_shadow_t elevation[TRLV_ELEVATION_COUNT];
    trlv_text_style_t text[TRLV_TEXT_COUNT];
} trlv_layout_tokens_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_TOKENS_H */
