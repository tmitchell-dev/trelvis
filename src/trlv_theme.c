/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_theme.c
 * Initialisation and token access. The MD3 theme itself arrives in
 * milestone 4; all theme integration will live in this file.
 */

#include "trelvis/trlv_theme.h"
#include "trlv_internal.h"

static const trlv_color_tokens_t * current_color;
static const trlv_layout_tokens_t * current_layout;

void trlv_init(lv_display_t * disp, const trlv_color_tokens_t * color,
               const trlv_layout_tokens_t * layout)
{
    if(disp == NULL) {
        LV_LOG_WARN("trlv_init: display is NULL; pass the display explicitly");
        return;
    }
    if(color == NULL || layout == NULL) {
        LV_LOG_WARN("trlv_init: color and layout tokens are required");
        return;
    }

    current_color = color;
    current_layout = layout;

    trlv_layout_styles_build(layout);
    trlv_text_styles_build(layout);

    /* Objects that already use a token style pick up the new values. */
    lv_obj_report_style_change(NULL);
}

const trlv_color_tokens_t * trlv_color(void)
{
    return current_color;
}

const trlv_layout_tokens_t * trlv_layout(void)
{
    return current_layout;
}
