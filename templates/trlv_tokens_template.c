/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_tokens_template.c
 * Token template: token values a project copies and owns. See
 * trlv_tokens_template.h for how to set it up.
 *
 * Colour: the Material Design 3 baseline schemes (seed #6750A4), as
 * published by Google's Material Design (m3.material.io). The custom colour
 * table holds four example status colours: fault, warning, advisory and bad
 * data. Their values are examples only and claim compliance with no
 * standard: take yours from your product's standard or alarm philosophy, or
 * replace the entries with your own (alarm priorities, brand accents), or
 * delete them. See docs/building-a-token-set.md.
 *
 * Layout: the Regular and Compact values from docs/concepts.md section 4.2.
 * Pass one of them to trlv_init; the layout mode is chosen once, at
 * start-up. In Regular, each text role uses the Montserrat size LVGL ships
 * nearest to its MD3 size, with the MD3 line height; Compact uses smaller
 * sizes and tighter lines. A size not enabled in lv_conf.h
 * (LV_FONT_MONTSERRAT_<size>) falls back to the next smaller enabled size,
 * else LV_FONT_DEFAULT. The template needs no font files.
 *
 * The tokens are const, so they live in flash. If your fonts only exist at
 * run time (FreeType, Tiny TTF), keep the layout tokens in RAM instead and
 * fill in the fonts before calling trlv_init.
 */

#include "trlv_tokens_template.h"   /* change to your copy's name */

/* A Figma / Material Theme Builder hex colour, e.g. TOKEN_HEX(0x6750A4). */
#define TOKEN_HEX(hex) LV_COLOR_MAKE(((hex) >> 16) & 0xFF, ((hex) >> 8) & 0xFF, (hex) & 0xFF)

/* MD3 state-layer and disabled opacities, shared by both colour modes. */
#define TOKEN_STATE_MD3                             \
    {                                               \
        .focus              = LV_OPA_10,            \
        .pressed            = LV_OPA_10,            \
        .disabled_content   = 97,   /* 38 % */      \
        .disabled_container = 31,   /* 12 % */      \
    }

/*---------------------------------------------------------------------------
 * Color collection
 *-------------------------------------------------------------------------*/

/* Custom colours: one entry per value of token_custom_color_t, in the same
 * order in every colour mode. Example status colours: the strong colour
 * (color / on_color: a banner, a badge) is the same in both modes, so a
 * status looks the same everywhere it appears; the calmer container
 * (color_container / on_color_container) follows the mode. None of them is
 * `error`, `primary` or `focus`, so a fault never reads as an input error
 * or a selection. Example values: take yours from your standard or alarm
 * philosophy. */
#define STATUS_FAULT_COLOR      TOKEN_HEX(0xC62828)
#define STATUS_FAULT_ON         TOKEN_HEX(0xFFFFFF)
#define STATUS_WARNING_COLOR    TOKEN_HEX(0xF9A825)
#define STATUS_WARNING_ON       TOKEN_HEX(0x000000)
#define STATUS_ADVISORY_COLOR   TOKEN_HEX(0x1565C0)
#define STATUS_ADVISORY_ON      TOKEN_HEX(0xFFFFFF)
#define STATUS_BAD_DATA_COLOR   TOKEN_HEX(0xA3006E)
#define STATUS_BAD_DATA_ON      TOKEN_HEX(0xFFFFFF)

static const trlv_custom_color_t custom_light[TOKEN_CUSTOM_COUNT] = {
    [TOKEN_CUSTOM_FAULT] = {
        .color              = STATUS_FAULT_COLOR,
        .on_color           = STATUS_FAULT_ON,
        .color_container    = TOKEN_HEX(0xFFDAD5),
        .on_color_container = TOKEN_HEX(0x410002),
    },
    [TOKEN_CUSTOM_WARNING] = {
        .color              = STATUS_WARNING_COLOR,
        .on_color           = STATUS_WARNING_ON,
        .color_container    = TOKEN_HEX(0xFFEFC2),
        .on_color_container = TOKEN_HEX(0x3A2A00),
    },
    [TOKEN_CUSTOM_ADVISORY] = {
        .color              = STATUS_ADVISORY_COLOR,
        .on_color           = STATUS_ADVISORY_ON,
        .color_container    = TOKEN_HEX(0xD6E3FF),
        .on_color_container = TOKEN_HEX(0x001B3E),
    },
    [TOKEN_CUSTOM_BAD_DATA] = {
        .color              = STATUS_BAD_DATA_COLOR,
        .on_color           = STATUS_BAD_DATA_ON,
        .color_container    = TOKEN_HEX(0xFFD8EC),
        .on_color_container = TOKEN_HEX(0x3E0026),
    },
};

static const trlv_custom_color_t custom_dark[TOKEN_CUSTOM_COUNT] = {
    [TOKEN_CUSTOM_FAULT] = {
        .color              = STATUS_FAULT_COLOR,
        .on_color           = STATUS_FAULT_ON,
        .color_container    = TOKEN_HEX(0x5C1410),
        .on_color_container = TOKEN_HEX(0xFFDAD5),
    },
    [TOKEN_CUSTOM_WARNING] = {
        .color              = STATUS_WARNING_COLOR,
        .on_color           = STATUS_WARNING_ON,
        .color_container    = TOKEN_HEX(0x4D3A00),
        .on_color_container = TOKEN_HEX(0xFFE08A),
    },
    [TOKEN_CUSTOM_ADVISORY] = {
        .color              = STATUS_ADVISORY_COLOR,
        .on_color           = STATUS_ADVISORY_ON,
        .color_container    = TOKEN_HEX(0x0F3B6E),
        .on_color_container = TOKEN_HEX(0xD6E3FF),
    },
    [TOKEN_CUSTOM_BAD_DATA] = {
        .color              = STATUS_BAD_DATA_COLOR,
        .on_color           = STATUS_BAD_DATA_ON,
        .color_container    = TOKEN_HEX(0x5E0043),
        .on_color_container = TOKEN_HEX(0xFFD8EC),
    },
};

const trlv_color_tokens_t tokens_light = {
    .primary                   = TOKEN_HEX(0x6750A4),
    .on_primary                = TOKEN_HEX(0xFFFFFF),
    .primary_container         = TOKEN_HEX(0xEADDFF),
    .on_primary_container      = TOKEN_HEX(0x21005D),

    .secondary                 = TOKEN_HEX(0x625B71),
    .on_secondary              = TOKEN_HEX(0xFFFFFF),
    .secondary_container       = TOKEN_HEX(0xE8DEF8),
    .on_secondary_container    = TOKEN_HEX(0x1D192B),

    .tertiary                  = TOKEN_HEX(0x7D5260),
    .on_tertiary               = TOKEN_HEX(0xFFFFFF),
    .tertiary_container        = TOKEN_HEX(0xFFD8E4),
    .on_tertiary_container     = TOKEN_HEX(0x31111D),

    .error                     = TOKEN_HEX(0xB3261E),
    .on_error                  = TOKEN_HEX(0xFFFFFF),
    .error_container           = TOKEN_HEX(0xF9DEDC),
    .on_error_container        = TOKEN_HEX(0x410E0B),

    .surface                   = TOKEN_HEX(0xFEF7FF),
    .on_surface                = TOKEN_HEX(0x1D1B20),
    .on_surface_variant        = TOKEN_HEX(0x49454F),
    .surface_dim               = TOKEN_HEX(0xDED8E1),
    .surface_bright            = TOKEN_HEX(0xFEF7FF),
    .surface_container_lowest  = TOKEN_HEX(0xFFFFFF),
    .surface_container_low     = TOKEN_HEX(0xF7F2FA),
    .surface_container         = TOKEN_HEX(0xF3EDF7),
    .surface_container_high    = TOKEN_HEX(0xECE6F0),
    .surface_container_highest = TOKEN_HEX(0xE6E0E9),

    .inverse_surface           = TOKEN_HEX(0x322F35),
    .inverse_on_surface        = TOKEN_HEX(0xF5EFF7),
    .inverse_primary           = TOKEN_HEX(0xD0BCFF),

    .outline                   = TOKEN_HEX(0x79747E),
    .outline_variant           = TOKEN_HEX(0xCAC4D0),
    .scrim                     = TOKEN_HEX(0x000000),
    .shadow                    = TOKEN_HEX(0x000000),

    .focus                     = TOKEN_HEX(0x625B71),   /* = secondary */

    .state                     = TOKEN_STATE_MD3,

    .custom                    = custom_light,
    .custom_count              = TOKEN_CUSTOM_COUNT,
};

const trlv_color_tokens_t tokens_dark = {
    .primary                   = TOKEN_HEX(0xD0BCFF),
    .on_primary                = TOKEN_HEX(0x381E72),
    .primary_container         = TOKEN_HEX(0x4F378B),
    .on_primary_container      = TOKEN_HEX(0xEADDFF),

    .secondary                 = TOKEN_HEX(0xCCC2DC),
    .on_secondary              = TOKEN_HEX(0x332D41),
    .secondary_container       = TOKEN_HEX(0x4A4458),
    .on_secondary_container    = TOKEN_HEX(0xE8DEF8),

    .tertiary                  = TOKEN_HEX(0xEFB8C8),
    .on_tertiary               = TOKEN_HEX(0x492532),
    .tertiary_container        = TOKEN_HEX(0x633B48),
    .on_tertiary_container     = TOKEN_HEX(0xFFD8E4),

    .error                     = TOKEN_HEX(0xF2B8B5),
    .on_error                  = TOKEN_HEX(0x601410),
    .error_container           = TOKEN_HEX(0x8C1D18),
    .on_error_container        = TOKEN_HEX(0xF9DEDC),

    .surface                   = TOKEN_HEX(0x141218),
    .on_surface                = TOKEN_HEX(0xE6E0E9),
    .on_surface_variant        = TOKEN_HEX(0xCAC4D0),
    .surface_dim               = TOKEN_HEX(0x141218),
    .surface_bright            = TOKEN_HEX(0x3B383E),
    .surface_container_lowest  = TOKEN_HEX(0x0F0D13),
    .surface_container_low     = TOKEN_HEX(0x1D1B20),
    .surface_container         = TOKEN_HEX(0x211F26),
    .surface_container_high    = TOKEN_HEX(0x2B2930),
    .surface_container_highest = TOKEN_HEX(0x36343B),

    .inverse_surface           = TOKEN_HEX(0xE6E0E9),
    .inverse_on_surface        = TOKEN_HEX(0x322F35),
    .inverse_primary           = TOKEN_HEX(0x6750A4),

    .outline                   = TOKEN_HEX(0x938F99),
    .outline_variant           = TOKEN_HEX(0x49454F),
    .scrim                     = TOKEN_HEX(0x000000),
    .shadow                    = TOKEN_HEX(0x000000),

    .focus                     = TOKEN_HEX(0xCCC2DC),   /* = secondary */

    .state                     = TOKEN_STATE_MD3,

    .custom                    = custom_dark,
    .custom_count              = TOKEN_CUSTOM_COUNT,
};

/*---------------------------------------------------------------------------
 * Layout collection
 *-------------------------------------------------------------------------*/

/* A text role with no design values yet: the default font, its own line
 * height and no letter spacing. */
#define TOKEN_TEXT_DEFAULT { .font = LV_FONT_DEFAULT, .line_height = 0, .letter_spacing = 0 }

/* A text role in an LVGL Montserrat size with its line height and no letter
 * spacing (a field left out of a designated initialiser is 0), if that size
 * is enabled in lv_conf.h. If it isn't, the next smaller enabled size is
 * used with its own line height, so the type hierarchy holds with any set of
 * sizes; with none enabled, TOKEN_TEXT_DEFAULT. Montserrat ships in one
 * weight, so this gives size hierarchy only. Once your roles have real
 * fonts, replace these with
 * { .font = &my_font, .line_height = 24, .letter_spacing = 0 } and
 * delete this block. */
#if LV_FONT_MONTSERRAT_12
#  define TOKEN_TEXT_12(line_px) { .font = &lv_font_montserrat_12, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_12(line_px) TOKEN_TEXT_DEFAULT
#endif
#if LV_FONT_MONTSERRAT_14
#  define TOKEN_TEXT_14(line_px) { .font = &lv_font_montserrat_14, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_14(line_px) TOKEN_TEXT_12(0)
#endif
#if LV_FONT_MONTSERRAT_16
#  define TOKEN_TEXT_16(line_px) { .font = &lv_font_montserrat_16, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_16(line_px) TOKEN_TEXT_14(0)
#endif
#if LV_FONT_MONTSERRAT_22
#  define TOKEN_TEXT_22(line_px) { .font = &lv_font_montserrat_22, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_22(line_px) TOKEN_TEXT_16(0)
#endif
#if LV_FONT_MONTSERRAT_24
#  define TOKEN_TEXT_24(line_px) { .font = &lv_font_montserrat_24, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_24(line_px) TOKEN_TEXT_22(0)
#endif
#if LV_FONT_MONTSERRAT_28
#  define TOKEN_TEXT_28(line_px) { .font = &lv_font_montserrat_28, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_28(line_px) TOKEN_TEXT_24(0)
#endif
#if LV_FONT_MONTSERRAT_32
#  define TOKEN_TEXT_32(line_px) { .font = &lv_font_montserrat_32, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_32(line_px) TOKEN_TEXT_28(0)
#endif
#if LV_FONT_MONTSERRAT_36
#  define TOKEN_TEXT_36(line_px) { .font = &lv_font_montserrat_36, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_36(line_px) TOKEN_TEXT_32(0)
#endif
#if LV_FONT_MONTSERRAT_44
#  define TOKEN_TEXT_44(line_px) { .font = &lv_font_montserrat_44, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_44(line_px) TOKEN_TEXT_36(0)
#endif
#if LV_FONT_MONTSERRAT_48
#  define TOKEN_TEXT_48(line_px) { .font = &lv_font_montserrat_48, .line_height = (line_px) }
#else
#  define TOKEN_TEXT_48(line_px) TOKEN_TEXT_44(0)
#endif

const trlv_layout_tokens_t tokens_regular = {
    .space = {
        [TRLV_SPACE_NONE] = 0,
        [TRLV_SPACE_2XS]  = 2,
        [TRLV_SPACE_XS]   = 4,
        [TRLV_SPACE_SM]   = 8,
        [TRLV_SPACE_MD]   = 12,
        [TRLV_SPACE_LG]   = 16,
        [TRLV_SPACE_XL]   = 24,
        [TRLV_SPACE_2XL]  = 32,
        [TRLV_SPACE_3XL]  = 48,
    },

    .radius = {
        [TRLV_RADIUS_NONE] = 0,
        [TRLV_RADIUS_XS]   = 4,
        [TRLV_RADIUS_SM]   = 8,
        [TRLV_RADIUS_MD]   = 12,
        [TRLV_RADIUS_LG]   = 16,
        [TRLV_RADIUS_XL]   = 28,
        [TRLV_RADIUS_FULL] = LV_RADIUS_CIRCLE,
    },

    .border_width = 1,
    .focus_width  = 3,

    .control_height = {
        [TRLV_SIZE_SM] = 32,
        [TRLV_SIZE_MD] = 40,
        [TRLV_SIZE_LG] = 56,
    },

    /* One LVGL shadow per level, approximating MD3's two-layer elevation
     * shadows (the ambient layer's geometry). Colour: tokens.shadow. */
    .elevation = {
        [TRLV_ELEVATION_0] = { .blur =  0, .spread = 0, .offset_x = 0, .offset_y = 0, .opa = LV_OPA_0  },
        [TRLV_ELEVATION_1] = { .blur =  3, .spread = 1, .offset_x = 0, .offset_y = 1, .opa = LV_OPA_20 },
        [TRLV_ELEVATION_2] = { .blur =  6, .spread = 2, .offset_x = 0, .offset_y = 2, .opa = LV_OPA_20 },
        [TRLV_ELEVATION_3] = { .blur =  8, .spread = 3, .offset_x = 0, .offset_y = 4, .opa = LV_OPA_20 },
        [TRLV_ELEVATION_4] = { .blur = 10, .spread = 4, .offset_x = 0, .offset_y = 6, .opa = LV_OPA_20 },
        [TRLV_ELEVATION_5] = { .blur = 12, .spread = 6, .offset_x = 0, .offset_y = 8, .opa = LV_OPA_20 },
    },

    /* MD3 type scale: MD3 size / line height in px noted for each role.
     * Montserrat has no 57, 45 or 11 px size, so the nearest is used. */
    .text = {
        [TRLV_TEXT_DISPLAY_LARGE]   = TOKEN_TEXT_48(64),   /* 57 / 64 */
        [TRLV_TEXT_DISPLAY_MEDIUM]  = TOKEN_TEXT_44(52),   /* 45 / 52 */
        [TRLV_TEXT_DISPLAY_SMALL]   = TOKEN_TEXT_36(44),   /* 36 / 44 */
        [TRLV_TEXT_HEADLINE_LARGE]  = TOKEN_TEXT_32(40),   /* 32 / 40 */
        [TRLV_TEXT_HEADLINE_MEDIUM] = TOKEN_TEXT_28(36),   /* 28 / 36 */
        [TRLV_TEXT_HEADLINE_SMALL]  = TOKEN_TEXT_24(32),   /* 24 / 32 */
        [TRLV_TEXT_TITLE_LARGE]     = TOKEN_TEXT_22(28),   /* 22 / 28 */
        [TRLV_TEXT_TITLE_MEDIUM]    = TOKEN_TEXT_16(24),   /* 16 / 24 */
        [TRLV_TEXT_TITLE_SMALL]     = TOKEN_TEXT_14(20),   /* 14 / 20 */
        [TRLV_TEXT_BODY_LARGE]      = TOKEN_TEXT_16(24),   /* 16 / 24 */
        [TRLV_TEXT_BODY_MEDIUM]     = TOKEN_TEXT_14(20),   /* 14 / 20 */
        [TRLV_TEXT_BODY_SMALL]      = TOKEN_TEXT_12(16),   /* 12 / 16 */
        [TRLV_TEXT_LABEL_LARGE]     = TOKEN_TEXT_14(20),   /* 14 / 20 */
        [TRLV_TEXT_LABEL_MEDIUM]    = TOKEN_TEXT_12(16),   /* 12 / 16 */
        [TRLV_TEXT_LABEL_SMALL]     = TOKEN_TEXT_12(16),   /* 11 / 16 */
    },
};

/* Compact: denser spacing, smaller corners and controls, no shadows (tone
 * only, cheaper to draw on an MCU), the values from docs/concepts.md
 * section 4.2. The display and headline roles step down one size; the
 * others keep their size on tighter lines. A real product's compact type
 * scale comes from its own design.
 *
 * The focus ring (focus_width plus space[2XS], 4 px) still fits in
 * space[SM] (4 px), so containers padded TRLV_SPACE_SM keep it visible in
 * both layouts. Delete this collection if you don't need it. */
const trlv_layout_tokens_t tokens_compact = {
    .space = {
        [TRLV_SPACE_NONE] = 0,
        [TRLV_SPACE_2XS]  = 2,
        [TRLV_SPACE_XS]   = 2,
        [TRLV_SPACE_SM]   = 4,
        [TRLV_SPACE_MD]   = 8,
        [TRLV_SPACE_LG]   = 12,
        [TRLV_SPACE_XL]   = 16,
        [TRLV_SPACE_2XL]  = 24,
        [TRLV_SPACE_3XL]  = 32,
    },

    .radius = {
        [TRLV_RADIUS_NONE] = 0,
        [TRLV_RADIUS_XS]   = 2,
        [TRLV_RADIUS_SM]   = 4,
        [TRLV_RADIUS_MD]   = 8,
        [TRLV_RADIUS_LG]   = 12,
        [TRLV_RADIUS_XL]   = 20,
        [TRLV_RADIUS_FULL] = LV_RADIUS_CIRCLE,
    },

    .border_width = 1,
    .focus_width  = 2,

    .control_height = {
        [TRLV_SIZE_SM] = 28,
        [TRLV_SIZE_MD] = 36,
        [TRLV_SIZE_LG] = 44,
    },

    /* Tone-only elevation: every level zero (no blur, spread or offset). */
    .elevation = {
        [TRLV_ELEVATION_0] = { .opa = LV_OPA_0 },
        [TRLV_ELEVATION_1] = { .opa = LV_OPA_0 },
        [TRLV_ELEVATION_2] = { .opa = LV_OPA_0 },
        [TRLV_ELEVATION_3] = { .opa = LV_OPA_0 },
        [TRLV_ELEVATION_4] = { .opa = LV_OPA_0 },
        [TRLV_ELEVATION_5] = { .opa = LV_OPA_0 },
    },

    /* Size / line height in px for each role. */
    .text = {
        [TRLV_TEXT_DISPLAY_LARGE]   = TOKEN_TEXT_44(52),
        [TRLV_TEXT_DISPLAY_MEDIUM]  = TOKEN_TEXT_36(44),
        [TRLV_TEXT_DISPLAY_SMALL]   = TOKEN_TEXT_32(40),
        [TRLV_TEXT_HEADLINE_LARGE]  = TOKEN_TEXT_28(36),
        [TRLV_TEXT_HEADLINE_MEDIUM] = TOKEN_TEXT_24(32),
        [TRLV_TEXT_HEADLINE_SMALL]  = TOKEN_TEXT_22(28),
        [TRLV_TEXT_TITLE_LARGE]     = TOKEN_TEXT_22(26),
        [TRLV_TEXT_TITLE_MEDIUM]    = TOKEN_TEXT_16(20),
        [TRLV_TEXT_TITLE_SMALL]     = TOKEN_TEXT_14(18),
        [TRLV_TEXT_BODY_LARGE]      = TOKEN_TEXT_16(20),
        [TRLV_TEXT_BODY_MEDIUM]     = TOKEN_TEXT_14(18),
        [TRLV_TEXT_BODY_SMALL]      = TOKEN_TEXT_12(14),
        [TRLV_TEXT_LABEL_LARGE]     = TOKEN_TEXT_14(18),
        [TRLV_TEXT_LABEL_MEDIUM]    = TOKEN_TEXT_12(14),
        [TRLV_TEXT_LABEL_SMALL]     = TOKEN_TEXT_12(14),
    },
};
