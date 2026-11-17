/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * TRLV_USE_THEME 0: trlv_init keeps the tokens and builds the shared styles,
 * but installs no theme. Built against a copy of the library compiled with
 * TRLV_USE_THEME=0 (see tests/CMakeLists.txt).
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#if TRLV_USE_THEME
#error "this test must be built with TRLV_USE_THEME=0"
#endif

#define ASSERT_TRUE(cond) \
    trlv_test_check_int(__FILE__, __LINE__, #cond, 1, (cond) ? 1 : 0)

/* The display keeps the theme it had. */
static void display_keeps_its_theme(lv_obj_t * screen)
{
    lv_display_t * disp = lv_obj_get_display(screen);
    lv_theme_t * before = lv_display_get_theme(disp);

    trlv_init(disp, &tokens_light, &tokens_regular);

    ASSERT_TRUE(lv_display_get_theme(disp) == before);
    ASSERT_TRUE(trlv_color() == &tokens_light);
}

/* Widgets are not given MD3 styles... */
static void buttons_are_not_md3(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    lv_obj_t * btn = lv_button_create(screen);

    ASSERT_TRUE(lv_obj_get_style_radius(btn, LV_PART_MAIN) != LV_RADIUS_CIRCLE);
}

/* ...but the shared styles still work. */
static void shared_styles_still_work(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    lv_obj_t * box = trlv_box(screen);
    trlv_pad(box, TRLV_SPACE_LG);

    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(box, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_LG]);
}

/* Surfaces and button variants set their colours without the theme. */
static void colour_modifiers_still_work(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    lv_obj_t * box = trlv_box(screen);
    trlv_surface(box, TRLV_SURFACE_CONTAINER_HIGH);
    lv_obj_t * btn = lv_button_create(screen);
    trlv_button_variant(btn, TRLV_BUTTON_TONAL);

    ASSERT_TRUE(lv_color_eq(lv_obj_get_style_bg_color(box, LV_PART_MAIN),
                            tokens_light.surface_container_high));
    ASSERT_TRUE(lv_color_eq(lv_obj_get_style_bg_color(btn, LV_PART_MAIN),
                            tokens_light.secondary_container));
    ASSERT_TRUE(lv_color_eq(lv_obj_get_style_text_color(btn, LV_PART_MAIN),
                            tokens_light.on_secondary_container));
}

int main(void)
{
    TRLV_TEST_RUN(display_keeps_its_theme);
    TRLV_TEST_RUN(buttons_are_not_md3);
    TRLV_TEST_RUN(shared_styles_still_work);
    TRLV_TEST_RUN(colour_modifiers_still_work);
    return trlv_test_finish();
}
