/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_spacer_create: takes the free space along the parent's main axis and none
 * across it. Uses gap NONE so the arithmetic is plain.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#include <string.h>

static int spacer_warnings;

static void log_cb(lv_log_level_t level, const char * buf)
{
    if(level == LV_LOG_LEVEL_WARN && strstr(buf, "trlv_spacer_create(") != NULL) spacer_warnings++;
}

/* lv_deinit clears the log callback, so it is registered in every test. */
static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    spacer_warnings = 0;
    lv_log_register_print_cb(log_cb);
    return screen;
}

static lv_obj_t * child(lv_obj_t * parent, int32_t w, int32_t h)
{
    lv_obj_t * obj = trlv_box_create(parent);
    lv_obj_set_size(obj, w, h);
    return obj;
}

/* In a row: pushes the next child to the right edge; 0 tall. */
static void pushes_apart_in_a_row(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    lv_obj_set_size(row, 200, 40);
    child(row, 30, 20);
    lv_obj_t * spacer = trlv_spacer_create(row);
    lv_obj_t * last = child(row, 40, 20);

    TRLV_ASSERT_RECT(spacer, 30, 0, 200 - 30 - 40, 0);
    TRLV_ASSERT_X(last, 160);
    TRLV_ASSERT_INT(spacer_warnings, 0);
}

/* In a column: pushes the next child to the bottom; 0 wide. */
static void pushes_apart_in_a_column(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    lv_obj_set_size(column, 40, 200);
    child(column, 20, 30);
    lv_obj_t * spacer = trlv_spacer_create(column);
    lv_obj_t * last = child(column, 20, 40);

    TRLV_ASSERT_RECT(spacer, 0, 30, 0, 200 - 30 - 40);
    TRLV_ASSERT_Y(last, 160);
}

/* The gap still applies on both sides of the spacer. */
static void respects_the_gap(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    lv_obj_set_size(row, 200, 40);
    child(row, 30, 20);
    lv_obj_t * spacer = trlv_spacer_create(row);
    lv_obj_t * last = child(row, 40, 20);

    TRLV_ASSERT_WIDTH(spacer, 200 - 30 - 40 - 2 * 8);
    TRLV_ASSERT_X(last, 160);
}

/* Several spacers share the free space equally. */
static void spacers_share_the_space(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    lv_obj_set_size(row, 200, 40);
    child(row, 20, 20);
    trlv_spacer_create(row);
    lv_obj_t * middle = child(row, 20, 20);
    trlv_spacer_create(row);
    lv_obj_t * last = child(row, 20, 20);

    TRLV_ASSERT_X(middle, 20 + 70);
    TRLV_ASSERT_X(last, 180);
}

/* In a row that hugs its width there is no free space: 0 wide, and a
 * warning naming trlv_spacer_create. */
static void warns_in_a_hugging_row(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    child(row, 30, 20);
    lv_obj_t * spacer = trlv_spacer_create(row);

    TRLV_ASSERT_WIDTH(spacer, 0);
    TRLV_ASSERT_INT(spacer_warnings, 1);
}

/* Without a flex parent it has no size. */
static void no_size_without_flex(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    lv_obj_set_size(box, 200, 100);
    lv_obj_t * spacer = trlv_spacer_create(box);

    TRLV_ASSERT_RECT(spacer, 0, 0, 0, 0);
    TRLV_ASSERT_INT(spacer_warnings, 0);
}

/* Works in a flex lv_obj TreLVis did not create. */
static void works_in_a_plain_flex_object(lv_obj_t * screen)
{
    lv_obj_t * obj = lv_obj_create(setup(screen));
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, 200, 40);
    lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW);
    child(obj, 30, 20);
    trlv_spacer_create(obj);
    lv_obj_t * last = child(obj, 40, 20);

    TRLV_ASSERT_X(last, 160);
}

int main(void)
{
    TRLV_TEST_RUN(pushes_apart_in_a_row);
    TRLV_TEST_RUN(pushes_apart_in_a_column);
    TRLV_TEST_RUN(respects_the_gap);
    TRLV_TEST_RUN(spacers_share_the_space);
    TRLV_TEST_RUN(warns_in_a_hugging_row);
    TRLV_TEST_RUN(no_size_without_flex);
    TRLV_TEST_RUN(works_in_a_plain_flex_object);
    return trlv_test_finish();
}
