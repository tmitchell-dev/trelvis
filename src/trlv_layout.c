/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_layout.c
 * Layout primitives and modifiers.
 */

#include "trelvis/trlv_layout.h"
#include "trelvis/trlv_styles.h"
#include "trlv_internal.h"
#include "trlv_style_group.h"

/*---------------------------------------------------------------------------
 * Sizing styles (constant). Each axis has its own group of three: hug, fill
 * by percentage (cross axis, or no flex parent) and fill by growing (main
 * axis). The two grow styles are identical but separate, so that changing
 * one axis never removes the other axis's style.
 *-------------------------------------------------------------------------*/

static const lv_style_const_prop_t hug_width_props[] = {
    LV_STYLE_CONST_WIDTH(LV_SIZE_CONTENT),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(hug_width, hug_width_props);

static const lv_style_const_prop_t full_width_props[] = {
    LV_STYLE_CONST_WIDTH(LV_PCT(100)),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(full_width, full_width_props);

static const lv_style_const_prop_t grow_width_props[] = {
    LV_STYLE_CONST_FLEX_GROW(1),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(grow_width, grow_width_props);

static const lv_style_const_prop_t hug_height_props[] = {
    LV_STYLE_CONST_HEIGHT(LV_SIZE_CONTENT),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(hug_height, hug_height_props);

static const lv_style_const_prop_t full_height_props[] = {
    LV_STYLE_CONST_HEIGHT(LV_PCT(100)),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(full_height, full_height_props);

static const lv_style_const_prop_t grow_height_props[] = {
    LV_STYLE_CONST_FLEX_GROW(1),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(grow_height, grow_height_props);

static const lv_style_t * const width_styles[] = { &hug_width, &full_width, &grow_width };
static const lv_style_t * const height_styles[] = { &hug_height, &full_height, &grow_height };
static const trlv_style_group_t width_group = TRLV_STYLE_GROUP_LIST(width_styles);
static const trlv_style_group_t height_group = TRLV_STYLE_GROUP_LIST(height_styles);

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

/* Pad: pad_top, pad_bottom, pad_left, pad_right, end marker. */
static lv_style_const_prop_t pad_props[TRLV_SPACE_COUNT][5];

static LV_STYLE_CONST_INIT(pad_none, pad_props[TRLV_SPACE_NONE]);
static LV_STYLE_CONST_INIT(pad_2xs, pad_props[TRLV_SPACE_2XS]);
static LV_STYLE_CONST_INIT(pad_xs, pad_props[TRLV_SPACE_XS]);
static LV_STYLE_CONST_INIT(pad_sm, pad_props[TRLV_SPACE_SM]);
static LV_STYLE_CONST_INIT(pad_md, pad_props[TRLV_SPACE_MD]);
static LV_STYLE_CONST_INIT(pad_lg, pad_props[TRLV_SPACE_LG]);
static LV_STYLE_CONST_INIT(pad_xl, pad_props[TRLV_SPACE_XL]);
static LV_STYLE_CONST_INIT(pad_2xl, pad_props[TRLV_SPACE_2XL]);
static LV_STYLE_CONST_INIT(pad_3xl, pad_props[TRLV_SPACE_3XL]);

/* Indexed by trlv_space_t. */
static const lv_style_t * const pad_styles[] = {
    &pad_none, &pad_2xs, &pad_xs, &pad_sm, &pad_md, &pad_lg, &pad_xl, &pad_2xl, &pad_3xl
};
TRLV_STATIC_ASSERT(sizeof(pad_styles) / sizeof(pad_styles[0]) == TRLV_SPACE_COUNT, pad_styles_cover_the_space_scale);
static const trlv_style_group_t pad_group = TRLV_STYLE_GROUP_LIST(pad_styles);

void trlv_layout_styles_build(const trlv_layout_tokens_t * layout)
{
    for(uint32_t i = 0; i < TRLV_SPACE_COUNT; i++) {
        int32_t space = layout->space[i];

        trlv_prop_set_num(&gap_props[i][0], LV_STYLE_PAD_ROW, space);
        trlv_prop_set_num(&gap_props[i][1], LV_STYLE_PAD_COLUMN, space);
        trlv_prop_set_end(&gap_props[i][2]);

        trlv_prop_set_num(&pad_props[i][0], LV_STYLE_PAD_TOP, space);
        trlv_prop_set_num(&pad_props[i][1], LV_STYLE_PAD_BOTTOM, space);
        trlv_prop_set_num(&pad_props[i][2], LV_STYLE_PAD_LEFT, space);
        trlv_prop_set_num(&pad_props[i][3], LV_STYLE_PAD_RIGHT, space);
        trlv_prop_set_end(&pad_props[i][4]);
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

lv_obj_t * trlv_row(lv_obj_t * parent, trlv_space_t gap)
{
    lv_obj_t * obj = trlv_box(parent);
    lv_obj_add_style(obj, &trlv_style_row, LV_PART_MAIN);
    trlv_gap(obj, gap);
    return obj;
}

lv_obj_t * trlv_column(lv_obj_t * parent, trlv_space_t gap)
{
    lv_obj_t * obj = trlv_box(parent);
    lv_obj_add_style(obj, &trlv_style_column, LV_PART_MAIN);
    trlv_gap(obj, gap);
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

void trlv_pad(lv_obj_t * obj, trlv_space_t space)
{
    LV_ASSERT_NULL(obj);
    if(!space_is_valid(space, "trlv_pad")) return;

    trlv_style_group_apply(obj, &pad_group, pad_styles[space], LV_PART_MAIN);
}

void trlv_hug_width(lv_obj_t * obj)
{
    LV_ASSERT_NULL(obj);
    trlv_style_group_apply(obj, &width_group, &hug_width, LV_PART_MAIN);
}

void trlv_hug_height(lv_obj_t * obj)
{
    LV_ASSERT_NULL(obj);
    trlv_style_group_apply(obj, &height_group, &hug_height, LV_PART_MAIN);
}

/* True if obj's parent is a flex container whose main axis is horizontal
 * (a row) when `horizontal` is true, or vertical (a column) when false. */
static bool parent_main_axis_is(lv_obj_t * obj, bool horizontal)
{
    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(parent == NULL) return false;
    if(lv_obj_get_style_layout(parent, LV_PART_MAIN) != LV_LAYOUT_FLEX) return false;

    bool parent_is_row = (lv_obj_get_style_flex_flow(parent, LV_PART_MAIN) & LV_FLEX_COLUMN) == 0;
    return parent_is_row == horizontal;
}

/* Growing along the main axis of a parent that hugs it always collapses:
 * the parent sizes itself without the growing child, so there is no free
 * space. TreLVis lets LVGL's result stand, but says so, naming the object.
 *
 * Filling across the axis (100 %) of a hugging parent is not warned about:
 * LVGL leaves the child out of the parent's content size, so it stretches
 * to the widest sibling (CSS align-self: stretch), which is a useful pattern.
 *
 * Compiled out unless LVGL logs warnings. */
static void warn_if_growing_in_hug(lv_obj_t * obj, bool horizontal, const char * func)
{
#if LV_USE_LOG && LV_LOG_LEVEL <= LV_LOG_LEVEL_WARN
    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(parent == NULL) return;

    int32_t size = horizontal ? lv_obj_get_style_width(parent, LV_PART_MAIN)
                   : lv_obj_get_style_height(parent, LV_PART_MAIN);
    if(size == LV_SIZE_CONTENT) {
        const char * axis = horizontal ? "width" : "height";
        LV_LOG_WARN("%s(%p): the parent hugs its %s, so there is no space to grow into "
                    "and the object collapses. Give the parent a %s (lv_obj_set_%s or "
                    "trlv_fill_%s) first.",
                    func, (void *)obj, axis, axis, axis, axis);
    }
#else
    LV_UNUSED(obj);
    LV_UNUSED(horizontal);
    LV_UNUSED(func);
#endif
}

void trlv_fill_width(lv_obj_t * obj)
{
    LV_ASSERT_NULL(obj);
    bool grow = parent_main_axis_is(obj, true);
    if(grow) warn_if_growing_in_hug(obj, true, "trlv_fill_width");
    trlv_style_group_apply(obj, &width_group, grow ? &grow_width : &full_width, LV_PART_MAIN);
}

void trlv_fill_height(lv_obj_t * obj)
{
    LV_ASSERT_NULL(obj);
    bool grow = parent_main_axis_is(obj, false);
    if(grow) warn_if_growing_in_hug(obj, false, "trlv_fill_height");
    trlv_style_group_apply(obj, &height_group, grow ? &grow_height : &full_height, LV_PART_MAIN);
}
