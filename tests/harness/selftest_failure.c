/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Harness self-test: a deliberately wrong assertion. CTest expects this
 * program to fail, and checks that the message names the file, line, test,
 * expected and actual values (see tests/CMakeLists.txt).
 */

#include "trlv_test.h"

static void wrong_expectation(lv_obj_t * screen)
{
    lv_obj_t * box = lv_obj_create(screen);
    lv_obj_remove_style_all(box);
    lv_obj_set_pos(box, 10, 0);

    TRLV_ASSERT_X(box, 99);   /* deliberately wrong: box.x is 10 */
}

int main(void)
{
    TRLV_TEST_RUN(wrong_expectation);
    return trlv_test_finish();
}
