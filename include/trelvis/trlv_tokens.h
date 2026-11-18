/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_tokens.h
 * MD3 design-token types: the Color and Layout collections and the scales
 * that index them.
 *
 * TreLVis defines only the types. A project supplies the values by copying
 * the token template (templates/trlv_tokens_template.c and .h) and passes
 * them to trlv_init. Read the values in use with trlv_get_color_tokens() and
 * trlv_get_layout_tokens().
 *
 * The collections mirror Figma variable collections:
 *
 * | Collection | Modes                    | Selected    | C type                 |
 * |------------|--------------------------|-------------|------------------------|
 * | Color      | light, dark, brand, ...  | at run time | trlv_color_tokens_t    |
 * | Layout     | compact, regular, ...    | at start-up | trlv_layout_tokens_t   |
 *
 * Mapping: Figma variable / MD3 token → CSS → TreLVis → where TreLVis uses it
 *
 * | Figma / MD3                          | CSS                          | TreLVis                               | Used by (v1)                                |
 * |--------------------------------------|------------------------------|---------------------------------------|---------------------------------------------|
 * | `space/md`                           | `--space-md`                 | TRLV_SPACE_MD, `space[]`              | trlv_set_gap, trlv_set_pad, row/column gap          |
 * | `md.sys.shape.corner.medium`         | `border-radius`              | TRLV_RADIUS_MD, `radius[]`            | cards (MD), buttons (FULL), fields, checkbox and menu (XS) |
 * | Stroke width                         | `border-width`               | `border_width`                        | outlined card, outlined button, fields      |
 * | Focus indicator thickness            | `outline-width`              | `focus_width`                         | theme focus ring                            |
 * | Control height                       | `height`                     | TRLV_SIZE_MD, `control_height[]`      | theme button (MD), switch track (SM)        |
 * | `md.sys.elevation.level1`            | `box-shadow`                 | TRLV_ELEVATION_1, `elevation[]`       | elevated card (level 1)                     |
 * | `md.sys.typescale.body-medium`       | `font`, `line-height`, ...   | TRLV_TEXT_BODY_MEDIUM, `text[]`       | trlv_text_create, trlv_apply_text_role, theme            |
 * | `md.sys.color.surface-container-high`| `--md-sys-color-*`           | `surface_container_high`              | theme, surfaces, cards                      |
 * | `md.sys.state.pressed.state-layer-opacity` | `opacity`              | `state.pressed`                       | theme state styles                          |
 * | Material Theme Builder custom colour | `--md-extended-color-*`      | trlv_custom_color_t, `custom[]`       | trlv_apply_surface_custom                         |
 *
 * Tokens that no TreLVis function reads (for example TRLV_SIZE_LG,
 * TRLV_ELEVATION_3 or `surface_dim`) are there for a project's own
 * components, so screens hold no magic numbers.
 *
 * The structs hold the standard MD3 roles and scales, plus what v1 reads.
 * Their shape never depends on a configuration option. Colours beyond the
 * MD3 roles (status, alarm priorities, extra brand accents) are the
 * project's own, in the custom colour table. Tokens for later features
 * (icon sizes, hover, motion) are added with the feature that uses them.
 * They are plain structs so that a future generator (DTCG or Material Theme
 * Builder JSON) can emit them unchanged.
 */

#ifndef TRLV_TOKENS_H
#define TRLV_TOKENS_H

#include "trlv_version.h"

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------
 * Scales. Each value indexes an array in trlv_layout_tokens_t; the names
 * mirror Figma variable names and the MD3 scales.
 *-------------------------------------------------------------------------*/

/**
 * Spacing scale: padding and gaps. Figma variables `space/none` ...
 * `space/3xl`. The template uses a 4 px grid with a 2 px half-step
 * (Regular: 0, 2, 4, 8, 12, 16, 24, 32, 48).
 */
typedef enum {
    TRLV_SPACE_NONE,    /**< `space/none`: 0 */
    TRLV_SPACE_2XS,     /**< `space/2xs`: hairline gaps */
    TRLV_SPACE_XS,      /**< `space/xs` */
    TRLV_SPACE_SM,      /**< `space/sm`: typical gap */
    TRLV_SPACE_MD,      /**< `space/md`: typical gap */
    TRLV_SPACE_LG,      /**< `space/lg`: typical card padding */
    TRLV_SPACE_XL,      /**< `space/xl`: section spacing */
    TRLV_SPACE_2XL,     /**< `space/2xl`: section spacing */
    TRLV_SPACE_3XL,     /**< `space/3xl` */
    TRLV_SPACE_COUNT    /**< Number of steps; not a step */
} trlv_space_t;

/**
 * MD3 shape scale: corner radius. MD3 tokens `md.sys.shape.corner.none`
 * ... `full`, Figma variables `radius/none` ... `radius/full`.
 */
typedef enum {
    TRLV_RADIUS_NONE,   /**< Corner None: square */
    TRLV_RADIUS_XS,     /**< Corner Extra small: text fields */
    TRLV_RADIUS_SM,     /**< Corner Small */
    TRLV_RADIUS_MD,     /**< Corner Medium: cards */
    TRLV_RADIUS_LG,     /**< Corner Large */
    TRLV_RADIUS_XL,     /**< Corner Extra large: dialogs, sheets */
    TRLV_RADIUS_FULL,   /**< Corner Full: pills and circles (buttons).
                             Set it to LV_RADIUS_CIRCLE. */
    TRLV_RADIUS_COUNT   /**< Number of steps; not a step */
} trlv_radius_t;

/** Control size: the height of buttons and other controls. Figma variables
 *  `size/sm` ... `size/lg`. */
typedef enum {
    TRLV_SIZE_SM,       /**< Small controls */
    TRLV_SIZE_MD,       /**< Default: the theme's button height */
    TRLV_SIZE_LG,       /**< Large controls */
    TRLV_SIZE_COUNT     /**< Number of sizes; not a size */
} trlv_size_t;

/** MD3 elevation levels 0-5 (`md.sys.elevation.level0` ... `level5`). */
typedef enum {
    TRLV_ELEVATION_0,   /**< Level 0: flat */
    TRLV_ELEVATION_1,   /**< Level 1: elevated card */
    TRLV_ELEVATION_2,   /**< Level 2 */
    TRLV_ELEVATION_3,   /**< Level 3: dialogs, menus */
    TRLV_ELEVATION_4,   /**< Level 4 */
    TRLV_ELEVATION_5,   /**< Level 5 */
    TRLV_ELEVATION_COUNT /**< Number of levels; not a level */
} trlv_elevation_t;

/**
 * MD3 type scale roles (`md.sys.typescale.*`), the Figma text styles
 * `Display/Large` ... `Label/Small`. Each role has a trlv_text_style_t in
 * trlv_layout_tokens_t::text.
 */
typedef enum {
    TRLV_TEXT_DISPLAY_LARGE,    /**< Display Large (MD3 57 / 64) */
    TRLV_TEXT_DISPLAY_MEDIUM,   /**< Display Medium (MD3 45 / 52) */
    TRLV_TEXT_DISPLAY_SMALL,    /**< Display Small (MD3 36 / 44) */
    TRLV_TEXT_HEADLINE_LARGE,   /**< Headline Large (MD3 32 / 40) */
    TRLV_TEXT_HEADLINE_MEDIUM,  /**< Headline Medium (MD3 28 / 36) */
    TRLV_TEXT_HEADLINE_SMALL,   /**< Headline Small (MD3 24 / 32) */
    TRLV_TEXT_TITLE_LARGE,      /**< Title Large (MD3 22 / 28) */
    TRLV_TEXT_TITLE_MEDIUM,     /**< Title Medium (MD3 16 / 24) */
    TRLV_TEXT_TITLE_SMALL,      /**< Title Small (MD3 14 / 20) */
    TRLV_TEXT_BODY_LARGE,       /**< Body Large (MD3 16 / 24) */
    TRLV_TEXT_BODY_MEDIUM,      /**< Body Medium (MD3 14 / 20): the default for screens and labels */
    TRLV_TEXT_BODY_SMALL,       /**< Body Small (MD3 12 / 16) */
    TRLV_TEXT_LABEL_LARGE,      /**< Label Large (MD3 14 / 20): button labels */
    TRLV_TEXT_LABEL_MEDIUM,     /**< Label Medium (MD3 12 / 16) */
    TRLV_TEXT_LABEL_SMALL,      /**< Label Small (MD3 11 / 16) */
    TRLV_TEXT_COUNT             /**< Number of roles; not a role */
} trlv_text_role_t;

/*---------------------------------------------------------------------------
 * Composite tokens
 *-------------------------------------------------------------------------*/

/**
 * One MD3 text style: a Figma text style, CSS `font`, `line-height` and
 * `letter-spacing`. Several roles may share one font.
 */
typedef struct {
    const lv_font_t * font;     /**< Font for this role (family + weight + size),
                                     compiled or loaded at run time. NULL logs a
                                     warning and uses LV_FONT_DEFAULT. */
    int32_t line_height;        /**< Figma / MD3 line height in px. LVGL has line
                                     spacing instead, so TreLVis sets
                                     `text_line_space = line_height -
                                     lv_font_get_line_height(font)`.
                                     0 = the font's own line height. */
    int32_t letter_spacing;     /**< Letter spacing in px (CSS `letter-spacing`).
                                     Figma gives it in %: px = % x size / 100. */
} trlv_text_style_t;

/**
 * Drop shadow approximating one MD3 elevation level: a Figma drop shadow,
 * CSS `box-shadow`. MD3 uses two shadow layers; LVGL draws one, so the
 * template approximates them. The colour comes from
 * trlv_color_tokens_t::shadow. Set every level to zero for tone-only
 * elevation (recommended on MCUs: shadows cost draw time).
 */
typedef struct {
    int32_t blur;               /**< Blur radius in px (LVGL `shadow_width`) */
    int32_t spread;             /**< Spread in px (LVGL `shadow_spread`) */
    int32_t offset_x;           /**< Horizontal offset in px (LVGL `shadow_offset_x`) */
    int32_t offset_y;           /**< Vertical offset in px (LVGL `shadow_offset_y`) */
    lv_opa_t opa;               /**< Opacity (LVGL `shadow_opa`); LV_OPA_0 = no shadow */
} trlv_shadow_t;

/**
 * MD3 state-layer and disabled opacities (`md.sys.state.*`). The theme
 * mixes them into pre-computed colours: a state layer is the content
 * colour over the container at this opacity.
 */
typedef struct {
    lv_opa_t focus;              /**< Focus state layer (MD3 10 %) */
    lv_opa_t pressed;            /**< Pressed state layer (MD3 10 %) */
    lv_opa_t disabled_content;   /**< Disabled content: on_surface at this
                                      opacity (MD3 38 %) */
    lv_opa_t disabled_container; /**< Disabled container: on_surface at this
                                      opacity (MD3 12 %) */
} trlv_state_tokens_t;

/**
 * One custom colour: a colour family the project defines beyond the MD3
 * roles. Material Theme Builder calls these custom (extended) colours and
 * exports the same four values per colour mode: `color`, `onColor`,
 * `colorContainer`, `onColorContainer`.
 *
 * TreLVis gives the entries no meaning; the project names them with its own
 * enum and uses that as the index into trlv_color_tokens_t::custom:
 *
 * - a consumer product: success, warning, an extra brand accent;
 * - a process HMI (ISA-101, ISA-18.2): one entry per alarm priority of the
 *   site's alarm philosophy;
 * - a medical device (IEC 60601-1-8): high, medium and low priority alarms.
 *
 * For alarm colours, take the values from the applicable standard or alarm
 * philosophy as they are: don't harmonise them towards the brand colour in
 * Material Theme Builder, and don't use them for anything but alarms.
 */
typedef struct {
    lv_color_t color;               /**< The colour itself: strong fills, icons, text */
    lv_color_t on_color;            /**< Content on color */
    lv_color_t color_container;     /**< Lower-emphasis container */
    lv_color_t on_color_container;  /**< Content on color_container */
} trlv_custom_color_t;

/*---------------------------------------------------------------------------
 * Collections (Figma variable collections)
 *-------------------------------------------------------------------------*/

/**
 * Color collection: the MD3 colour roles for one colour mode (light, dark,
 * a brand scheme, ...). Selected at run time. Field names are the MD3 role
 * names in snake_case: *Surface Container High* (`md.sys.color.
 * surface-container-high`) is `surface_container_high`.
 *
 * Every container role has an `on_` role for content drawn on it. Use them
 * in pairs; the theme, trlv_apply_surface and trlv_apply_card do so through text-colour
 * inheritance.
 */
typedef struct {
    lv_color_t primary;                 /**< Key colour: filled buttons, active states */
    lv_color_t on_primary;              /**< Content on primary */
    lv_color_t primary_container;       /**< Prominent containers */
    lv_color_t on_primary_container;    /**< Content on primary_container */

    lv_color_t secondary;               /**< Less prominent accents */
    lv_color_t on_secondary;            /**< Content on secondary */
    lv_color_t secondary_container;     /**< Tonal buttons, chips */
    lv_color_t on_secondary_container;  /**< Content on secondary_container */

    lv_color_t tertiary;                /**< Contrasting accents */
    lv_color_t on_tertiary;             /**< Content on tertiary */
    lv_color_t tertiary_container;      /**< Contrasting containers */
    lv_color_t on_tertiary_container;   /**< Content on tertiary_container */

    lv_color_t error;                   /**< Errors */
    lv_color_t on_error;                /**< Content on error */
    lv_color_t error_container;         /**< Error containers */
    lv_color_t on_error_container;      /**< Content on error_container */

    lv_color_t surface;                 /**< Default background: screens */
    lv_color_t on_surface;              /**< Default content colour */
    lv_color_t on_surface_variant;      /**< Lower-emphasis content */
    lv_color_t surface_dim;             /**< Dimmest surface */
    lv_color_t surface_bright;          /**< Brightest surface */
    lv_color_t surface_container_lowest;  /**< Surface container ladder, lowest */
    lv_color_t surface_container_low;     /**< Elevated card */
    lv_color_t surface_container;         /**< Menus, navigation */
    lv_color_t surface_container_high;    /**< Higher emphasis */
    lv_color_t surface_container_highest; /**< Filled card, slider track */

    lv_color_t inverse_surface;         /**< Snackbars, tooltips */
    lv_color_t inverse_on_surface;      /**< Content on inverse_surface */
    lv_color_t inverse_primary;         /**< Actions on inverse_surface */

    lv_color_t outline;                 /**< Borders: outlined buttons and fields */
    lv_color_t outline_variant;         /**< Decorative borders: dividers, outlined cards */
    lv_color_t scrim;                   /**< Modal backdrop */
    lv_color_t shadow;                  /**< Elevation shadow colour */

    lv_color_t focus;                   /**< Extension: keypad / encoder focus ring.
                                             MD3 uses secondary; kept separate so a
                                             product can make focus more visible. */

    trlv_state_tokens_t state;          /**< State-layer and disabled opacities */

    const trlv_custom_color_t * custom; /**< The project's custom colours for this
                                             colour mode, indexed by the project's
                                             own enum; NULL if it has none. The
                                             array must outlive the tokens' use.
                                             Give every colour mode the same
                                             entries in the same order. */
    uint32_t custom_count;              /**< Number of entries in custom. Entries
                                             beyond TRLV_CUSTOM_COLOR_MAX can be
                                             read with trlv_get_color_tokens() but not used
                                             with trlv_apply_surface_custom. */
} trlv_color_tokens_t;

/**
 * Layout collection: one layout mode (compact, regular, ...). Selected once,
 * at start-up. The arrays are indexed by the scale enums above.
 */
typedef struct {
    int32_t space[TRLV_SPACE_COUNT];               /**< Spacing scale in px */
    int32_t radius[TRLV_RADIUS_COUNT];             /**< Shape scale in px;
                                                        FULL = LV_RADIUS_CIRCLE */
    int32_t border_width;                          /**< Stroke width in px */
    int32_t focus_width;                           /**< Focus ring width in px */
    int32_t control_height[TRLV_SIZE_COUNT];       /**< Control heights in px.
                                                        Touch targets should be
                                                        44-48 px including
                                                        surrounding space. */
    trlv_shadow_t elevation[TRLV_ELEVATION_COUNT]; /**< Shadow per elevation level */
    trlv_text_style_t text[TRLV_TEXT_COUNT];       /**< Text style per type-scale role */
} trlv_layout_tokens_t;

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_TOKENS_H */
