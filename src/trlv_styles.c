/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_styles.c
 * The constant layout styles in trlv_styles.h. They are LV_STYLE_CONST_INIT
 * styles, so they live in flash and use no RAM.
 */

#include "trelvis/trlv_styles.h"

static const lv_style_const_prop_t box_props[] = {
    LV_STYLE_CONST_BG_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_BORDER_WIDTH(0),
    LV_STYLE_CONST_PAD_TOP(0),
    LV_STYLE_CONST_PAD_BOTTOM(0),
    LV_STYLE_CONST_PAD_LEFT(0),
    LV_STYLE_CONST_PAD_RIGHT(0),
    LV_STYLE_CONST_WIDTH(LV_SIZE_CONTENT),
    LV_STYLE_CONST_HEIGHT(LV_SIZE_CONTENT),
    LV_STYLE_CONST_PROPS_END
};
LV_STYLE_CONST_INIT(trlv_style_box, box_props);

static const lv_style_const_prop_t row_props[] = {
    LV_STYLE_CONST_LAYOUT(LV_LAYOUT_FLEX),
    LV_STYLE_CONST_FLEX_FLOW(LV_FLEX_FLOW_ROW),
    LV_STYLE_CONST_PROPS_END
};
LV_STYLE_CONST_INIT(trlv_style_row, row_props);

static const lv_style_const_prop_t column_props[] = {
    LV_STYLE_CONST_LAYOUT(LV_LAYOUT_FLEX),
    LV_STYLE_CONST_FLEX_FLOW(LV_FLEX_FLOW_COLUMN),
    LV_STYLE_CONST_PROPS_END
};
LV_STYLE_CONST_INIT(trlv_style_column, column_props);
