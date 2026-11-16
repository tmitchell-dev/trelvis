/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_layout.c
 * Layout primitives and modifiers (provisional milestone 2 slice).
 */

#include "trelvis/trlv_layout.h"
#include "trelvis/trlv_styles.h"
#include "trlv_internal.h"
#include "trlv_style_group.h"

/*---------------------------------------------------------------------------
 * Token styles: one per space step, written by trlv_layout_styles_build.
 * See trlv_internal.h for how token styles work.
 *-------------------------------------------------------------------------*/

/* Gap: pad_row, pad_column, end marker. */
static lv_style_const_prop_t gap_props[TRLV_SPACE_COUNT][3];

static LV_STYLE_CONST_INIT(gap_none, gap_props[TRLV_SPACE_NONE]);
static LV_STYLE_CONST_INIT(gap_2xs, gap_props[TRLV_SPACE_2XS]);
static LV_STYLE_CONST_INIT(gap_xs, gap_props[TRLV_SPACE_XS]);
static LV_STYLE_CONST_INIT(gap_sm, gap_props[TRLV_SPACE_SM]);
static LV_STYLE_CONST_INIT(gap_md, gap_props[TRLV_SPACE_MD]);
static LV_STYLE_CONST_INIT(gap_lg, gap_props[TRLV_SPACE_LG]);
static LV_STYLE_CONST_INIT(gap_xl, gap_props[TRLV_SPACE_XL]);
static LV_STYLE_CONST_INIT(gap_2xl, gap_props[TRLV_SPACE_2XL]);
static LV_STYLE_CONST_INIT(gap_3xl, gap_props[TRLV_SPACE_3XL]);

/* Indexed by trlv_space_t. */
static const lv_style_t * const gap_styles[] = {
    &gap_none, &gap_2xs, &gap_xs, &gap_sm, &gap_md, &gap_lg, &gap_xl, &gap_2xl, &gap_3xl
};
TRLV_STATIC_ASSERT(sizeof(gap_styles) / sizeof(gap_styles[0]) == TRLV_SPACE_COUNT, gap_styles_cover_the_space_scale);
static const trlv_style_group_t gap_group = TRLV_STYLE_GROUP_LIST(gap_styles);

void trlv_layout_styles_build(const trlv_layout_tokens_t * layout)
{
    for(uint32_t i = 0; i < TRLV_SPACE_COUNT; i++) {
        int32_t space = layout->space[i];

        trlv_prop_set_num(&gap_props[i][0], LV_STYLE_PAD_ROW, space);
        trlv_prop_set_num(&gap_props[i][1], LV_STYLE_PAD_COLUMN, space);
        trlv_prop_set_end(&gap_props[i][2]);
    }
}

/* True if `space` is a step of the scale; logs a warning naming `func` if not. */
static bool space_is_valid(trlv_space_t space, const char * func)
{
    if((uint32_t)space < TRLV_SPACE_COUNT) return true;

    LV_LOG_WARN("%s: %d is not a trlv_space_t step", func, (int)space);
    LV_UNUSED(func);
    return false;
}

/*---------------------------------------------------------------------------
 * Constructors
 *-------------------------------------------------------------------------*/

lv_obj_t * trlv_box(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_add_style(obj, &trlv_style_box, LV_PART_MAIN);
    lv_obj_set_scrollable(obj, false);
    lv_obj_set_clickable(obj, false);
    return obj;
}

/*---------------------------------------------------------------------------
 * Modifiers
 *-------------------------------------------------------------------------*/

void trlv_gap(lv_obj_t * obj, trlv_space_t space)
{
    LV_ASSERT_NULL(obj);
    if(!space_is_valid(space, "trlv_gap")) return;

    trlv_style_group_apply(obj, &gap_group, gap_styles[space], LV_PART_MAIN);
}
