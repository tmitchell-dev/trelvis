/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file token_check.c
 * A small check of a colour token set. See token_check.h.
 */

#include "token_check.h"

#include <math.h>
#include <stdio.h>

/* WCAG 2.x relative luminance of one sRGB channel. */
static double channel(uint8_t value)
{
    double c = value / 255.0;
    return c <= 0.03928 ? c / 12.92 : pow((c + 0.055) / 1.055, 2.4);
}

static double luminance(lv_color_t color)
{
    return 0.2126 * channel(color.red) + 0.7152 * channel(color.green) + 0.0722 * channel(color.blue);
}

double token_check_contrast(lv_color_t a, lv_color_t b)
{
    double la = luminance(a);
    double lb = luminance(b);
    double hi = la > lb ? la : lb;
    double lo = la > lb ? lb : la;
    return (hi + 0.05) / (lo + 0.05);
}

static bool same_color(lv_color_t a, lv_color_t b)
{
    return a.red == b.red && a.green == b.green && a.blue == b.blue;
}

/* One contrast check: prints and counts a problem when below `minimum`. */
static uint32_t pair(const char * mode, const char * fg_name, lv_color_t fg,
                     const char * bg_name, lv_color_t bg, double minimum)
{
    double ratio = token_check_contrast(fg, bg);
    if(ratio + 1e-9 >= minimum) return 0;
    printf("%s: %s on %s has contrast %.2f:1, below %.1f:1\n", mode, fg_name, bg_name, ratio, minimum);
    return 1;
}

#define TEXT_PAIR(fg, bg) pair(mode_name, #fg, color->fg, #bg, color->bg, TOKEN_CHECK_TEXT_CONTRAST)
#define UI_PAIR(fg, bg)   pair(mode_name, #fg, color->fg, #bg, color->bg, TOKEN_CHECK_UI_CONTRAST)

uint32_t token_check_color_mode(const char * mode_name, const trlv_color_tokens_t * color)
{
    uint32_t problems = 0;

    if(color == NULL) {
        printf("%s: no colour tokens\n", mode_name);
        return 1;
    }

    /* Content colours on the colour they are paired with. */
    problems += TEXT_PAIR(on_primary, primary);
    problems += TEXT_PAIR(on_primary_container, primary_container);
    problems += TEXT_PAIR(on_secondary, secondary);
    problems += TEXT_PAIR(on_secondary_container, secondary_container);
    problems += TEXT_PAIR(on_tertiary, tertiary);
    problems += TEXT_PAIR(on_tertiary_container, tertiary_container);
    problems += TEXT_PAIR(on_error, error);
    problems += TEXT_PAIR(on_error_container, error_container);
    problems += TEXT_PAIR(inverse_on_surface, inverse_surface);

    /* Normal and lower-emphasis text on every surface. */
    problems += TEXT_PAIR(on_surface, surface);
    problems += TEXT_PAIR(on_surface, surface_dim);
    problems += TEXT_PAIR(on_surface, surface_bright);
    problems += TEXT_PAIR(on_surface, surface_container_lowest);
    problems += TEXT_PAIR(on_surface, surface_container_low);
    problems += TEXT_PAIR(on_surface, surface_container);
    problems += TEXT_PAIR(on_surface, surface_container_high);
    problems += TEXT_PAIR(on_surface, surface_container_highest);
    problems += TEXT_PAIR(on_surface_variant, surface);
    problems += TEXT_PAIR(on_surface_variant, surface_dim);
    problems += TEXT_PAIR(on_surface_variant, surface_bright);
    problems += TEXT_PAIR(on_surface_variant, surface_container_lowest);
    problems += TEXT_PAIR(on_surface_variant, surface_container_low);
    problems += TEXT_PAIR(on_surface_variant, surface_container);
    problems += TEXT_PAIR(on_surface_variant, surface_container_high);
    problems += TEXT_PAIR(on_surface_variant, surface_container_highest);

    /* UI parts on the page. */
    problems += UI_PAIR(outline, surface);
    problems += UI_PAIR(focus, surface);

    /* Custom colours: both pairs readable, and the strong colour kept apart
     * from the roles that mean something else. */
    if(color->custom_count > 0 && color->custom == NULL) {
        printf("%s: custom_count is %u but custom is NULL\n", mode_name, (unsigned)color->custom_count);
        problems++;
    }
    else {
        for(uint32_t i = 0; i < color->custom_count; i++) {
            const trlv_custom_color_t * custom = &color->custom[i];
            char fg[48];
            char bg[48];

            snprintf(fg, sizeof(fg), "custom[%u].on_color", (unsigned)i);
            snprintf(bg, sizeof(bg), "custom[%u].color", (unsigned)i);
            problems += pair(mode_name, fg, custom->on_color, bg, custom->color, TOKEN_CHECK_TEXT_CONTRAST);

            snprintf(fg, sizeof(fg), "custom[%u].on_color_container", (unsigned)i);
            snprintf(bg, sizeof(bg), "custom[%u].color_container", (unsigned)i);
            problems += pair(mode_name, fg, custom->on_color_container, bg, custom->color_container,
                             TOKEN_CHECK_TEXT_CONTRAST);

            const struct {
                const char * name;
                lv_color_t value;
            } reserved[] = {
                { "error", color->error },
                { "primary", color->primary },
                { "focus", color->focus },
            };
            for(uint32_t r = 0; r < sizeof(reserved) / sizeof(reserved[0]); r++) {
                if(same_color(custom->color, reserved[r].value)) {
                    printf("%s: custom[%u].color is the same as %s\n", mode_name, (unsigned)i, reserved[r].name);
                    problems++;
                }
            }
        }
    }

    return problems;
}

uint32_t token_check_color_modes_match(const char * a_name, const trlv_color_tokens_t * a,
                                       const char * b_name, const trlv_color_tokens_t * b)
{
    if(a == NULL || b == NULL) return 0;
    if(a->custom_count == b->custom_count) return 0;
    printf("%s has %u custom colours, %s has %u: every colour mode needs the same entries\n",
           a_name, (unsigned)a->custom_count, b_name, (unsigned)b->custom_count);
    return 1;
}
