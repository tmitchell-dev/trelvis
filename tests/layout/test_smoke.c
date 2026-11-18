/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Smoke tests for the harness itself, using plain LVGL objects. The objects
 * have their theme styles removed so the results don't depend on LVGL's
 * default theme.
 */

#include "trlv_test.h"

static lv_obj_t * plain_obj(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    return obj;
}

/* The test display has the requested size. */
static void screen_matches_display(lv_obj_t * screen)
{
    TRLV_ASSERT_RECT(screen, 0, 0, 800, 480);
}

/* An object placed by hand is where it was put. */
static void explicit_position_and_size(lv_obj_t * screen)
{
    lv_obj_t * box = plain_obj(screen);
    lv_obj_set_pos(box, 10, 20);
    lv_obj_set_size(box, 100, 50);

    TRLV_ASSERT_RECT(box, 10, 20, 100, 50);
}

/* Positions include the parent's padding (measured from its outer edge),
 * and a flex row places its children one after another with the gap. */
static void flex_row_places_children(lv_obj_t * screen)
{
    lv_obj_t * row = plain_obj(screen);
    lv_obj_set_pos(row, 5, 5);
    lv_obj_set_size(row, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(row, 16, 0);
    lv_obj_set_style_pad_column(row, 8, 0);

    lv_obj_t * a = plain_obj(row);
    lv_obj_set_size(a, 40, 20);
    lv_obj_t * b = plain_obj(row);
    lv_obj_set_size(b, 30, 10);

    TRLV_ASSERT_RECT(a, 16, 16, 40, 20);
    TRLV_ASSERT_RECT(b, 16 + 40 + 8, 16, 30, 10);
    /* The row fits its children: padding + a + gap + b + padding. */
    TRLV_ASSERT_RECT(row, 5, 5, 16 + 40 + 8 + 30 + 16, 16 + 20 + 16);
}

/* A custom display size applies to the tests that follow. */
static void custom_display_size(lv_obj_t * screen)
{
    TRLV_ASSERT_WIDTH(screen, 320);
    TRLV_ASSERT_HEIGHT(screen, 240);
}

int main(void)
{
    TRLV_TEST_RUN(screen_matches_display);
    TRLV_TEST_RUN(explicit_position_and_size);
    TRLV_TEST_RUN(flex_row_places_children);

    trlv_test_set_display_size(320, 240);
    TRLV_TEST_RUN(custom_display_size);

    return trlv_test_finish();
}
