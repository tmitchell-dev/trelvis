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
 * Sizing styles (constant). Each axis has its own group of three: fit, fill
 * by percentage (cross axis, or no flex parent) and fill by growing (main
 * axis). The two grow styles are identical but separate, so that changing
 * one axis never removes the other axis's style.
 *-------------------------------------------------------------------------*/

static const lv_style_const_prop_t fit_width_props[] = {
    LV_STYLE_CONST_WIDTH(LV_SIZE_CONTENT),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(fit_width, fit_width_props);

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

static const lv_style_const_prop_t fit_height_props[] = {
    LV_STYLE_CONST_HEIGHT(LV_SIZE_CONTENT),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(fit_height, fit_height_props);

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

static const lv_style_t * const width_styles[] = { &fit_width, &full_width, &grow_width };
static const lv_style_t * const height_styles[] = { &fit_height, &full_height, &grow_height };
static const trlv_style_group_t width_group = TRLV_STYLE_GROUP_LIST(width_styles);
static const trlv_style_group_t height_group = TRLV_STYLE_GROUP_LIST(height_styles);

/*---------------------------------------------------------------------------
 * Flex placement styles (constant): one per value, so each modifier is one
 * group. trlv_justify_t and trlv_align_items_t equal lv_flex_align_t, but
 * the styles are picked through a switch rather than indexed by value, which
 * keeps them independent of LVGL's numbering.
 *-------------------------------------------------------------------------*/

#define TRLV_PLACE_STYLE(name_, prop_, value_)                \
    static const lv_style_const_prop_t name_##_props[] = {    \
        LV_STYLE_CONST_##prop_(value_),                       \
        LV_STYLE_CONST_PROPS_END                              \
    };                                                        \
    static LV_STYLE_CONST_INIT(name_, name_##_props)

TRLV_PLACE_STYLE(justify_start, FLEX_MAIN_PLACE, LV_FLEX_ALIGN_START);
TRLV_PLACE_STYLE(justify_center, FLEX_MAIN_PLACE, LV_FLEX_ALIGN_CENTER);
TRLV_PLACE_STYLE(justify_end, FLEX_MAIN_PLACE, LV_FLEX_ALIGN_END);
TRLV_PLACE_STYLE(justify_space_between, FLEX_MAIN_PLACE, LV_FLEX_ALIGN_SPACE_BETWEEN);
TRLV_PLACE_STYLE(justify_space_around, FLEX_MAIN_PLACE, LV_FLEX_ALIGN_SPACE_AROUND);
TRLV_PLACE_STYLE(justify_space_evenly, FLEX_MAIN_PLACE, LV_FLEX_ALIGN_SPACE_EVENLY);

/* Align items sets the track placement too. LVGL places items within their
 * line (track), which is only as deep as the deepest item and sits at the
 * start of the container, so cross_place alone would centre a short item
 * against its tallest sibling rather than in the row. Placing the line the
 * same way gives Figma's and CSS's result. */
#define TRLV_ALIGN_ITEMS_STYLE(name_, value_)                 \
    static const lv_style_const_prop_t name_##_props[] = {    \
        LV_STYLE_CONST_FLEX_CROSS_PLACE(value_),              \
        LV_STYLE_CONST_FLEX_TRACK_PLACE(value_),              \
        LV_STYLE_CONST_PROPS_END                              \
    };                                                        \
    static LV_STYLE_CONST_INIT(name_, name_##_props)

TRLV_ALIGN_ITEMS_STYLE(align_items_start, LV_FLEX_ALIGN_START);
TRLV_ALIGN_ITEMS_STYLE(align_items_center, LV_FLEX_ALIGN_CENTER);
TRLV_ALIGN_ITEMS_STYLE(align_items_end, LV_FLEX_ALIGN_END);

static const lv_style_t * const justify_styles[] = {
    &justify_start, &justify_center, &justify_end,
    &justify_space_between, &justify_space_around, &justify_space_evenly
};
static const trlv_style_group_t justify_group = TRLV_STYLE_GROUP_LIST(justify_styles);

static const lv_style_t * const align_items_styles[] = {
    &align_items_start, &align_items_center, &align_items_end
};
static const trlv_style_group_t align_items_group = TRLV_STYLE_GROUP_LIST(align_items_styles);

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

/* Padding is two groups, one per axis, so trlv_set_pad (both) and the
 * per-axis setters replace each other per axis, in any order, like CSS
 * `padding` and `padding-inline`. */

/* Horizontal padding: pad_left, pad_right, end marker. */
static lv_style_const_prop_t pad_hor_props[TRLV_SPACE_COUNT][3];

static LV_STYLE_CONST_INIT(pad_hor_none, pad_hor_props[TRLV_SPACE_NONE]);
static LV_STYLE_CONST_INIT(pad_hor_2xs, pad_hor_props[TRLV_SPACE_2XS]);
static LV_STYLE_CONST_INIT(pad_hor_xs, pad_hor_props[TRLV_SPACE_XS]);
static LV_STYLE_CONST_INIT(pad_hor_sm, pad_hor_props[TRLV_SPACE_SM]);
static LV_STYLE_CONST_INIT(pad_hor_md, pad_hor_props[TRLV_SPACE_MD]);
static LV_STYLE_CONST_INIT(pad_hor_lg, pad_hor_props[TRLV_SPACE_LG]);
static LV_STYLE_CONST_INIT(pad_hor_xl, pad_hor_props[TRLV_SPACE_XL]);
static LV_STYLE_CONST_INIT(pad_hor_2xl, pad_hor_props[TRLV_SPACE_2XL]);
static LV_STYLE_CONST_INIT(pad_hor_3xl, pad_hor_props[TRLV_SPACE_3XL]);

/* Indexed by trlv_space_t. */
static const lv_style_t * const pad_hor_styles[] = {
    &pad_hor_none, &pad_hor_2xs, &pad_hor_xs, &pad_hor_sm, &pad_hor_md, &pad_hor_lg, &pad_hor_xl, &pad_hor_2xl, &pad_hor_3xl
};
TRLV_STATIC_ASSERT(sizeof(pad_hor_styles) / sizeof(pad_hor_styles[0]) == TRLV_SPACE_COUNT, pad_hor_styles_cover_the_space_scale);
static const trlv_style_group_t pad_hor_group = TRLV_STYLE_GROUP_LIST(pad_hor_styles);

/* Vertical padding: pad_top, pad_bottom, end marker. */
static lv_style_const_prop_t pad_ver_props[TRLV_SPACE_COUNT][3];

static LV_STYLE_CONST_INIT(pad_ver_none, pad_ver_props[TRLV_SPACE_NONE]);
static LV_STYLE_CONST_INIT(pad_ver_2xs, pad_ver_props[TRLV_SPACE_2XS]);
static LV_STYLE_CONST_INIT(pad_ver_xs, pad_ver_props[TRLV_SPACE_XS]);
static LV_STYLE_CONST_INIT(pad_ver_sm, pad_ver_props[TRLV_SPACE_SM]);
static LV_STYLE_CONST_INIT(pad_ver_md, pad_ver_props[TRLV_SPACE_MD]);
static LV_STYLE_CONST_INIT(pad_ver_lg, pad_ver_props[TRLV_SPACE_LG]);
static LV_STYLE_CONST_INIT(pad_ver_xl, pad_ver_props[TRLV_SPACE_XL]);
static LV_STYLE_CONST_INIT(pad_ver_2xl, pad_ver_props[TRLV_SPACE_2XL]);
static LV_STYLE_CONST_INIT(pad_ver_3xl, pad_ver_props[TRLV_SPACE_3XL]);

/* Indexed by trlv_space_t. */
static const lv_style_t * const pad_ver_styles[] = {
    &pad_ver_none, &pad_ver_2xs, &pad_ver_xs, &pad_ver_sm, &pad_ver_md, &pad_ver_lg, &pad_ver_xl, &pad_ver_2xl, &pad_ver_3xl
};
TRLV_STATIC_ASSERT(sizeof(pad_ver_styles) / sizeof(pad_ver_styles[0]) == TRLV_SPACE_COUNT, pad_ver_styles_cover_the_space_scale);
static const trlv_style_group_t pad_ver_group = TRLV_STYLE_GROUP_LIST(pad_ver_styles);

void trlv_layout_styles_build(const trlv_layout_tokens_t * layout)
{
    for(uint32_t i = 0; i < TRLV_SPACE_COUNT; i++) {
        int32_t space = layout->space[i];

        trlv_prop_set_num(&gap_props[i][0], LV_STYLE_PAD_ROW, space);
        trlv_prop_set_num(&gap_props[i][1], LV_STYLE_PAD_COLUMN, space);
        trlv_prop_set_end(&gap_props[i][2]);

        trlv_prop_set_num(&pad_hor_props[i][0], LV_STYLE_PAD_LEFT, space);
        trlv_prop_set_num(&pad_hor_props[i][1], LV_STYLE_PAD_RIGHT, space);
        trlv_prop_set_end(&pad_hor_props[i][2]);

        trlv_prop_set_num(&pad_ver_props[i][0], LV_STYLE_PAD_TOP, space);
        trlv_prop_set_num(&pad_ver_props[i][1], LV_STYLE_PAD_BOTTOM, space);
        trlv_prop_set_end(&pad_ver_props[i][2]);
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

lv_obj_t * trlv_box_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_add_style(obj, &trlv_style_box, LV_PART_MAIN);
    lv_obj_set_scrollable(obj, false);
    lv_obj_set_clickable(obj, false);
    return obj;
}

lv_obj_t * trlv_row_create(lv_obj_t * parent)
{
    lv_obj_t * obj = trlv_box_create(parent);
    lv_obj_add_style(obj, &trlv_style_row, LV_PART_MAIN);
    return obj;
}

lv_obj_t * trlv_column_create(lv_obj_t * parent)
{
    lv_obj_t * obj = trlv_box_create(parent);
    lv_obj_add_style(obj, &trlv_style_column, LV_PART_MAIN);
    return obj;
}

/* Defined with the modifiers below. */
static bool parent_main_axis_is(lv_obj_t * obj, bool horizontal);
static void fill(lv_obj_t * obj, bool horizontal, const char * func);

lv_obj_t * trlv_spacer_create(lv_obj_t * parent)
{
    lv_obj_t * obj = trlv_box_create(parent);

    /* An empty box fits to 0 x 0. Fill only along the main axis, so it takes
     * no room across it; with no flex parent it stays 0 x 0. */
    if(parent_main_axis_is(obj, true)) fill(obj, true, "trlv_spacer_create");
    else if(parent_main_axis_is(obj, false)) fill(obj, false, "trlv_spacer_create");
    return obj;
}

lv_obj_t * trlv_divider_create(lv_obj_t * parent)
{
    lv_obj_t * obj = trlv_box_create(parent);

    /* Vertical in a row, horizontal otherwise. The line fills across the
     * parent's main axis (100 %), so it never grows along it. */
    bool vertical = parent_main_axis_is(obj, true);
    lv_obj_add_style(obj, trlv_divider_style(vertical), LV_PART_MAIN);
    fill(obj, !vertical, "trlv_divider_create");
    return obj;
}

/*---------------------------------------------------------------------------
 * Modifiers
 *-------------------------------------------------------------------------*/

void trlv_set_gap(lv_obj_t * obj, trlv_space_t space)
{
    LV_ASSERT_NULL(obj);
    if(!space_is_valid(space, "trlv_set_gap")) return;

    trlv_style_group_apply(obj, &gap_group, gap_styles[space], LV_PART_MAIN);
}

void trlv_set_pad(lv_obj_t * obj, trlv_space_t space)
{
    LV_ASSERT_NULL(obj);
    if(!space_is_valid(space, "trlv_set_pad")) return;

    trlv_style_group_apply(obj, &pad_hor_group, pad_hor_styles[space], LV_PART_MAIN);
    trlv_style_group_apply(obj, &pad_ver_group, pad_ver_styles[space], LV_PART_MAIN);
}

void trlv_set_width_fit(lv_obj_t * obj)
{
    LV_ASSERT_NULL(obj);
    trlv_style_group_apply(obj, &width_group, &fit_width, LV_PART_MAIN);
}

void trlv_set_height_fit(lv_obj_t * obj)
{
    LV_ASSERT_NULL(obj);
    trlv_style_group_apply(obj, &height_group, &fit_height, LV_PART_MAIN);
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

/* Growing along the main axis of a parent that fits that axis to its
 * content always collapses: the parent sizes itself without the growing child, so there is no free
 * space. TreLVis lets LVGL's result stand, but says so, naming the object.
 *
 * Filling across the axis (100 %) of a fit-content parent is not warned about:
 * LVGL leaves the child out of the parent's content size, so it stretches
 * to the widest sibling (CSS align-self: stretch), which is a useful pattern.
 *
 * Compiled out unless LVGL logs warnings. */
static void warn_if_growing_in_fit_parent(lv_obj_t * obj, bool horizontal, const char * func)
{
#if LV_USE_LOG && LV_LOG_LEVEL <= LV_LOG_LEVEL_WARN
    lv_obj_t * parent = lv_obj_get_parent(obj);
    if(parent == NULL) return;

    int32_t size = horizontal ? lv_obj_get_style_width(parent, LV_PART_MAIN)
                   : lv_obj_get_style_height(parent, LV_PART_MAIN);
    if(size == LV_SIZE_CONTENT) {
        const char * axis = horizontal ? "width" : "height";
        LV_LOG_WARN("%s(%p): the parent fits its %s to its content, so there is no space "
                    "to grow into and the object collapses. Give the parent a %s (lv_obj_set_%s or "
                    "trlv_set_%s_fill) first.",
                    func, (void *)obj, axis, axis, axis, axis);
    }
#else
    LV_UNUSED(obj);
    LV_UNUSED(horizontal);
    LV_UNUSED(func);
#endif
}

/* Axis-aware fill on one axis; `func` names the public caller in the warning. */
static void fill(lv_obj_t * obj, bool horizontal, const char * func)
{
    bool grow = parent_main_axis_is(obj, horizontal);
    if(grow) warn_if_growing_in_fit_parent(obj, horizontal, func);

    if(horizontal) {
        trlv_style_group_apply(obj, &width_group, grow ? &grow_width : &full_width, LV_PART_MAIN);
    }
    else {
        trlv_style_group_apply(obj, &height_group, grow ? &grow_height : &full_height, LV_PART_MAIN);
    }
}

void trlv_set_width_fill(lv_obj_t * obj)
{
    LV_ASSERT_NULL(obj);
    fill(obj, true, "trlv_set_width_fill");
}

void trlv_set_height_fill(lv_obj_t * obj)
{
    LV_ASSERT_NULL(obj);
    fill(obj, false, "trlv_set_height_fill");
}

void trlv_set_justify(lv_obj_t * obj, trlv_justify_t justify)
{
    LV_ASSERT_NULL(obj);

    const lv_style_t * style;
    switch(justify) {
        case TRLV_JUSTIFY_START:         style = &justify_start; break;
        case TRLV_JUSTIFY_CENTER:        style = &justify_center; break;
        case TRLV_JUSTIFY_END:           style = &justify_end; break;
        case TRLV_JUSTIFY_SPACE_BETWEEN: style = &justify_space_between; break;
        case TRLV_JUSTIFY_SPACE_AROUND:  style = &justify_space_around; break;
        case TRLV_JUSTIFY_SPACE_EVENLY:  style = &justify_space_evenly; break;
        default:
            LV_LOG_WARN("trlv_set_justify: %d is not a trlv_justify_t value", (int)justify);
            return;
    }
    trlv_style_group_apply(obj, &justify_group, style, LV_PART_MAIN);
}

void trlv_set_align_items(lv_obj_t * obj, trlv_align_items_t align)
{
    LV_ASSERT_NULL(obj);

    const lv_style_t * style;
    switch(align) {
        case TRLV_ALIGN_ITEMS_START:  style = &align_items_start; break;
        case TRLV_ALIGN_ITEMS_CENTER: style = &align_items_center; break;
        case TRLV_ALIGN_ITEMS_END:    style = &align_items_end; break;
        default:
            LV_LOG_WARN("trlv_set_align_items: %d is not a trlv_align_items_t value", (int)align);
            return;
    }
    trlv_style_group_apply(obj, &align_items_group, style, LV_PART_MAIN);
}
