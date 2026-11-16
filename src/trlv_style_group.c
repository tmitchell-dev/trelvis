/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_style_group.c
 * Internal: style groups. See trlv_style_group.h.
 *
 * Uses only LVGL's public style API. lv_obj_replace_style keeps the object's
 * style order, which is what keeps a modifier's precedence stable when it is
 * called again. LVGL has no public "does this object have this style?" call,
 * so the slot a member occupies is found by replacing it with an empty
 * placeholder style, then replacing the placeholder with the new member.
 */

#include "trlv_style_group.h"

/* An empty constant style that only ever holds a slot for a moment. */
static const lv_style_const_prop_t placeholder_props[] = {
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(placeholder, placeholder_props);

static const lv_style_t * member(const trlv_style_group_t * group, uint32_t i)
{
    return group->list ? group->list[i] : &group->array[i];
}

/* Replace the first member found with the placeholder and remove the rest.
 * Returns true if the object had a member (now the placeholder). */
static bool take_slot(lv_obj_t * obj, const trlv_style_group_t * group, lv_style_selector_t selector)
{
    bool found = false;

    for(uint32_t i = 0; i < group->count; i++) {
        const lv_style_t * m = member(group, i);
        if(!found) found = lv_obj_replace_style(obj, m, &placeholder, selector);
        else lv_obj_remove_style(obj, m, selector);
    }
    return found;
}

void trlv_style_group_apply(lv_obj_t * obj, const trlv_style_group_t * group,
                            const lv_style_t * style, lv_style_selector_t selector)
{
    LV_ASSERT_NULL(obj);
    LV_ASSERT_NULL(group);
    LV_ASSERT_NULL(style);

    if(take_slot(obj, group, selector)) {
        lv_obj_replace_style(obj, &placeholder, style, selector);
    }
    else {
        lv_obj_add_style(obj, style, selector);
    }
}

void trlv_style_group_remove(lv_obj_t * obj, const trlv_style_group_t * group,
                             lv_style_selector_t selector)
{
    LV_ASSERT_NULL(obj);
    LV_ASSERT_NULL(group);

    for(uint32_t i = 0; i < group->count; i++) {
        lv_obj_remove_style(obj, member(group, i), selector);
    }
}
