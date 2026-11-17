/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_tokens_hmi.c
 * Example colour tokens for a process or instrument HMI. See
 * trlv_tokens_hmi.h. Example values only; they claim compliance with no
 * standard.
 *
 * How the scheme is built:
 *
 * - Surfaces are neutral greys, with no tint, so that colour on the screen
 *   means something. The light scheme sits on a mid-light grey rather than
 *   white, to cut glare; the dark scheme on a dark grey rather than black.
 * - The accent roles (primary, secondary, tertiary) are low-saturation:
 *   primary is a muted steel blue, used for buttons and selected controls.
 * - The alarm priorities live in the custom colour table, not in MD3's
 *   roles. Their strong colours are the same in both modes and are not
 *   harmonised with primary: an alarm colour should look the same
 *   everywhere it appears.
 * - MD3 `error` (form validation, failed actions) is a muted brick red,
 *   clearly not the high-priority alarm red, so an input error never reads
 *   as an alarm.
 * - The focus ring is a strong blue, distinct from the alarm colours.
 */

#include "trlv_tokens_hmi.h"   /* change to your copy's name */

/* A hex colour, e.g. TOKEN_HEX(0x2F5D86). */
#define TOKEN_HEX(hex) LV_COLOR_MAKE(((hex) >> 16) & 0xFF, ((hex) >> 8) & 0xFF, (hex) & 0xFF)

/* MD3 state-layer and disabled opacities, as in the template. */
#define TOKEN_STATE_MD3                             \
    {                                               \
        .focus              = LV_OPA_10,            \
        .pressed            = LV_OPA_10,            \
        .disabled_content   = 97,   /* 38 % */      \
        .disabled_container = 31,   /* 12 % */      \
    }

/* Alarm priorities. The strong colour (an alarm banner, a priority badge)
 * and its content colour are the same in both modes; only the
 * lower-emphasis containers follow the mode. */
#define ALARM_HIGH_COLOR      TOKEN_HEX(0xE00000)
#define ALARM_HIGH_ON         TOKEN_HEX(0xFFFFFF)
#define ALARM_MEDIUM_COLOR    TOKEN_HEX(0xFFD400)
#define ALARM_MEDIUM_ON       TOKEN_HEX(0x000000)
#define ALARM_LOW_COLOR       TOKEN_HEX(0xFF8C00)
#define ALARM_LOW_ON          TOKEN_HEX(0x000000)

static const trlv_custom_color_t alarms_light[TOKEN_ALARM_COUNT] = {
    [TOKEN_ALARM_HIGH] = {
        .color              = ALARM_HIGH_COLOR,
        .on_color           = ALARM_HIGH_ON,
        .color_container    = TOKEN_HEX(0xF6CFCB),
        .on_color_container = TOKEN_HEX(0x4A0000),
    },
    [TOKEN_ALARM_MEDIUM] = {
        .color              = ALARM_MEDIUM_COLOR,
        .on_color           = ALARM_MEDIUM_ON,
        .color_container    = TOKEN_HEX(0xFBEDA8),
        .on_color_container = TOKEN_HEX(0x3A3000),
    },
    [TOKEN_ALARM_LOW] = {
        .color              = ALARM_LOW_COLOR,
        .on_color           = ALARM_LOW_ON,
        .color_container    = TOKEN_HEX(0xF9DCB8),
        .on_color_container = TOKEN_HEX(0x3D2000),
    },
};

static const trlv_custom_color_t alarms_dark[TOKEN_ALARM_COUNT] = {
    [TOKEN_ALARM_HIGH] = {
        .color              = ALARM_HIGH_COLOR,
        .on_color           = ALARM_HIGH_ON,
        .color_container    = TOKEN_HEX(0x5A1414),
        .on_color_container = TOKEN_HEX(0xF6CFCB),
    },
    [TOKEN_ALARM_MEDIUM] = {
        .color              = ALARM_MEDIUM_COLOR,
        .on_color           = ALARM_MEDIUM_ON,
        .color_container    = TOKEN_HEX(0x4A3E00),
        .on_color_container = TOKEN_HEX(0xFBEDA8),
    },
    [TOKEN_ALARM_LOW] = {
        .color              = ALARM_LOW_COLOR,
        .on_color           = ALARM_LOW_ON,
        .color_container    = TOKEN_HEX(0x5A3300),
        .on_color_container = TOKEN_HEX(0xF9DCB8),
    },
};

const trlv_color_tokens_t tokens_hmi_light = {
    .primary                   = TOKEN_HEX(0x2F5D86),
    .on_primary                = TOKEN_HEX(0xFFFFFF),
    .primary_container         = TOKEN_HEX(0xC9D8E6),
    .on_primary_container      = TOKEN_HEX(0x0F2A40),

    .secondary                 = TOKEN_HEX(0x56616B),
    .on_secondary              = TOKEN_HEX(0xFFFFFF),
    .secondary_container       = TOKEN_HEX(0xD0D6DC),
    .on_secondary_container    = TOKEN_HEX(0x1C252D),

    .tertiary                  = TOKEN_HEX(0x4F6152),
    .on_tertiary               = TOKEN_HEX(0xFFFFFF),
    .tertiary_container        = TOKEN_HEX(0xCFDAD0),
    .on_tertiary_container     = TOKEN_HEX(0x18241A),

    .error                     = TOKEN_HEX(0x7A2E2A),
    .on_error                  = TOKEN_HEX(0xFFFFFF),
    .error_container           = TOKEN_HEX(0xE6D0CE),
    .on_error_container        = TOKEN_HEX(0x3A0D0A),

    .surface                   = TOKEN_HEX(0xDCDCDC),
    .on_surface                = TOKEN_HEX(0x1C1C1C),
    .on_surface_variant        = TOKEN_HEX(0x474747),
    .surface_dim               = TOKEN_HEX(0xBABABA),
    .surface_bright            = TOKEN_HEX(0xDCDCDC),
    .surface_container_lowest  = TOKEN_HEX(0xE8E8E8),
    .surface_container_low     = TOKEN_HEX(0xD5D5D5),
    .surface_container         = TOKEN_HEX(0xCFCFCF),
    .surface_container_high    = TOKEN_HEX(0xC8C8C8),
    .surface_container_highest = TOKEN_HEX(0xC1C1C1),

    .inverse_surface           = TOKEN_HEX(0x2E2E2E),
    .inverse_on_surface        = TOKEN_HEX(0xEBEBEB),
    .inverse_primary           = TOKEN_HEX(0xA3C3E0),

    .outline                   = TOKEN_HEX(0x6E6E6E),
    .outline_variant           = TOKEN_HEX(0xAFAFAF),
    .scrim                     = TOKEN_HEX(0x000000),
    .shadow                    = TOKEN_HEX(0x000000),

    .focus                     = TOKEN_HEX(0x00478F),

    .state                     = TOKEN_STATE_MD3,

    .custom                    = alarms_light,
    .custom_count              = TOKEN_ALARM_COUNT,
};

const trlv_color_tokens_t tokens_hmi_dark = {
    .primary                   = TOKEN_HEX(0xA3C3E0),
    .on_primary                = TOKEN_HEX(0x0E2F4A),
    .primary_container         = TOKEN_HEX(0x2A4A66),
    .on_primary_container      = TOKEN_HEX(0xD2E3F3),

    .secondary                 = TOKEN_HEX(0xBBC5CE),
    .on_secondary              = TOKEN_HEX(0x27313A),
    .secondary_container       = TOKEN_HEX(0x3E4852),
    .on_secondary_container    = TOKEN_HEX(0xD7E1EA),

    .tertiary                  = TOKEN_HEX(0xB6C8B8),
    .on_tertiary               = TOKEN_HEX(0x233428),
    .tertiary_container        = TOKEN_HEX(0x3A4B3D),
    .on_tertiary_container     = TOKEN_HEX(0xD2E4D4),

    .error                     = TOKEN_HEX(0xE6B4AF),
    .on_error                  = TOKEN_HEX(0x4A1512),
    .error_container           = TOKEN_HEX(0x6A2925),
    .on_error_container        = TOKEN_HEX(0xF5DAD7),

    .surface                   = TOKEN_HEX(0x262626),
    .on_surface                = TOKEN_HEX(0xDADADA),
    .on_surface_variant        = TOKEN_HEX(0xB5B5B5),
    .surface_dim               = TOKEN_HEX(0x262626),
    .surface_bright            = TOKEN_HEX(0x474747),
    .surface_container_lowest  = TOKEN_HEX(0x1E1E1E),
    .surface_container_low     = TOKEN_HEX(0x2C2C2C),
    .surface_container         = TOKEN_HEX(0x303030),
    .surface_container_high    = TOKEN_HEX(0x383838),
    .surface_container_highest = TOKEN_HEX(0x414141),

    .inverse_surface           = TOKEN_HEX(0xDADADA),
    .inverse_on_surface        = TOKEN_HEX(0x2E2E2E),
    .inverse_primary           = TOKEN_HEX(0x2F5D86),

    .outline                   = TOKEN_HEX(0x8A8A8A),
    .outline_variant           = TOKEN_HEX(0x4A4A4A),
    .scrim                     = TOKEN_HEX(0x000000),
    .shadow                    = TOKEN_HEX(0x000000),

    .focus                     = TOKEN_HEX(0x8CB8E6),

    .state                     = TOKEN_STATE_MD3,

    .custom                    = alarms_dark,
    .custom_count              = TOKEN_ALARM_COUNT,
};
