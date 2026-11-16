/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_layout.c
 * Layout primitives and modifiers (provisional milestone 2 slice).
 */

#include "trelvis/trlv_layout.h"
#include "trelvis/trlv_styles.h"
#include "trlv_internal.h"

/*---------------------------------------------------------------------------
 * Token styles
 *-------------------------------------------------------------------------*/

void trlv_layout_styles_build(const trlv_layout_tokens_t * layout)
{
    /* The gap and pad styles arrive with #12 and #13. */
    (void)layout;
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
