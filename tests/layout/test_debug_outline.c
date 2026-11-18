/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * TRLV_USE_DEBUG_OUTLINE. Built twice: against the normal library (option
 * off) and against a copy built with the option on. Either way the layout is
 * the same; only the outline differs.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    return screen;
}

static void assert_outline(lv_obj_t * obj)
{
    TRLV_ASSERT_INT(lv_obj_get_style_outline_width(obj, LV_PART_MAIN), TRLV_USE_DEBUG_OUTLINE ? 1 : 0);
#if TRLV_USE_DEBUG_OUTLINE
    TRLV_ASSERT_INT(lv_obj_get_style_outline_pad(obj, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_outline_opa(obj, LV_PART_MAIN), LV_OPA_COVER);
#endif
}

/* Box, row, column and spacer each carry the outline when it is on. */
static void containers_have_the_outline(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen), TRLV_SPACE_SM);
    lv_obj_set_height(column, 100);
    lv_obj_t * row = trlv_row_create(column, TRLV_SPACE_SM);
    lv_obj_t * box = trlv_box_create(row);
    lv_obj_t * spacer = trlv_spacer_create(column);

    assert_outline(column);
    assert_outline(row);
    assert_outline(box);
    assert_outline(spacer);
}

/* The outline takes no space: positions and sizes are unchanged. */
static void layout_is_unchanged(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen), TRLV_SPACE_SM);
    trlv_set_pad(row, TRLV_SPACE_LG);
    lv_obj_t * a = trlv_box_create(row);
    lv_obj_set_size(a, 30, 20);
    lv_obj_t * b = trlv_box_create(row);
    lv_obj_set_size(b, 40, 10);

    TRLV_ASSERT_RECT(row, 0, 0, 16 + 30 + 8 + 40 + 16, 16 + 20 + 16);
    TRLV_ASSERT_RECT(a, 16, 16, 30, 20);
    TRLV_ASSERT_RECT(b, 16 + 30 + 8, 16, 40, 10);
}

/* LVGL widgets are not outlined, and neither is a divider: it draws a line
 * of its own. */
static void widgets_and_dividers_are_not_outlined(lv_obj_t * screen)
{
    lv_obj_t * label = lv_label_create(setup(screen));
    TRLV_ASSERT_INT(lv_obj_get_style_outline_width(label, LV_PART_MAIN), 0);

    lv_obj_t * column = trlv_column_create(screen, TRLV_SPACE_NONE);
    lv_obj_t * divider = trlv_divider_create(column);
    TRLV_ASSERT_INT(lv_obj_get_style_outline_width(divider, LV_PART_MAIN), 0);
}

int main(void)
{
    TRLV_TEST_RUN(containers_have_the_outline);
    TRLV_TEST_RUN(layout_is_unchanged);
    TRLV_TEST_RUN(widgets_and_dividers_are_not_outlined);
    return trlv_test_finish();
}
