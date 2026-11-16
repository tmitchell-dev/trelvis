/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Style groups (src/trlv_style_group.h): modifiers replace rather than stack.
 *
 * Which styles an object carries is checked through their effect: each test
 * style sets a distinct value, and objects start with all styles removed, so
 * every property starts at 0.
 */

#include "trlv_test.h"
#include "trlv_style_group.h"

/* A pad group (contiguous array), a gap group (list of pointers) and a
 * project's own styles. */
static lv_style_t pad[3];
static lv_style_t gap_a;
static lv_style_t gap_b;
static const lv_style_t * const gap_list[] = { &gap_a, &gap_b };
static lv_style_t project_bg;
static lv_style_t project_pad;

static const trlv_style_group_t pad_group = TRLV_STYLE_GROUP_ARRAY(pad);
static const trlv_style_group_t gap_group = TRLV_STYLE_GROUP_LIST(gap_list);

/* Styles are rebuilt for every test, because each test runs on a fresh LVGL. */
static lv_obj_t * setup(lv_obj_t * screen)
{
    static const int32_t pad_values[3] = { 4, 8, 12 };
    for(int i = 0; i < 3; i++) {
        lv_style_init(&pad[i]);
        lv_style_set_pad_all(&pad[i], pad_values[i]);
    }
    lv_style_init(&gap_a);
    lv_style_set_pad_row(&gap_a, 2);
    lv_style_init(&gap_b);
    lv_style_set_pad_row(&gap_b, 6);
    lv_style_init(&project_bg);
    lv_style_set_bg_opa(&project_bg, LV_OPA_COVER);
    lv_style_init(&project_pad);
    lv_style_set_pad_all(&project_pad, 20);

    lv_obj_t * obj = lv_obj_create(screen);
    lv_obj_remove_style_all(obj);
    return obj;
}

#define PAD(obj) lv_obj_get_style_pad_left(obj, LV_PART_MAIN)

/* Applying two members leaves exactly one: removing the second leaves no pad. */
static void second_member_replaces_first(lv_obj_t * screen)
{
    lv_obj_t * obj = setup(screen);

    trlv_style_group_apply(obj, &pad_group, &pad[0], LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 4);
    trlv_style_group_apply(obj, &pad_group, &pad[1], LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 8);

    lv_obj_remove_style(obj, &pad[1], LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 0);   /* pad[0] is gone, not just hidden */
}

/* Other groups and the project's styles are left alone. */
static void other_styles_untouched(lv_obj_t * screen)
{
    lv_obj_t * obj = setup(screen);

    trlv_style_group_apply(obj, &gap_group, &gap_b, LV_PART_MAIN);
    lv_obj_add_style(obj, &project_bg, LV_PART_MAIN);
    trlv_style_group_apply(obj, &pad_group, &pad[0], LV_PART_MAIN);
    trlv_style_group_apply(obj, &pad_group, &pad[2], LV_PART_MAIN);

    TRLV_ASSERT_INT(PAD(obj), 12);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_row(obj, LV_PART_MAIN), 6);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(obj, LV_PART_MAIN), LV_OPA_COVER);
}

/* A project style added after a modifier still wins after the modifier is
 * called again: the replacement keeps the old member's place. */
static void replacement_keeps_precedence(lv_obj_t * screen)
{
    lv_obj_t * obj = setup(screen);

    trlv_style_group_apply(obj, &pad_group, &pad[0], LV_PART_MAIN);
    lv_obj_add_style(obj, &project_pad, LV_PART_MAIN);
    trlv_style_group_apply(obj, &pad_group, &pad[1], LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 20);

    lv_obj_remove_style(obj, &project_pad, LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 8);
}

/* Applying the member the object already has changes nothing, not even its
 * precedence. */
static void reapplying_same_member_is_a_no_op(lv_obj_t * screen)
{
    lv_obj_t * obj = setup(screen);

    trlv_style_group_apply(obj, &pad_group, &pad[0], LV_PART_MAIN);
    lv_obj_add_style(obj, &project_pad, LV_PART_MAIN);
    trlv_style_group_apply(obj, &pad_group, &pad[0], LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 20);

    lv_obj_remove_style(obj, &project_pad, LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 4);
    lv_obj_remove_style(obj, &pad[0], LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 0);   /* only one copy was there */
}

/* A member added by hand alongside the modifier's is cleaned up too. */
static void extra_members_are_removed(lv_obj_t * screen)
{
    lv_obj_t * obj = setup(screen);

    lv_obj_add_style(obj, &pad[0], LV_PART_MAIN);
    lv_obj_add_style(obj, &pad[1], LV_PART_MAIN);
    trlv_style_group_apply(obj, &pad_group, &pad[2], LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 12);

    lv_obj_remove_style(obj, &pad[2], LV_PART_MAIN);
    TRLV_ASSERT_INT(PAD(obj), 0);
}

/* Selectors are independent: a member on another part is not replaced. */
static void selectors_are_independent(lv_obj_t * screen)
{
    lv_obj_t * obj = setup(screen);

    trlv_style_group_apply(obj, &pad_group, &pad[0], LV_PART_INDICATOR);
    trlv_style_group_apply(obj, &pad_group, &pad[1], LV_PART_MAIN);

    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(obj, LV_PART_INDICATOR), 4);
    TRLV_ASSERT_INT(PAD(obj), 8);
}

/* Removing a group clears every member at that selector, and only those. */
static void remove_clears_the_group(lv_obj_t * screen)
{
    lv_obj_t * obj = setup(screen);

    trlv_style_group_apply(obj, &pad_group, &pad[1], LV_PART_MAIN);
    trlv_style_group_apply(obj, &gap_group, &gap_a, LV_PART_MAIN);
    trlv_style_group_remove(obj, &pad_group, LV_PART_MAIN);

    TRLV_ASSERT_INT(PAD(obj), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_row(obj, LV_PART_MAIN), 2);
}

int main(void)
{
    TRLV_TEST_RUN(second_member_replaces_first);
    TRLV_TEST_RUN(other_styles_untouched);
    TRLV_TEST_RUN(replacement_keeps_precedence);
    TRLV_TEST_RUN(reapplying_same_member_is_a_no_op);
    TRLV_TEST_RUN(extra_members_are_removed);
    TRLV_TEST_RUN(selectors_are_independent);
    TRLV_TEST_RUN(remove_clears_the_group);
    return trlv_test_finish();
}
