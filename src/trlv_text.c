/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_text.c
 * Text primitive and the text role styles.
 * All label and text specifics live in this file.
 */

#include "trelvis/trlv_text.h"
#include "trlv_internal.h"
#include "trlv_style_group.h"

/*---------------------------------------------------------------------------
 * Token styles: one per role, written by trlv_text_styles_build.
 * See trlv_internal.h for how token styles work.
 *-------------------------------------------------------------------------*/

/* text_font, text_line_space, text_letter_space, end marker. */
static lv_style_const_prop_t text_props[TRLV_TEXT_COUNT][4];

static LV_STYLE_CONST_INIT(text_display_large, text_props[TRLV_TEXT_DISPLAY_LARGE]);
static LV_STYLE_CONST_INIT(text_display_medium, text_props[TRLV_TEXT_DISPLAY_MEDIUM]);
static LV_STYLE_CONST_INIT(text_display_small, text_props[TRLV_TEXT_DISPLAY_SMALL]);
static LV_STYLE_CONST_INIT(text_headline_large, text_props[TRLV_TEXT_HEADLINE_LARGE]);
static LV_STYLE_CONST_INIT(text_headline_medium, text_props[TRLV_TEXT_HEADLINE_MEDIUM]);
static LV_STYLE_CONST_INIT(text_headline_small, text_props[TRLV_TEXT_HEADLINE_SMALL]);
static LV_STYLE_CONST_INIT(text_title_large, text_props[TRLV_TEXT_TITLE_LARGE]);
static LV_STYLE_CONST_INIT(text_title_medium, text_props[TRLV_TEXT_TITLE_MEDIUM]);
static LV_STYLE_CONST_INIT(text_title_small, text_props[TRLV_TEXT_TITLE_SMALL]);
static LV_STYLE_CONST_INIT(text_body_large, text_props[TRLV_TEXT_BODY_LARGE]);
static LV_STYLE_CONST_INIT(text_body_medium, text_props[TRLV_TEXT_BODY_MEDIUM]);
static LV_STYLE_CONST_INIT(text_body_small, text_props[TRLV_TEXT_BODY_SMALL]);
static LV_STYLE_CONST_INIT(text_label_large, text_props[TRLV_TEXT_LABEL_LARGE]);
static LV_STYLE_CONST_INIT(text_label_medium, text_props[TRLV_TEXT_LABEL_MEDIUM]);
static LV_STYLE_CONST_INIT(text_label_small, text_props[TRLV_TEXT_LABEL_SMALL]);

/* Indexed by trlv_text_role_t. One group, so a later role (trlv_text_role,
 * milestone 5) replaces the earlier one. */
static const lv_style_t * const text_styles[] = {
    &text_display_large, &text_display_medium, &text_display_small,
    &text_headline_large, &text_headline_medium, &text_headline_small,
    &text_title_large, &text_title_medium, &text_title_small,
    &text_body_large, &text_body_medium, &text_body_small,
    &text_label_large, &text_label_medium, &text_label_small
};
TRLV_STATIC_ASSERT(sizeof(text_styles) / sizeof(text_styles[0]) == TRLV_TEXT_COUNT, text_styles_cover_the_type_scale);
static const trlv_style_group_t text_group = TRLV_STYLE_GROUP_LIST(text_styles);

void trlv_text_styles_build(const trlv_layout_tokens_t * layout)
{
    for(uint32_t i = 0; i < TRLV_TEXT_COUNT; i++) {
        const trlv_text_style_t * token = &layout->text[i];
        const lv_font_t * font = token->font;

        if(font == NULL) {
            LV_LOG_WARN("trlv_init: text role %d has no font; using LV_FONT_DEFAULT", (int)i);
            font = LV_FONT_DEFAULT;
        }

        /* Figma line height -> LVGL line spacing; 0 keeps the font's own. */
        int32_t line_space = token->line_height == 0 ? 0
                             : token->line_height - lv_font_get_line_height(font);

        trlv_prop_set_ptr(&text_props[i][0], LV_STYLE_TEXT_FONT, font);
        trlv_prop_set_num(&text_props[i][1], LV_STYLE_TEXT_LINE_SPACE, line_space);
        trlv_prop_set_num(&text_props[i][2], LV_STYLE_TEXT_LETTER_SPACE, token->letter_spacing);
        trlv_prop_set_end(&text_props[i][3]);
    }
}

/*---------------------------------------------------------------------------
 * Constructor
 *-------------------------------------------------------------------------*/

lv_obj_t * trlv_text(lv_obj_t * parent, const char * text, trlv_text_role_t role)
{
    lv_obj_t * obj = lv_label_create(parent);
    lv_label_set_text(obj, text);

    if((uint32_t)role < TRLV_TEXT_COUNT) {
        trlv_style_group_apply(obj, &text_group, text_styles[role], LV_PART_MAIN);
    }
    else {
        LV_LOG_WARN("trlv_text: %d is not a trlv_text_role_t", (int)role);
    }
    return obj;
}
