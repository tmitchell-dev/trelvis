/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * The token check (token_check.c) on the token template: both colour
 * modes pass, and the check finds the problems it is meant to find.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"
#include "token_check.h"

/* The template's two colour modes pass every check. */
static void template_passes(lv_obj_t * screen)
{
    LV_UNUSED(screen);
    TRLV_ASSERT_INT((int32_t)token_check_color_mode("tokens_light", &tokens_light), 0);
    TRLV_ASSERT_INT((int32_t)token_check_color_mode("tokens_dark", &tokens_dark), 0);
    TRLV_ASSERT_INT((int32_t)token_check_color_modes_match("tokens_light", &tokens_light,
                                                           "tokens_dark", &tokens_dark), 0);
}

/* The contrast ratio follows WCAG: 21:1 for black on white, 1:1 for a
 * colour on itself. */
static void contrast_ratio(lv_obj_t * screen)
{
    LV_UNUSED(screen);
    TRLV_ASSERT_INT((int32_t)(token_check_contrast(lv_color_black(), lv_color_white()) * 10 + 0.5), 210);
    TRLV_ASSERT_INT((int32_t)(token_check_contrast(lv_color_hex(0x6750A4), lv_color_hex(0x6750A4)) * 10 + 0.5), 10);
}

/* Unreadable text on a container is reported. */
static void finds_low_contrast(lv_obj_t * screen)
{
    LV_UNUSED(screen);
    trlv_color_tokens_t broken = tokens_light;
    broken.on_primary_container = broken.primary_container;
    TRLV_ASSERT_INT(token_check_color_mode("broken", &broken) >= 1, true);
}

/* A custom colour that is the error, primary or focus colour is reported. */
static void finds_a_custom_colour_that_means_something_else(lv_obj_t * screen)
{
    LV_UNUSED(screen);
    trlv_custom_color_t custom[TOKEN_CUSTOM_COUNT];
    for(uint32_t i = 0; i < TOKEN_CUSTOM_COUNT; i++) custom[i] = tokens_light.custom[i];
    custom[TOKEN_CUSTOM_FAULT].color = tokens_light.error;
    custom[TOKEN_CUSTOM_FAULT].on_color = tokens_light.on_error;

    trlv_color_tokens_t broken = tokens_light;
    broken.custom = custom;
    TRLV_ASSERT_INT((int32_t)token_check_color_mode("broken", &broken), 1);
}

/* Colour modes with different custom colour counts are reported. */
static void finds_modes_that_differ(lv_obj_t * screen)
{
    LV_UNUSED(screen);
    trlv_color_tokens_t fewer = tokens_dark;
    fewer.custom_count = 1;
    TRLV_ASSERT_INT((int32_t)token_check_color_modes_match("tokens_light", &tokens_light, "fewer", &fewer), 1);
}

int main(void)
{
    TRLV_TEST_RUN(template_passes);
    TRLV_TEST_RUN(contrast_ratio);
    TRLV_TEST_RUN(finds_low_contrast);
    TRLV_TEST_RUN(finds_a_custom_colour_that_means_something_else);
    TRLV_TEST_RUN(finds_modes_that_differ);
    return trlv_test_finish();
}
