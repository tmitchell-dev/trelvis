/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_theme.c
 * Initialisation, token access and the MD3 theme. All theme integration
 * lives in this file, and uses LVGL's public theme functions only.
 */

#include "trelvis/trlv_theme.h"
#include "trlv_internal.h"

#if TRLV_USE_THEME && TRLV_THEME_PARENT == TRLV_THEME_PARENT_DEFAULT && !LV_USE_THEME_DEFAULT
#error "TRLV_THEME_PARENT_DEFAULT needs LV_USE_THEME_DEFAULT 1 in lv_conf.h; or set TRLV_THEME_PARENT to TRLV_THEME_PARENT_NONE in trlv_conf.h"
#endif

static const trlv_color_tokens_t * current_color;
static const trlv_layout_tokens_t * current_layout;

#if TRLV_USE_THEME

/*---------------------------------------------------------------------------
 * Theme styles. Constant ones live in flash; token ones are written by
 * theme_styles_build (see trlv_internal.h for how token styles work).
 *-------------------------------------------------------------------------*/

/* Screen: bg_color, bg_opa, text_color, end marker. The Body Medium text
 * role style is added next to it. */
static lv_style_const_prop_t screen_props[4];
static LV_STYLE_CONST_INIT(screen_style, screen_props);

/* Plain lv_obj: bg_color, border_color, text_color, end marker. The parent
 * theme gives it its shape (radius, border width, padding). Boxes, rows and
 * columns remove all theme styles, so this never reaches them. */
static lv_style_const_prop_t obj_props[4];
static LV_STYLE_CONST_INIT(obj_style, obj_props);

/* Filled button, token values: bg_color, bg_opa, text_color, radius,
 * height, pad_left, pad_right, pad_column, end marker. The Label Large text
 * role style is added next to it. */
static lv_style_const_prop_t button_props[9];
static LV_STYLE_CONST_INIT(button_style, button_props);

/* Filled button, constant: content centred in a row (an icon and a label
 * sit side by side, as in an MD3 button), no vertical padding (the height
 * is fixed) and no shadow (MD3 filled buttons are at elevation level 0). */
static const lv_style_const_prop_t button_layout_props[] = {
    LV_STYLE_CONST_LAYOUT(LV_LAYOUT_FLEX),
    LV_STYLE_CONST_FLEX_FLOW(LV_FLEX_FLOW_ROW),
    LV_STYLE_CONST_FLEX_MAIN_PLACE(LV_FLEX_ALIGN_CENTER),
    LV_STYLE_CONST_FLEX_CROSS_PLACE(LV_FLEX_ALIGN_CENTER),
    LV_STYLE_CONST_FLEX_TRACK_PLACE(LV_FLEX_ALIGN_CENTER),
    LV_STYLE_CONST_PAD_TOP(0),
    LV_STYLE_CONST_PAD_BOTTOM(0),
    LV_STYLE_CONST_SHADOW_WIDTH(0),
    LV_STYLE_CONST_SHADOW_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(button_layout_style, button_layout_props);

static void prop_set_color(lv_style_const_prop_t * entry, lv_style_prop_t prop, lv_color_t color)
{
    entry->prop = prop;
    entry->value.color = color;
}

static void theme_styles_build(const trlv_color_tokens_t * color, const trlv_layout_tokens_t * layout)
{
    prop_set_color(&screen_props[0], LV_STYLE_BG_COLOR, color->surface);
    trlv_prop_set_num(&screen_props[1], LV_STYLE_BG_OPA, LV_OPA_COVER);
    prop_set_color(&screen_props[2], LV_STYLE_TEXT_COLOR, color->on_surface);
    trlv_prop_set_end(&screen_props[3]);

    prop_set_color(&obj_props[0], LV_STYLE_BG_COLOR, color->surface);
    prop_set_color(&obj_props[1], LV_STYLE_BORDER_COLOR, color->outline_variant);
    prop_set_color(&obj_props[2], LV_STYLE_TEXT_COLOR, color->on_surface);
    trlv_prop_set_end(&obj_props[3]);

    /* MD3 filled button: 24 px side padding (space XL), 8 px between an
     * icon and the label (space SM). */
    prop_set_color(&button_props[0], LV_STYLE_BG_COLOR, color->primary);
    trlv_prop_set_num(&button_props[1], LV_STYLE_BG_OPA, LV_OPA_COVER);
    prop_set_color(&button_props[2], LV_STYLE_TEXT_COLOR, color->on_primary);
    trlv_prop_set_num(&button_props[3], LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_FULL]);
    trlv_prop_set_num(&button_props[4], LV_STYLE_HEIGHT, layout->control_height[TRLV_SIZE_MD]);
    trlv_prop_set_num(&button_props[5], LV_STYLE_PAD_LEFT, layout->space[TRLV_SPACE_XL]);
    trlv_prop_set_num(&button_props[6], LV_STYLE_PAD_RIGHT, layout->space[TRLV_SPACE_XL]);
    trlv_prop_set_num(&button_props[7], LV_STYLE_PAD_COLUMN, layout->space[TRLV_SPACE_SM]);
    trlv_prop_set_end(&button_props[8]);
}

/*---------------------------------------------------------------------------
 * The theme
 *-------------------------------------------------------------------------*/

/* Its address marks a theme as the TreLVis theme (in the theme's user data),
 * so trlv_init can find and reuse it instead of creating another. */
static char theme_marker;

/* Styles each object when it is created, after the parent theme. Labels get
 * nothing: they inherit their text role and colour from what they sit in. */
static void theme_apply(lv_theme_t * theme, lv_obj_t * obj)
{
    LV_UNUSED(theme);

    if(lv_obj_get_parent(obj) == NULL) {
        lv_obj_add_style(obj, &screen_style, LV_PART_MAIN);
        lv_obj_add_style(obj, trlv_text_role_style(TRLV_TEXT_BODY_MEDIUM), LV_PART_MAIN);
    }
    else if(lv_obj_check_type(obj, &lv_obj_class)) {
        lv_obj_add_style(obj, &obj_style, LV_PART_MAIN);
    }
    else if(lv_obj_check_type(obj, &lv_button_class)) {
        lv_obj_add_style(obj, &button_style, LV_PART_MAIN);
        lv_obj_add_style(obj, &button_layout_style, LV_PART_MAIN);
        lv_obj_add_style(obj, trlv_text_role_style(TRLV_TEXT_LABEL_LARGE), LV_PART_MAIN);
    }
}

#if TRLV_THEME_PARENT == TRLV_THEME_PARENT_DEFAULT
/* LVGL's default theme needs a dark flag: dark when `surface` is below 50 %
 * luminance. */
static bool is_dark(const trlv_color_tokens_t * color)
{
    return lv_color_luminance(color->surface) < 128;
}
#endif

static void theme_install(lv_display_t * disp, const trlv_color_tokens_t * color,
                          const trlv_layout_tokens_t * layout)
{
    lv_theme_t * parent = NULL;

#if TRLV_THEME_PARENT == TRLV_THEME_PARENT_DEFAULT
    const lv_font_t * body_font = layout->text[TRLV_TEXT_BODY_MEDIUM].font;
    if(body_font == NULL) body_font = LV_FONT_DEFAULT;

    parent = lv_theme_default_init(disp, color->primary, color->secondary, is_dark(color), body_font);
    if(parent == NULL) {
        LV_LOG_WARN("trlv_init: LVGL's default theme could not be initialised");
    }
#else
    LV_UNUSED(color);
    LV_UNUSED(layout);
#endif

    /* Reuse the TreLVis theme if the display already has it (trlv_init
     * called again). After lv_deinit and lv_init the display is new and has
     * LVGL's theme, so a new one is created in the new LVGL heap. */
    lv_theme_t * theme = lv_display_get_theme(disp);
    if(theme == NULL || lv_theme_get_user_data(theme) != &theme_marker) {
        theme = lv_theme_create();
        if(theme == NULL) {
            LV_LOG_WARN("trlv_init: out of memory for the theme; no theme installed");
            return;
        }
    }

    /* The parent's colours and fonts are what lv_theme_get_color_primary()
     * and lv_theme_get_font_normal() return for this theme. */
    if(parent != NULL) lv_theme_copy(theme, parent);
    lv_theme_set_parent(theme, parent);
    lv_theme_set_apply_cb(theme, theme_apply);
    lv_theme_set_user_data(theme, &theme_marker);

    lv_display_set_theme(disp, theme);

    /* The display's first screen exists before trlv_init can run, so it
     * still has LVGL's theme styles. lv_display_set_theme restyles only the
     * display's first screen object, which in LVGL 9.6 is the bottom layer,
     * so restyle the active screen here while it is still empty. */
    lv_obj_t * screen = lv_display_get_screen_active(disp);
    if(screen != NULL && lv_obj_get_child_count(screen) == 0) {
        lv_theme_apply(screen);
    }
}

#endif /* TRLV_USE_THEME */

/*---------------------------------------------------------------------------
 * Public API
 *-------------------------------------------------------------------------*/

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

#if TRLV_USE_THEME
    theme_styles_build(color, layout);
    theme_install(disp, color, layout);
#endif

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
