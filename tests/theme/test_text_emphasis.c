/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_apply_text_emphasis: lower-emphasis text (on_surface_variant) on a
 * label or inherited from a container, removed by NORMAL, following
 * colour-mode switches, and independent of the text role.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    return screen;
}

/* Let LVGL's style transitions finish. */
static void settle(void)
{
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
}

static lv_color_t text_of(lv_obj_t * obj)
{
    return lv_obj_get_style_text_color(obj, LV_PART_MAIN);
}

/* A surface container holding a column with a label two levels down. */
static lv_obj_t * panel_with_label(lv_obj_t * parent, lv_obj_t ** column, lv_obj_t ** label)
{
    lv_obj_t * panel = trlv_column_create(parent);
    trlv_apply_surface(panel, TRLV_SURFACE_CONTAINER);
    *column = trlv_column_create(panel);
    lv_obj_t * row = trlv_row_create(*column);
    *label = trlv_text_create(row, "TI-101", TRLV_TEXT_LABEL_MEDIUM);
    return panel;
}

/* LOW on a label: the label takes on_surface_variant. */
static void low_on_a_label(lv_obj_t * screen)
{
    lv_obj_t * column;
    lv_obj_t * label;
    panel_with_label(setup(screen), &column, &label);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface);

    trlv_apply_text_emphasis(label, TRLV_TEXT_EMPHASIS_LOW);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface_variant);
}

/* LOW on a container: every label inside inherits it. */
static void low_on_a_container(lv_obj_t * screen)
{
    lv_obj_t * column;
    lv_obj_t * label;
    panel_with_label(setup(screen), &column, &label);

    trlv_apply_text_emphasis(column, TRLV_TEXT_EMPHASIS_LOW);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface_variant);
}

/* NORMAL removes it: the label inherits the surface's on_ colour again. */
static void normal_removes_it(lv_obj_t * screen)
{
    lv_obj_t * column;
    lv_obj_t * label;
    panel_with_label(setup(screen), &column, &label);

    trlv_apply_text_emphasis(label, TRLV_TEXT_EMPHASIS_LOW);
    trlv_apply_text_emphasis(label, TRLV_TEXT_EMPHASIS_NORMAL);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface);

    /* NORMAL on an object without it changes nothing. */
    trlv_apply_text_emphasis(label, TRLV_TEXT_EMPHASIS_NORMAL);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface);
}

/* The style is shared and re-set on a colour-mode switch. */
static void follows_the_color_mode(lv_obj_t * screen)
{
    lv_obj_t * column;
    lv_obj_t * label;
    panel_with_label(setup(screen), &column, &label);
    trlv_apply_text_emphasis(column, TRLV_TEXT_EMPHASIS_LOW);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();
    ASSERT_COLOR(text_of(label), tokens_dark.on_surface_variant);

    trlv_theme_set_color_mode(&tokens_light);
    settle();
    ASSERT_COLOR(text_of(label), tokens_light.on_surface_variant);
}

/* Emphasis sets only the colour; the text role keeps the font. */
static void combines_with_the_text_role(lv_obj_t * screen)
{
    lv_obj_t * column;
    lv_obj_t * label;
    panel_with_label(setup(screen), &column, &label);

    trlv_apply_text_emphasis(label, TRLV_TEXT_EMPHASIS_LOW);
    trlv_apply_text_role(label, TRLV_TEXT_BODY_SMALL);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface_variant);
    TRLV_ASSERT_INT(lv_obj_get_style_text_font(label, LV_PART_MAIN) == tokens_regular.text[TRLV_TEXT_BODY_SMALL].font,
                    true);
}

/* On an object that also has a surface, the one added last wins, as in
 * LVGL: emphasis after the surface gives lower-emphasis text. */
static void after_a_surface_on_the_same_object(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_column_create(setup(screen));
    lv_obj_t * label = trlv_text_create(box, "kPa", TRLV_TEXT_BODY_MEDIUM);
    trlv_apply_surface(box, TRLV_SURFACE_CONTAINER_HIGH);
    trlv_apply_text_emphasis(box, TRLV_TEXT_EMPHASIS_LOW);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface_variant);
}

/* A value that isn't an emphasis changes nothing. */
static void ignores_an_invalid_value(lv_obj_t * screen)
{
    lv_obj_t * column;
    lv_obj_t * label;
    panel_with_label(setup(screen), &column, &label);

    trlv_apply_text_emphasis(label, TRLV_TEXT_EMPHASIS_COUNT);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface);
}

int main(void)
{
    TRLV_TEST_RUN(low_on_a_label);
    TRLV_TEST_RUN(low_on_a_container);
    TRLV_TEST_RUN(normal_removes_it);
    TRLV_TEST_RUN(follows_the_color_mode);
    TRLV_TEST_RUN(combines_with_the_text_role);
    TRLV_TEST_RUN(after_a_surface_on_the_same_object);
    TRLV_TEST_RUN(ignores_an_invalid_value);
    return trlv_test_finish();
}
