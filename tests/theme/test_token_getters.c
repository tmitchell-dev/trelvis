/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Token value getters: trlv_get_space, trlv_get_radius and
 * trlv_get_control_height return the layout tokens' values, and 0 for a
 * value that isn't a step or before trlv_init.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

/* Before trlv_init there are no tokens: 0. This runs first, on a fresh
 * LVGL, before any test has called trlv_init. */
static void zero_before_init(lv_obj_t * screen)
{
    LV_UNUSED(screen);
    if(trlv_get_layout_tokens() != NULL) return;   /* another test ran first */
    TRLV_ASSERT_INT(trlv_get_space(TRLV_SPACE_LG), 0);
    TRLV_ASSERT_INT(trlv_get_radius(TRLV_RADIUS_MD), 0);
    TRLV_ASSERT_INT(trlv_get_control_height(TRLV_SIZE_MD), 0);
}

/* Every step matches the layout tokens passed to trlv_init. */
static void match_the_layout_tokens(lv_obj_t * screen)
{
    const trlv_layout_tokens_t * layouts[] = { &tokens_regular, &tokens_compact };
    for(uint32_t l = 0; l < 2; l++) {
        const trlv_layout_tokens_t * t = layouts[l];
        trlv_init(lv_obj_get_display(screen), &tokens_light, t);
        for(int32_t i = 0; i < TRLV_SPACE_COUNT; i++) {
            TRLV_ASSERT_INT(trlv_get_space((trlv_space_t)i), t->space[i]);
        }
        for(int32_t i = 0; i < TRLV_RADIUS_COUNT; i++) {
            TRLV_ASSERT_INT(trlv_get_radius((trlv_radius_t)i), t->radius[i]);
        }
        for(int32_t i = 0; i < TRLV_SIZE_COUNT; i++) {
            TRLV_ASSERT_INT(trlv_get_control_height((trlv_size_t)i), t->control_height[i]);
        }
    }
}

/* A value that isn't a step gives 0. */
static void zero_for_an_invalid_value(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    TRLV_ASSERT_INT(trlv_get_space(TRLV_SPACE_COUNT), 0);
    TRLV_ASSERT_INT(trlv_get_radius(TRLV_RADIUS_COUNT), 0);
    TRLV_ASSERT_INT(trlv_get_control_height(TRLV_SIZE_COUNT), 0);
    TRLV_ASSERT_INT(trlv_get_space((trlv_space_t)-1), 0);
}

/* The getter gives the same value a modifier uses: a raw padding next to
 * trlv_set_pad reads in the same dialect. */
static void same_value_as_the_modifier(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    lv_obj_t * bar = trlv_row_create(screen);
    trlv_set_pad(bar, TRLV_SPACE_SM);
    lv_obj_set_style_pad_hor(bar, trlv_get_space(TRLV_SPACE_LG), 0);

    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(bar, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_SM]);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(bar, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_LG]);
}

int main(void)
{
    TRLV_TEST_RUN(zero_before_init);
    TRLV_TEST_RUN(match_the_layout_tokens);
    TRLV_TEST_RUN(zero_for_an_invalid_value);
    TRLV_TEST_RUN(same_value_as_the_modifier);
    return trlv_test_finish();
}
