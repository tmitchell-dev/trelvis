/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_box_create: a Figma frame without auto layout.
 */

#include "trlv_test.h"

/* A fixed-size child placed by hand. */
static lv_obj_t * child_at(lv_obj_t * parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
    lv_obj_t * obj = trlv_box_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);
    return obj;
}

/* A box fits its children: it is as big as the furthest child edge. */
static void fits_its_children(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(screen);
    lv_obj_set_pos(box, 10, 20);
    child_at(box, 0, 0, 40, 30);
    child_at(box, 50, 10, 20, 60);

    TRLV_ASSERT_RECT(box, 10, 20, 50 + 20, 10 + 60);
}

/* An empty box has no size. */
static void empty_box_is_zero_size(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(screen);

    TRLV_ASSERT_RECT(box, 0, 0, 0, 0);
}

/* No theme padding or border: a child at (0, 0) sits at the box's outer corner. */
static void no_padding_or_border(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(screen);
    lv_obj_t * child = child_at(box, 0, 0, 10, 10);

    TRLV_ASSERT_RECT(child, 0, 0, 10, 10);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(box, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(box, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(box, LV_PART_MAIN), 0);
}

/* Transparent: no theme background. */
static void transparent(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(screen);

    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(box, LV_PART_MAIN), LV_OPA_TRANSP);
}

/* Not scrollable and not clickable, unlike a plain lv_obj. */
static void not_scrollable_or_clickable(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(screen);
    lv_obj_t * plain = lv_obj_create(screen);

    TRLV_ASSERT_INT(lv_obj_is_scrollable(box), false);
    TRLV_ASSERT_INT(lv_obj_is_clickable(box), false);
    /* The difference is deliberate: LVGL's default is the opposite. */
    TRLV_ASSERT_INT(lv_obj_is_scrollable(plain), true);
    TRLV_ASSERT_INT(lv_obj_is_clickable(plain), true);
}

/* A fixed size set with LVGL replaces the content size. */
static void fixed_size_with_lvgl(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(screen);
    lv_obj_set_size(box, 120, 80);
    child_at(box, 0, 0, 10, 10);

    TRLV_ASSERT_RECT(box, 0, 0, 120, 80);
}

int main(void)
{
    TRLV_TEST_RUN(fits_its_children);
    TRLV_TEST_RUN(empty_box_is_zero_size);
    TRLV_TEST_RUN(no_padding_or_border);
    TRLV_TEST_RUN(transparent);
    TRLV_TEST_RUN(not_scrollable_or_clickable);
    TRLV_TEST_RUN(fixed_size_with_lvgl);
    return trlv_test_finish();
}
