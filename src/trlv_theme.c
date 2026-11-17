/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_theme.c
 * Initialisation, token access, colour-mode switching and the MD3 theme.
 * All theme integration lives in this file, and uses LVGL's public theme
 * functions only.
 */

#include "trelvis/trlv_theme.h"
#include "trlv_internal.h"

#if TRLV_USE_THEME && TRLV_THEME_PARENT == TRLV_THEME_PARENT_DEFAULT && !LV_USE_THEME_DEFAULT
#error "TRLV_THEME_PARENT_DEFAULT needs LV_USE_THEME_DEFAULT 1 in lv_conf.h; or set TRLV_THEME_PARENT to TRLV_THEME_PARENT_NONE in trlv_conf.h"
#endif

static const trlv_color_tokens_t * current_color;
static const trlv_layout_tokens_t * current_layout;

/* The display trlv_init installed the theme on, for colour-mode switches. */
static lv_display_t * current_display;

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

/*---------------------------------------------------------------------------
 * State styles. Each interactive widget has its own pressed and focus
 * styles (the state layer depends on its colour pair, see
 * trlv_state_layer); the focus ring and the disabled treatment are shared.
 * Each also switches off what LVGL's default theme does in that state
 * (a recolour that tints the content too, and a grow on press).
 *-------------------------------------------------------------------------*/

/* Filled button pressed: bg_color (state layer), end marker. */
static lv_style_const_prop_t button_pressed_props[2];
static LV_STYLE_CONST_INIT(button_pressed_style, button_pressed_props);

/* Filled button focused by keypad or encoder: bg_color (state layer), end
 * marker. The focus ring style is added next to it. */
static lv_style_const_prop_t button_focus_props[2];
static LV_STYLE_CONST_INIT(button_focus_style, button_focus_props);

/* Pressed, for every widget: no recolour, no grow. */
static const lv_style_const_prop_t pressed_reset_props[] = {
    LV_STYLE_CONST_RECOLOR_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_TRANSFORM_WIDTH(0),
    LV_STYLE_CONST_TRANSFORM_HEIGHT(0),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(pressed_reset_style, pressed_reset_props);

/* Focus ring, for every widget: outline_width, outline_color, outline_opa,
 * outline_pad, end marker. MD3: a focus indicator 3 px thick, 2 px outside
 * the widget (`focus_width`, space 2XS). */
static lv_style_const_prop_t focus_ring_props[5];
static LV_STYLE_CONST_INIT(focus_ring_style, focus_ring_props);

/* Disabled, for filled widgets: container in on_surface at
 * `state.disabled_container`, content in on_surface at
 * `state.disabled_content`, no recolour. bg_color, bg_opa, text_color,
 * text_opa, recolor_opa, end marker. Real opacity, as MD3 specifies, so
 * the disabled widget shows what it sits on. */
static lv_style_const_prop_t disabled_props[6];
static LV_STYLE_CONST_INIT(disabled_style, disabled_props);

static void theme_styles_build(const trlv_color_tokens_t * color, const trlv_layout_tokens_t * layout)
{
    const trlv_state_tokens_t * state = &color->state;

    trlv_prop_set_color(&screen_props[0], LV_STYLE_BG_COLOR, color->surface);
    trlv_prop_set_num(&screen_props[1], LV_STYLE_BG_OPA, LV_OPA_COVER);
    trlv_prop_set_color(&screen_props[2], LV_STYLE_TEXT_COLOR, color->on_surface);
    trlv_prop_set_end(&screen_props[3]);

    trlv_prop_set_color(&obj_props[0], LV_STYLE_BG_COLOR, color->surface);
    trlv_prop_set_color(&obj_props[1], LV_STYLE_BORDER_COLOR, color->outline_variant);
    trlv_prop_set_color(&obj_props[2], LV_STYLE_TEXT_COLOR, color->on_surface);
    trlv_prop_set_end(&obj_props[3]);

    /* MD3 filled button: 24 px side padding (space XL), 8 px between an
     * icon and the label (space SM). */
    trlv_prop_set_color(&button_props[0], LV_STYLE_BG_COLOR, color->primary);
    trlv_prop_set_num(&button_props[1], LV_STYLE_BG_OPA, LV_OPA_COVER);
    trlv_prop_set_color(&button_props[2], LV_STYLE_TEXT_COLOR, color->on_primary);
    trlv_prop_set_num(&button_props[3], LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_FULL]);
    trlv_prop_set_num(&button_props[4], LV_STYLE_HEIGHT, layout->control_height[TRLV_SIZE_MD]);
    trlv_prop_set_num(&button_props[5], LV_STYLE_PAD_LEFT, layout->space[TRLV_SPACE_XL]);
    trlv_prop_set_num(&button_props[6], LV_STYLE_PAD_RIGHT, layout->space[TRLV_SPACE_XL]);
    trlv_prop_set_num(&button_props[7], LV_STYLE_PAD_COLUMN, layout->space[TRLV_SPACE_SM]);
    trlv_prop_set_end(&button_props[8]);

    trlv_prop_set_color(&button_pressed_props[0], LV_STYLE_BG_COLOR,
                        trlv_state_layer(color->primary, color->on_primary, state->pressed));
    trlv_prop_set_end(&button_pressed_props[1]);

    trlv_prop_set_color(&button_focus_props[0], LV_STYLE_BG_COLOR,
                        trlv_state_layer(color->primary, color->on_primary, state->focus));
    trlv_prop_set_end(&button_focus_props[1]);

    trlv_prop_set_num(&focus_ring_props[0], LV_STYLE_OUTLINE_WIDTH, layout->focus_width);
    trlv_prop_set_color(&focus_ring_props[1], LV_STYLE_OUTLINE_COLOR, color->focus);
    trlv_prop_set_num(&focus_ring_props[2], LV_STYLE_OUTLINE_OPA, LV_OPA_COVER);
    trlv_prop_set_num(&focus_ring_props[3], LV_STYLE_OUTLINE_PAD, layout->space[TRLV_SPACE_2XS]);
    trlv_prop_set_end(&focus_ring_props[4]);

    trlv_prop_set_color(&disabled_props[0], LV_STYLE_BG_COLOR, color->on_surface);
    trlv_prop_set_num(&disabled_props[1], LV_STYLE_BG_OPA, state->disabled_container);
    trlv_prop_set_color(&disabled_props[2], LV_STYLE_TEXT_COLOR, color->on_surface);
    trlv_prop_set_num(&disabled_props[3], LV_STYLE_TEXT_OPA, state->disabled_content);
    trlv_prop_set_num(&disabled_props[4], LV_STYLE_RECOLOR_OPA, LV_OPA_TRANSP);
    trlv_prop_set_end(&disabled_props[5]);
}

/* Add the shared state styles to an interactive widget, with its own
 * pressed and focus state layers. Each later widget calls this too. */
static void add_state_styles(lv_obj_t * obj, const lv_style_t * pressed, const lv_style_t * focus)
{
    lv_obj_add_style(obj, pressed, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(obj, &pressed_reset_style, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(obj, focus, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_add_style(obj, &focus_ring_style, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_add_style(obj, &disabled_style, LV_PART_MAIN | LV_STATE_DISABLED);
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
        add_state_styles(obj, &button_pressed_style, &button_focus_style);
    }
}

/* The TreLVis theme on `disp`, or NULL if the display has another theme. */
static lv_theme_t * own_theme(lv_display_t * disp)
{
    lv_theme_t * theme = lv_display_get_theme(disp);
    return theme != NULL && lv_theme_get_user_data(theme) == &theme_marker ? theme : NULL;
}

/* (Re-)initialise the parent theme with the colours. Returns NULL with
 * TRLV_THEME_PARENT_NONE or on failure. */
static lv_theme_t * parent_init(lv_display_t * disp, const trlv_color_tokens_t * color,
                                const trlv_layout_tokens_t * layout)
{
#if TRLV_THEME_PARENT == TRLV_THEME_PARENT_DEFAULT
    /* LVGL's default theme needs a dark flag: dark when `surface` is below
     * 50 % luminance. */
    bool dark = lv_color_luminance(color->surface) < 128;
    const lv_font_t * body_font = layout->text[TRLV_TEXT_BODY_MEDIUM].font;
    if(body_font == NULL) body_font = LV_FONT_DEFAULT;

    lv_theme_t * parent = lv_theme_default_init(disp, color->primary, color->secondary, dark, body_font);
    if(parent == NULL) {
        LV_LOG_WARN("trlv: LVGL's default theme could not be initialised");
    }
    return parent;
#else
    LV_UNUSED(disp);
    LV_UNUSED(color);
    LV_UNUSED(layout);
    return NULL;
#endif
}

/* Point the TreLVis theme at its parent and callback. The parent's colours
 * and fonts are what lv_theme_get_color_primary() and
 * lv_theme_get_font_normal() return for this theme. */
static void theme_set_up(lv_theme_t * theme, lv_theme_t * parent)
{
    if(parent != NULL) lv_theme_copy(theme, parent);
    lv_theme_set_parent(theme, parent);
    lv_theme_set_apply_cb(theme, theme_apply);
    lv_theme_set_user_data(theme, &theme_marker);
}

static void theme_install(lv_display_t * disp, const trlv_color_tokens_t * color,
                          const trlv_layout_tokens_t * layout)
{
    lv_theme_t * parent = parent_init(disp, color, layout);

    /* Reuse the TreLVis theme if the display already has it (trlv_init
     * called again). After lv_deinit and lv_init the display is new and has
     * LVGL's theme, so a new one is created in the new LVGL heap. */
    lv_theme_t * theme = own_theme(disp);
    if(theme == NULL) {
        theme = lv_theme_create();
        if(theme == NULL) {
            LV_LOG_WARN("trlv_init: out of memory for the theme; no theme installed");
            return;
        }
    }

    theme_set_up(theme, parent);
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

/* Colour-mode switch: the parent theme gets the new colours, and the
 * TreLVis theme copies them again. Allocates nothing new. */
static void theme_recolor(lv_display_t * disp, const trlv_color_tokens_t * color,
                          const trlv_layout_tokens_t * layout)
{
    lv_theme_t * parent = parent_init(disp, color, layout);
    lv_theme_t * theme = own_theme(disp);
    if(theme != NULL) theme_set_up(theme, parent);
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
    current_display = disp;

    trlv_layout_styles_build(layout);
    trlv_text_styles_build(layout);

#if TRLV_USE_THEME
    theme_styles_build(color, layout);
    theme_install(disp, color, layout);
#endif

    /* Objects that already use a token style pick up the new values. */
    lv_obj_report_style_change(NULL);
}

void trlv_theme_set_color_mode(const trlv_color_tokens_t * color)
{
    if(color == NULL) {
        LV_LOG_WARN("trlv_theme_set_color_mode: color tokens are required");
        return;
    }
    if(current_layout == NULL) {
        LV_LOG_WARN("trlv_theme_set_color_mode: call trlv_init first");
        return;
    }

    current_color = color;

#if TRLV_USE_THEME
    theme_styles_build(color, current_layout);
    theme_recolor(current_display, color, current_layout);
#endif

    /* Every object redraws with the re-set shared styles. TreLVis never
     * walks the object tree. */
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
