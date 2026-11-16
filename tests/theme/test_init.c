/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_init and the token accessors, using the token template.
 *
 * TreLVis keeps its token pointers across tests (the harness resets LVGL,
 * not TreLVis), so these tests run in order and each states what it starts
 * from.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_PTR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual " == " #expected, 1, (actual) == (expected))

/* Before trlv_init, there are no tokens. */
static void accessors_are_null_before_init(lv_obj_t * screen)
{
    (void)screen;
    ASSERT_PTR(trlv_color(), NULL);
    ASSERT_PTR(trlv_layout(), NULL);
}

/* The accessors return exactly the tokens passed in (not copies). */
static void init_stores_the_tokens(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);

    ASSERT_PTR(trlv_color(), &tokens_light);
    ASSERT_PTR(trlv_layout(), &tokens_regular);
}

/* Calling it again switches to the new tokens. Starts from: light. */
static void init_again_uses_the_new_tokens(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_dark, &tokens_regular);

    ASSERT_PTR(trlv_color(), &tokens_dark);
    ASSERT_PTR(trlv_layout(), &tokens_regular);
}

/* A NULL display or token pointer is rejected and nothing changes.
 * Starts from: dark. */
static void null_arguments_are_rejected(lv_obj_t * screen)
{
    lv_display_t * disp = lv_obj_get_display(screen);

    trlv_init(NULL, &tokens_light, &tokens_regular);
    trlv_init(disp, NULL, &tokens_regular);
    trlv_init(disp, &tokens_light, NULL);

    ASSERT_PTR(trlv_color(), &tokens_dark);
    ASSERT_PTR(trlv_layout(), &tokens_regular);
}

int main(void)
{
    TRLV_TEST_RUN(accessors_are_null_before_init);
    TRLV_TEST_RUN(init_stores_the_tokens);
    TRLV_TEST_RUN(init_again_uses_the_new_tokens);
    TRLV_TEST_RUN(null_arguments_are_rejected);
    return trlv_test_finish();
}
