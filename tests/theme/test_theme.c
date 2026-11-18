/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * The MD3 theme: installation over LVGL's default theme, the screen, plain
 * objects, labels and the filled button, using the token template.
 *
 * The harness gives each test an empty display, so trlv_init comes before
 * any object is created, as it should in a project.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

#define ASSERT_TRUE(cond) \
    trlv_test_check_int(__FILE__, __LINE__, #cond, 1, (cond) ? 1 : 0)

static lv_display_t * init_with(lv_obj_t * screen, const trlv_color_tokens_t * color)
{
    lv_display_t * disp = lv_obj_get_display(screen);
    trlv_init(disp, color, &tokens_regular);
    return disp;
}

/* trlv_init installs a theme of its own, with LVGL's default theme, seeded
 * with the MD3 colours and the Body Medium font, as its parent. */
static void theme_is_installed_over_the_default_theme(lv_obj_t * screen)
{
    lv_display_t * disp = init_with(screen, &tokens_light);
    lv_theme_t * theme = lv_display_get_theme(disp);

    ASSERT_TRUE(theme != NULL);
    ASSERT_TRUE(theme != lv_theme_default_get());
    ASSERT_COLOR(lv_theme_get_color_primary(screen), tokens_light.primary);
    ASSERT_COLOR(lv_theme_get_color_secondary(screen), tokens_light.secondary);
    ASSERT_TRUE(lv_theme_get_font_normal(screen) == tokens_regular.text[TRLV_TEXT_BODY_MEDIUM].font);
}

/* A widget the theme doesn't cover is styled by the parent theme, in the
 * MD3 primary colour. */
static void uncovered_widgets_use_the_parent_theme(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * roller = lv_roller_create(screen);

    ASSERT_COLOR(lv_obj_get_style_bg_color(roller, LV_PART_SELECTED), tokens_light.primary);
}

/* The parent theme is dark when `surface` is dark. */
static void dark_tokens_make_the_parent_theme_dark(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * light_roller = lv_roller_create(screen);
    lv_color_t light_bg = lv_obj_get_style_bg_color(light_roller, LV_PART_MAIN);

    init_with(screen, &tokens_dark);
    lv_obj_t * dark_roller = lv_roller_create(screen);
    lv_color_t dark_bg = lv_obj_get_style_bg_color(dark_roller, LV_PART_MAIN);

    ASSERT_TRUE(lv_color_luminance(light_bg) >= 128);
    ASSERT_TRUE(lv_color_luminance(dark_bg) < 128);
}

/* Calling trlv_init again reuses the theme: no new theme, no new memory. */
static void init_again_reuses_the_theme(lv_obj_t * screen)
{
    lv_display_t * disp = init_with(screen, &tokens_light);
    lv_theme_t * theme = lv_display_get_theme(disp);
    lv_mem_monitor_t before;
    lv_mem_monitor(&before);

    init_with(screen, &tokens_light);

    lv_mem_monitor_t after;
    lv_mem_monitor(&after);
    ASSERT_TRUE(lv_display_get_theme(disp) == theme);
    TRLV_ASSERT_INT((int32_t)after.total_size - (int32_t)after.free_size,
                    (int32_t)before.total_size - (int32_t)before.free_size);
}

/* The screen has the surface colour, and a label on it inherits on_surface
 * and Body Medium. */
static void screen_gives_labels_surface_colours_and_body_text(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * label = lv_label_create(screen);

    ASSERT_COLOR(lv_obj_get_style_bg_color(screen, LV_PART_MAIN), tokens_light.surface);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(screen, LV_PART_MAIN), LV_OPA_COVER);
    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_light.on_surface);
    ASSERT_TRUE(lv_obj_get_style_text_font(label, LV_PART_MAIN) ==
                tokens_regular.text[TRLV_TEXT_BODY_MEDIUM].font);
}

/* The same in dark mode. */
static void screen_in_dark_mode(lv_obj_t * screen)
{
    init_with(screen, &tokens_dark);
    lv_obj_t * label = lv_label_create(screen);

    ASSERT_COLOR(lv_obj_get_style_bg_color(screen, LV_PART_MAIN), tokens_dark.surface);
    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_dark.on_surface);
}

/* A plain lv_obj keeps LVGL's container shape, in MD3 colours: surface with
 * an outline_variant border and on_surface content. */
static void plain_obj_is_an_md3_container(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * obj = lv_obj_create(screen);
    lv_obj_t * label = lv_label_create(obj);

    ASSERT_COLOR(lv_obj_get_style_bg_color(obj, LV_PART_MAIN), tokens_light.surface);
    ASSERT_COLOR(lv_obj_get_style_border_color(obj, LV_PART_MAIN), tokens_light.outline_variant);
    ASSERT_TRUE(lv_obj_get_style_border_width(obj, LV_PART_MAIN) > 0);
    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_light.on_surface);
}

/* TreLVis boxes are not affected by the plain-object styles. */
static void box_stays_transparent(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * box = trlv_box_create(screen);

    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(box, LV_PART_MAIN), LV_OPA_TRANSP);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(box, LV_PART_MAIN), 0);
}

/* lv_button_create gives an MD3 filled button: primary, full radius, the
 * MD control height, 24 px side padding and Label Large on_primary text. */
static void button_is_an_md3_filled_button(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "OK");
    const lv_font_t * font = tokens_regular.text[TRLV_TEXT_LABEL_LARGE].font;
    lv_point_t text;
    lv_text_get_size(&text, "OK", font, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.primary);
    TRLV_ASSERT_INT(lv_obj_get_style_radius(btn, LV_PART_MAIN), LV_RADIUS_CIRCLE);
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_width(btn, LV_PART_MAIN), 0);
    TRLV_ASSERT_HEIGHT(btn, tokens_regular.control_height[TRLV_SIZE_MD]);
    TRLV_ASSERT_WIDTH(btn, text.x + 2 * tokens_regular.space[TRLV_SPACE_XL]);

    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_light.on_primary);
    ASSERT_TRUE(lv_obj_get_style_text_font(label, LV_PART_MAIN) == font);

    /* The label is centred. */
    TRLV_ASSERT_X(label, tokens_regular.space[TRLV_SPACE_XL]);
    TRLV_ASSERT_Y(label, (tokens_regular.control_height[TRLV_SIZE_MD] - trlv_test_height(label)) / 2);
}

/* An icon and a label sit side by side, 8 px apart, centred together. */
static void button_puts_icon_and_label_in_a_row(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_set_width(btn, 200);
    lv_obj_t * icon = trlv_box_create(btn);
    lv_obj_set_size(icon, 18, 18);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Save");

    int32_t content = 18 + tokens_regular.space[TRLV_SPACE_SM] + trlv_test_width(label);
    int32_t start = (200 - content) / 2;
    TRLV_ASSERT_X(icon, start);
    TRLV_ASSERT_X(label, start + 18 + tokens_regular.space[TRLV_SPACE_SM]);
}

/* The button height follows the layout tokens and can be changed with
 * LVGL, as the docs say. */
static void button_height_can_be_changed(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_set_height(btn, tokens_regular.control_height[TRLV_SIZE_LG]);

    TRLV_ASSERT_HEIGHT(btn, tokens_regular.control_height[TRLV_SIZE_LG]);
}

/* The row layout places a button's children, so lv_obj_align on them is
 * ignored. Turning the layout off on the button gives LVGL's placement
 * back; a floating child is placed by hand either way. */
static void button_layout_can_be_turned_off(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_set_width(btn, 200);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "OK");
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    int32_t pad = tokens_regular.space[TRLV_SPACE_XL];

    TRLV_ASSERT_X(label, (200 - trlv_test_width(label)) / 2);   /* centred by the row */

    lv_obj_set_layout(btn, LV_LAYOUT_NONE);
    TRLV_ASSERT_X(label, pad);                                  /* left, as aligned */

    lv_obj_t * badge = trlv_box_create(btn);
    lv_obj_set_size(badge, 8, 8);
    lv_obj_set_layout(btn, LV_LAYOUT_FLEX);
    lv_obj_set_floating(badge, true);
    lv_obj_align(badge, LV_ALIGN_TOP_RIGHT, 0, 0);
    TRLV_ASSERT_X(badge, 200 - pad - 8);
    TRLV_ASSERT_X(label, (200 - trlv_test_width(label)) / 2);   /* the row ignores the badge */
}

/* The tabs of an lv_tabview are lv_buttons, but not MD3 buttons: they keep
 * the parent theme's tab look. */
static void tabview_tabs_are_not_filled_buttons(lv_obj_t * screen)
{
    init_with(screen, &tokens_light);
    lv_obj_t * tabview = lv_tabview_create(screen);
    lv_tabview_add_tab(tabview, "One");
    lv_tabview_add_tab(tabview, "Two");
    lv_obj_t * tab = lv_obj_get_child(lv_tabview_get_tab_bar(tabview), 1);

    ASSERT_TRUE(lv_obj_check_type(tab, &lv_button_class));
    ASSERT_TRUE(lv_obj_get_style_radius(tab, LV_PART_MAIN) != LV_RADIUS_CIRCLE);
    ASSERT_TRUE(lv_obj_get_style_layout(tab, LV_PART_MAIN) != LV_LAYOUT_FLEX);
    ASSERT_TRUE(lv_obj_get_style_bg_opa(tab, LV_PART_MAIN) != LV_OPA_COVER);
}

/* In dark mode the button uses the dark primary pair. */
static void button_in_dark_mode(lv_obj_t * screen)
{
    init_with(screen, &tokens_dark);
    lv_obj_t * btn = lv_button_create(screen);
    lv_obj_t * label = lv_label_create(btn);

    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_dark.primary);
    ASSERT_COLOR(lv_obj_get_style_text_color(label, LV_PART_MAIN), tokens_dark.on_primary);
}

int main(void)
{
    TRLV_TEST_RUN(theme_is_installed_over_the_default_theme);
    TRLV_TEST_RUN(uncovered_widgets_use_the_parent_theme);
    TRLV_TEST_RUN(dark_tokens_make_the_parent_theme_dark);
    TRLV_TEST_RUN(init_again_reuses_the_theme);
    TRLV_TEST_RUN(screen_gives_labels_surface_colours_and_body_text);
    TRLV_TEST_RUN(screen_in_dark_mode);
    TRLV_TEST_RUN(plain_obj_is_an_md3_container);
    TRLV_TEST_RUN(box_stays_transparent);
    TRLV_TEST_RUN(button_is_an_md3_filled_button);
    TRLV_TEST_RUN(button_puts_icon_and_label_in_a_row);
    TRLV_TEST_RUN(button_height_can_be_changed);
    TRLV_TEST_RUN(button_layout_can_be_turned_off);
    TRLV_TEST_RUN(tabview_tabs_are_not_filled_buttons);
    TRLV_TEST_RUN(button_in_dark_mode);
    return trlv_test_finish();
}
