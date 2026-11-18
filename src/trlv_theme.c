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
 * Which styles this build needs. A style shared by several widgets is
 * compiled only when one of them is on in lv_conf.h, so a product with few
 * widgets pays no RAM or flash for the others' styles.
 *-------------------------------------------------------------------------*/

#define THEME_TRACK         (LV_USE_BAR || LV_USE_SLIDER || LV_USE_SWITCH)
#define THEME_INDICATOR     (THEME_TRACK || LV_USE_ARC)
#define THEME_ARC           (LV_USE_ARC || LV_USE_SPINNER)
#define THEME_FIELD         (LV_USE_DROPDOWN || LV_USE_TEXTAREA)
#define THEME_FOCUS_RING    (LV_USE_BUTTON || LV_USE_SWITCH || LV_USE_CHECKBOX || LV_USE_SLIDER || LV_USE_ARC)
#define THEME_PRESSED_RESET (LV_USE_BUTTON || LV_USE_CHECKBOX || LV_USE_SLIDER || THEME_FIELD)
#define THEME_DISABLED      (LV_USE_BUTTON || THEME_TRACK)
#define THEME_DISABLED_PART (THEME_TRACK || LV_USE_CHECKBOX || LV_USE_ARC)
#define THEME_ROUND         (THEME_TRACK || LV_USE_ARC)
#define THEME_KNOB          (LV_USE_SLIDER || LV_USE_ARC)
#define THEME_DPX           (THEME_KNOB || THEME_ARC || LV_USE_CHECKBOX)

/*---------------------------------------------------------------------------
 * Writing token styles. Each token style has a static property array sized
 * for its properties plus the end marker (see trlv_internal.h). A writer
 * fills one array and checks its size, so a miscounted array is reported
 * instead of overflowing.
 *-------------------------------------------------------------------------*/

/* A token style: its writable property array and the style over it. */
#define TOKEN_STYLE(name, prop_count)                                  \
    static lv_style_const_prop_t name##_props[(prop_count) + 1];       \
    static LV_STYLE_CONST_INIT(name##_style, name##_props)

typedef struct {
    lv_style_const_prop_t * props;
    uint32_t size;      /* entries, including the end marker */
    uint32_t count;     /* entries written */
    const char * name;
} style_writer_t;

#define WRITER(name) { name##_props, LV_ARRAYLEN(name##_props), 0, #name }

static lv_style_const_prop_t * writer_next(style_writer_t * w)
{
    if(w->count + 1 >= w->size) {
        LV_LOG_ERROR("trlv: theme style %s has too few entries", w->name);
        return NULL;
    }
    return &w->props[w->count++];
}

static void put_num(style_writer_t * w, lv_style_prop_t prop, int32_t num)
{
    lv_style_const_prop_t * entry = writer_next(w);
    if(entry != NULL) trlv_prop_set_num(entry, prop, num);
}

static void put_color(style_writer_t * w, lv_style_prop_t prop, lv_color_t color)
{
    lv_style_const_prop_t * entry = writer_next(w);
    if(entry != NULL) trlv_prop_set_color(entry, prop, color);
}

static void put_end(style_writer_t * w)
{
    if(w->count + 1 != w->size) {
        LV_LOG_ERROR("trlv: theme style %s has %d unused entries", w->name, (int)(w->size - w->count - 1));
    }
    trlv_prop_set_end(&w->props[w->count]);
}

/* Write one style: `name` is the TOKEN_STYLE name, the body uses `w`. */
#define BUILD(name, body)                   \
    do {                                    \
        style_writer_t writer = WRITER(name); \
        style_writer_t * w = &writer;       \
        body                                \
        put_end(w);                         \
    } while(0)

#if THEME_DPX
/* A size with no token, scaled by the display's DPI as LVGL's default theme
 * scales it (LV_DPX_CALC), so these widgets keep the default theme's shape
 * with either parent theme. */
static int32_t dpx(lv_display_t * disp, int32_t n)
{
    return LV_DPX_CALC(lv_display_get_dpi(disp), n);
}
#endif

/*---------------------------------------------------------------------------
 * Constant styles (flash)
 *-------------------------------------------------------------------------*/

#if LV_USE_BUTTON
/* Filled button: content centred in a row (an icon and a label sit side
 * by side, as in an MD3 button), no vertical padding (the height is fixed)
 * and no shadow (MD3 filled buttons are at elevation level 0). */
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
#endif

#if THEME_PRESSED_RESET
/* Pressed, for every widget: switch off LVGL's default recolour (it tints
 * the content too) and grow. */
static const lv_style_const_prop_t pressed_reset_props[] = {
    LV_STYLE_CONST_RECOLOR_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_TRANSFORM_WIDTH(0),
    LV_STYLE_CONST_TRANSFORM_HEIGHT(0),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(pressed_reset_style, pressed_reset_props);
#endif

#if THEME_FIELD
/* Fields show focus with their border, not a ring: switch off LVGL's
 * default outline. */
static const lv_style_const_prop_t no_outline_props[] = {
    LV_STYLE_CONST_OUTLINE_WIDTH(0),
    LV_STYLE_CONST_OUTLINE_OPA(LV_OPA_TRANSP),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(no_outline_style, no_outline_props);
#endif

#if LV_USE_CHECKBOX
/* The check mark of a checked checkbox, as in LVGL's default theme. */
static const lv_style_const_prop_t check_mark_props[] = {
    LV_STYLE_CONST_BG_IMAGE_SRC(LV_SYMBOL_OK),
    LV_STYLE_CONST_PROPS_END
};
static LV_STYLE_CONST_INIT(check_mark_style, check_mark_props);
#endif

/*---------------------------------------------------------------------------
 * Token styles (RAM, written by theme_styles_build)
 *-------------------------------------------------------------------------*/

/* Screen and plain lv_obj. The screen also gets the Body Medium role. A
 * plain lv_obj keeps the parent theme's shape (radius, border width,
 * padding); boxes, rows and columns remove all theme styles. */
TOKEN_STYLE(screen, 3);
TOKEN_STYLE(obj, 3);

/* Filled button, its state layers. Label Large is added next to it. */
#if LV_USE_BUTTON
TOKEN_STYLE(button, 8);
TOKEN_STYLE(button_pressed, 1);
TOKEN_STYLE(button_focus, 1);
#endif

/* Shared states. Focus ring: MD3 focus indicator, `focus_width` thick and
 * space 2XS outside. Disabled: on_surface at the disabled opacities, as
 * real opacity (MD3), for a filled container, a strong part (indicator,
 * knob, check box) and text. */
#if THEME_FOCUS_RING
TOKEN_STYLE(focus_ring, 4);
#endif
#if THEME_DISABLED
TOKEN_STYLE(disabled, 5);
#endif
#if THEME_DISABLED_PART
TOKEN_STYLE(disabled_part, 3);
#endif
#if LV_USE_CHECKBOX
TOKEN_STYLE(disabled_text, 2);
#endif

/* Geometry. The widgets' shape is TreLVis's too, not only their colours,
 * so they keep it with TRLV_THEME_PARENT_NONE. Round: radius FULL for
 * tracks, indicators and knobs. Knob: the slider's and arc's knob reaches
 * 6 dpx beyond the track (LVGL's default theme). */
#if THEME_ROUND
TOKEN_STYLE(round, 1);
#endif
#if THEME_KNOB
TOKEN_STYLE(knob, 4);
#endif

/* Bars, sliders and switches: the inactive track and the active
 * indicator. The slider's knob is the indicator colour. */
#if THEME_TRACK
TOKEN_STYLE(track, 2);
#endif
#if THEME_INDICATOR
TOKEN_STYLE(indicator, 2);
#endif

/* Arcs and spinners: the same pair as arc colours, 15 dpx wide with
 * rounded ends (LVGL's default theme). */
#if THEME_ARC
TOKEN_STYLE(arc_track, 4);
TOKEN_STYLE(arc_indicator, 4);
TOKEN_STYLE(arc_disabled_track, 2);
TOKEN_STYLE(arc_disabled_indicator, 2);
#endif

/* Switch: the MD3 52 x 32 track (`control_height[SM]` tall), an outline
 * when off, primary when on; the handle is 16 px off, 24 px on, 28 px
 * pressed (as fractions of the track height). */
#if LV_USE_SWITCH
TOKEN_STYLE(switch_track, 4);
TOKEN_STYLE(switch_track_checked, 1);
TOKEN_STYLE(switch_knob, 7);
TOKEN_STYLE(switch_knob_checked, 5);
TOKEN_STYLE(switch_knob_pressed, 4);
#endif

/* Checkbox: a space SM gap before the text; the box outlined in
 * on_surface_variant, 3 dpx around the mark (LVGL's default theme), filled
 * primary with an on_primary check mark when checked. */
#if LV_USE_CHECKBOX
TOKEN_STYLE(checkbox, 1);
TOKEN_STYLE(checkbox_box, 8);
TOKEN_STYLE(checkbox_box_checked, 4);
TOKEN_STYLE(checkbox_box_disabled, 3);
#endif

/* Dropdown and textarea: an MD3 outlined field. Space LG at the sides as in
 * MD3, space SM at the top and bottom, so a one-line field set to
 * `control_height[MD]` fits a line of body text (MD3's 16 px is for its
 * 56 px field). The focused border is primary and `focus_width` thick:
 * the field's focus indicator. */
#if THEME_FIELD
TOKEN_STYLE(field, 9);
TOKEN_STYLE(field_focused, 2);
TOKEN_STYLE(field_disabled, 5);
#endif

/* Textarea parts: a primary caret, placeholder in on_surface_variant. */
#if LV_USE_TEXTAREA
TOKEN_STYLE(cursor, 1);
TOKEN_STYLE(placeholder, 1);
#endif

/* Dropdown list: an MD3 menu on surface_container at elevation level 2;
 * the selected option in secondary_container, a pressed one with the
 * state layer. */
#if LV_USE_DROPDOWN
TOKEN_STYLE(menu, 11);
TOKEN_STYLE(menu_option, 2);
TOKEN_STYLE(menu_option_checked, 2);
TOKEN_STYLE(menu_option_pressed, 2);
#endif

static void theme_styles_build(lv_display_t * disp, const trlv_color_tokens_t * color,
                               const trlv_layout_tokens_t * layout)
{
    const trlv_state_tokens_t * state = &color->state;

    /* Unused when lv_conf.h turns off every widget the theme styles. */
    LV_UNUSED(disp);
    LV_UNUSED(state);
    LV_UNUSED(layout);

    BUILD(screen, {
        put_color(w, LV_STYLE_BG_COLOR, color->surface);
        put_num(w, LV_STYLE_BG_OPA, LV_OPA_COVER);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface);
    });
    BUILD(obj, {
        put_color(w, LV_STYLE_BG_COLOR, color->surface);
        put_color(w, LV_STYLE_BORDER_COLOR, color->outline_variant);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface);
    });

#if LV_USE_BUTTON
    /* MD3 filled button: 24 px side padding (space XL), 8 px between an
     * icon and the label (space SM). */
    BUILD(button, {
        put_color(w, LV_STYLE_BG_COLOR, color->primary);
        put_num(w, LV_STYLE_BG_OPA, LV_OPA_COVER);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_primary);
        put_num(w, LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_FULL]);
        put_num(w, LV_STYLE_HEIGHT, layout->control_height[TRLV_SIZE_MD]);
        put_num(w, LV_STYLE_PAD_LEFT, layout->space[TRLV_SPACE_XL]);
        put_num(w, LV_STYLE_PAD_RIGHT, layout->space[TRLV_SPACE_XL]);
        put_num(w, LV_STYLE_PAD_COLUMN, layout->space[TRLV_SPACE_SM]);
    });
    BUILD(button_pressed, {
        put_color(w, LV_STYLE_BG_COLOR, trlv_state_layer(color->primary, color->on_primary, state->pressed));
    });
    BUILD(button_focus, {
        put_color(w, LV_STYLE_BG_COLOR, trlv_state_layer(color->primary, color->on_primary, state->focus));
    });
#endif

#if THEME_FOCUS_RING
    BUILD(focus_ring, {
        put_num(w, LV_STYLE_OUTLINE_WIDTH, layout->focus_width);
        put_color(w, LV_STYLE_OUTLINE_COLOR, color->focus);
        put_num(w, LV_STYLE_OUTLINE_OPA, LV_OPA_COVER);
        put_num(w, LV_STYLE_OUTLINE_PAD, layout->space[TRLV_SPACE_2XS]);
    });
#endif
#if THEME_DISABLED
    BUILD(disabled, {
        put_color(w, LV_STYLE_BG_COLOR, color->on_surface);
        put_num(w, LV_STYLE_BG_OPA, state->disabled_container);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface);
        put_num(w, LV_STYLE_TEXT_OPA, state->disabled_content);
        put_num(w, LV_STYLE_RECOLOR_OPA, LV_OPA_TRANSP);
    });
#endif
#if THEME_DISABLED_PART
    BUILD(disabled_part, {
        put_color(w, LV_STYLE_BG_COLOR, color->on_surface);
        put_num(w, LV_STYLE_BG_OPA, state->disabled_content);
        put_num(w, LV_STYLE_RECOLOR_OPA, LV_OPA_TRANSP);
    });
#endif
#if LV_USE_CHECKBOX
    BUILD(disabled_text, {
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface);
        put_num(w, LV_STYLE_TEXT_OPA, state->disabled_content);
    });
#endif

#if THEME_ROUND
    BUILD(round, {
        put_num(w, LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_FULL]);
    });
#endif
#if THEME_KNOB
    int32_t knob_pad = dpx(disp, 6);
    BUILD(knob, {
        put_num(w, LV_STYLE_PAD_TOP, knob_pad);
        put_num(w, LV_STYLE_PAD_BOTTOM, knob_pad);
        put_num(w, LV_STYLE_PAD_LEFT, knob_pad);
        put_num(w, LV_STYLE_PAD_RIGHT, knob_pad);
    });
#endif

#if THEME_TRACK
    BUILD(track, {
        put_color(w, LV_STYLE_BG_COLOR, color->surface_container_highest);
        put_num(w, LV_STYLE_BG_OPA, LV_OPA_COVER);
    });
#endif
#if THEME_INDICATOR
    BUILD(indicator, {
        put_color(w, LV_STYLE_BG_COLOR, color->primary);
        put_num(w, LV_STYLE_BG_OPA, LV_OPA_COVER);
    });
#endif

#if THEME_ARC
    int32_t arc_width = dpx(disp, 15);
    BUILD(arc_track, {
        put_color(w, LV_STYLE_ARC_COLOR, color->surface_container_highest);
        put_num(w, LV_STYLE_ARC_OPA, LV_OPA_COVER);
        put_num(w, LV_STYLE_ARC_WIDTH, arc_width);
        put_num(w, LV_STYLE_ARC_ROUNDED, 1);
    });
    BUILD(arc_indicator, {
        put_color(w, LV_STYLE_ARC_COLOR, color->primary);
        put_num(w, LV_STYLE_ARC_OPA, LV_OPA_COVER);
        put_num(w, LV_STYLE_ARC_WIDTH, arc_width);
        put_num(w, LV_STYLE_ARC_ROUNDED, 1);
    });
    BUILD(arc_disabled_track, {
        put_color(w, LV_STYLE_ARC_COLOR, color->on_surface);
        put_num(w, LV_STYLE_ARC_OPA, state->disabled_container);
    });
    BUILD(arc_disabled_indicator, {
        put_color(w, LV_STYLE_ARC_COLOR, color->on_surface);
        put_num(w, LV_STYLE_ARC_OPA, state->disabled_content);
    });
#endif

#if LV_USE_SWITCH
    int32_t switch_height = layout->control_height[TRLV_SIZE_SM];
    BUILD(switch_track, {
        put_num(w, LV_STYLE_WIDTH, switch_height * 13 / 8);
        put_num(w, LV_STYLE_HEIGHT, switch_height);
        put_color(w, LV_STYLE_BORDER_COLOR, color->outline);
        put_num(w, LV_STYLE_BORDER_WIDTH, layout->border_width);
    });
    BUILD(switch_track_checked, {
        put_color(w, LV_STYLE_BORDER_COLOR, color->primary);
    });
    BUILD(switch_knob, {
        put_color(w, LV_STYLE_BG_COLOR, color->outline);
        put_num(w, LV_STYLE_BG_OPA, LV_OPA_COVER);
        put_num(w, LV_STYLE_PAD_TOP, -switch_height / 4);
        put_num(w, LV_STYLE_PAD_BOTTOM, -switch_height / 4);
        put_num(w, LV_STYLE_PAD_LEFT, -switch_height / 4);
        put_num(w, LV_STYLE_PAD_RIGHT, -switch_height / 4);
        put_num(w, LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_FULL]);
    });
    BUILD(switch_knob_checked, {
        put_color(w, LV_STYLE_BG_COLOR, color->on_primary);
        put_num(w, LV_STYLE_PAD_TOP, -switch_height / 8);
        put_num(w, LV_STYLE_PAD_BOTTOM, -switch_height / 8);
        put_num(w, LV_STYLE_PAD_LEFT, -switch_height / 8);
        put_num(w, LV_STYLE_PAD_RIGHT, -switch_height / 8);
    });
    BUILD(switch_knob_pressed, {
        put_num(w, LV_STYLE_PAD_TOP, -switch_height / 16);
        put_num(w, LV_STYLE_PAD_BOTTOM, -switch_height / 16);
        put_num(w, LV_STYLE_PAD_LEFT, -switch_height / 16);
        put_num(w, LV_STYLE_PAD_RIGHT, -switch_height / 16);
    });
#endif

#if LV_USE_CHECKBOX
    BUILD(checkbox, {
        put_num(w, LV_STYLE_PAD_COLUMN, layout->space[TRLV_SPACE_SM]);
    });
    int32_t mark_pad = dpx(disp, 3);
    BUILD(checkbox_box, {
        put_num(w, LV_STYLE_BG_OPA, LV_OPA_TRANSP);
        put_color(w, LV_STYLE_BORDER_COLOR, color->on_surface_variant);
        put_num(w, LV_STYLE_BORDER_WIDTH, layout->border_width);
        put_num(w, LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_XS]);
        put_num(w, LV_STYLE_PAD_TOP, mark_pad);
        put_num(w, LV_STYLE_PAD_BOTTOM, mark_pad);
        put_num(w, LV_STYLE_PAD_LEFT, mark_pad);
        put_num(w, LV_STYLE_PAD_RIGHT, mark_pad);
    });
    BUILD(checkbox_box_checked, {
        put_color(w, LV_STYLE_BG_COLOR, color->primary);
        put_num(w, LV_STYLE_BG_OPA, LV_OPA_COVER);
        put_color(w, LV_STYLE_BORDER_COLOR, color->primary);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_primary);
    });
    BUILD(checkbox_box_disabled, {
        put_color(w, LV_STYLE_BORDER_COLOR, color->on_surface);
        put_num(w, LV_STYLE_BORDER_OPA, state->disabled_content);
        put_num(w, LV_STYLE_RECOLOR_OPA, LV_OPA_TRANSP);
    });
#endif

#if THEME_FIELD
    int32_t pad_ver = layout->space[TRLV_SPACE_SM];
    int32_t pad_hor = layout->space[TRLV_SPACE_LG];
    BUILD(field, {
        put_color(w, LV_STYLE_BG_COLOR, color->surface);
        put_color(w, LV_STYLE_BORDER_COLOR, color->outline);
        put_num(w, LV_STYLE_BORDER_WIDTH, layout->border_width);
        put_num(w, LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_XS]);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface);
        put_num(w, LV_STYLE_PAD_TOP, pad_ver);
        put_num(w, LV_STYLE_PAD_BOTTOM, pad_ver);
        put_num(w, LV_STYLE_PAD_LEFT, pad_hor);
        put_num(w, LV_STYLE_PAD_RIGHT, pad_hor);
    });
    BUILD(field_focused, {
        put_color(w, LV_STYLE_BORDER_COLOR, color->primary);
        put_num(w, LV_STYLE_BORDER_WIDTH, layout->focus_width);
    });
    BUILD(field_disabled, {
        put_color(w, LV_STYLE_BORDER_COLOR, color->on_surface);
        put_num(w, LV_STYLE_BORDER_OPA, state->disabled_container);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface);
        put_num(w, LV_STYLE_TEXT_OPA, state->disabled_content);
        put_num(w, LV_STYLE_RECOLOR_OPA, LV_OPA_TRANSP);
    });
#endif
#if LV_USE_TEXTAREA
    BUILD(cursor, {
        put_color(w, LV_STYLE_BORDER_COLOR, color->primary);
    });
    BUILD(placeholder, {
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface_variant);
    });
#endif

#if LV_USE_DROPDOWN
    const trlv_shadow_t * menu_shadow = &layout->elevation[TRLV_ELEVATION_2];
    BUILD(menu, {
        put_color(w, LV_STYLE_BG_COLOR, color->surface_container);
        put_num(w, LV_STYLE_BG_OPA, LV_OPA_COVER);
        put_num(w, LV_STYLE_BORDER_WIDTH, 0);
        put_num(w, LV_STYLE_RADIUS, layout->radius[TRLV_RADIUS_XS]);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface);
        put_color(w, LV_STYLE_SHADOW_COLOR, color->shadow);
        put_num(w, LV_STYLE_SHADOW_WIDTH, menu_shadow->blur);
        put_num(w, LV_STYLE_SHADOW_SPREAD, menu_shadow->spread);
        put_num(w, LV_STYLE_SHADOW_OFFSET_X, menu_shadow->offset_x);
        put_num(w, LV_STYLE_SHADOW_OFFSET_Y, menu_shadow->offset_y);
        put_num(w, LV_STYLE_SHADOW_OPA, menu_shadow->opa);
    });
    BUILD(menu_option, {
        put_color(w, LV_STYLE_BG_COLOR, color->surface_container);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_surface);
    });
    BUILD(menu_option_checked, {
        put_color(w, LV_STYLE_BG_COLOR, color->secondary_container);
        put_color(w, LV_STYLE_TEXT_COLOR, color->on_secondary_container);
    });
    BUILD(menu_option_pressed, {
        put_color(w, LV_STYLE_BG_COLOR,
                  trlv_state_layer(color->surface_container, color->on_surface, state->pressed));
        put_num(w, LV_STYLE_RECOLOR_OPA, LV_OPA_TRANSP);
    });
#endif
}

/*---------------------------------------------------------------------------
 * Applying styles
 *-------------------------------------------------------------------------*/

#if THEME_FOCUS_RING
/* The focus ring for keypad and encoder focus, also while an encoder edits
 * the widget (LVGL's default theme draws a different outline then). */
static void add_focus_ring(lv_obj_t * obj)
{
    lv_obj_add_style(obj, &focus_ring_style, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_add_style(obj, &focus_ring_style, LV_PART_MAIN | LV_STATE_EDITED);
}
#endif

#if LV_USE_BUTTON
/* The shared states of a filled widget, with its own pressed and focus
 * state layers. */
static void add_state_styles(lv_obj_t * obj, const lv_style_t * pressed, const lv_style_t * focus)
{
    lv_obj_add_style(obj, pressed, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(obj, &pressed_reset_style, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(obj, focus, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    add_focus_ring(obj);
    lv_obj_add_style(obj, &disabled_style, LV_PART_MAIN | LV_STATE_DISABLED);
}
#endif

#if LV_USE_BAR || LV_USE_SLIDER
/* A track with an indicator: bars and sliders. */
static void add_track_styles(lv_obj_t * obj)
{
    lv_obj_add_style(obj, &track_style, LV_PART_MAIN);
    lv_obj_add_style(obj, &round_style, LV_PART_MAIN);
    lv_obj_add_style(obj, &indicator_style, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &round_style, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &disabled_style, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_style(obj, &disabled_part_style, LV_PART_INDICATOR | LV_STATE_DISABLED);
}
#endif

#if THEME_ARC
/* An arc track with an indicator: arcs and spinners. */
static void add_arc_styles(lv_obj_t * obj)
{
    lv_obj_add_style(obj, &arc_track_style, LV_PART_MAIN);
    lv_obj_add_style(obj, &arc_indicator_style, LV_PART_INDICATOR);
    lv_obj_add_style(obj, &arc_disabled_track_style, LV_PART_MAIN | LV_STATE_DISABLED);
    lv_obj_add_style(obj, &arc_disabled_indicator_style, LV_PART_INDICATOR | LV_STATE_DISABLED);
}
#endif

#if THEME_FIELD
/* An outlined field: dropdowns and textareas. */
static void add_field_styles(lv_obj_t * obj)
{
    lv_obj_add_style(obj, &field_style, LV_PART_MAIN);
    lv_obj_add_style(obj, &field_focused_style, LV_PART_MAIN | LV_STATE_FOCUSED);
    lv_obj_add_style(obj, &no_outline_style, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_add_style(obj, &no_outline_style, LV_PART_MAIN | LV_STATE_EDITED);
    lv_obj_add_style(obj, &pressed_reset_style, LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_add_style(obj, &field_disabled_style, LV_PART_MAIN | LV_STATE_DISABLED);
}
#endif

/*---------------------------------------------------------------------------
 * The theme
 *-------------------------------------------------------------------------*/

/* Its address marks a theme as the TreLVis theme (in the theme's user data),
 * so trlv_init can find and reuse it instead of creating another. */
static char theme_marker;

/* Styles each object when it is created, after the parent theme. Labels get
 * nothing: they inherit their text role and colour from what they sit in.
 * Widgets not listed keep the parent theme's styles. */
static void theme_apply(lv_theme_t * theme, lv_obj_t * obj)
{
    LV_UNUSED(theme);

    if(lv_obj_get_parent(obj) == NULL) {
        lv_obj_add_style(obj, &screen_style, LV_PART_MAIN);
        lv_obj_add_style(obj, trlv_text_role_style(TRLV_TEXT_BODY_MEDIUM), LV_PART_MAIN);
        return;
    }
    if(lv_obj_check_type(obj, &lv_obj_class)) {
        lv_obj_add_style(obj, &obj_style, LV_PART_MAIN);
        return;
    }
#if LV_USE_BUTTON
    if(lv_obj_check_type(obj, &lv_button_class)) {
#if LV_USE_TABVIEW
        /* The tabs of an lv_tabview are lv_buttons in its tab bar (its
         * first child). They are not MD3 buttons: leave them to the parent
         * theme, as LVGL's default theme does. */
        lv_obj_t * bar = lv_obj_get_parent(obj);
        lv_obj_t * tabview = lv_obj_get_parent(bar);
        if(tabview != NULL && lv_obj_check_type(tabview, &lv_tabview_class) &&
           lv_obj_get_child(tabview, 0) == bar) {
            return;
        }
#endif
        lv_obj_add_style(obj, &button_style, LV_PART_MAIN);
        lv_obj_add_style(obj, &button_layout_style, LV_PART_MAIN);
        lv_obj_add_style(obj, trlv_text_role_style(TRLV_TEXT_LABEL_LARGE), LV_PART_MAIN);
        add_state_styles(obj, &button_pressed_style, &button_focus_style);
        return;
    }
#endif
#if LV_USE_SWITCH
    if(lv_obj_check_type(obj, &lv_switch_class)) {
        lv_obj_add_style(obj, &track_style, LV_PART_MAIN);
        lv_obj_add_style(obj, &round_style, LV_PART_MAIN);
        lv_obj_add_style(obj, &switch_track_style, LV_PART_MAIN);
        lv_obj_add_style(obj, &switch_track_checked_style, LV_PART_MAIN | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &round_style, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &indicator_style, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &switch_knob_style, LV_PART_KNOB);
        lv_obj_add_style(obj, &switch_knob_checked_style, LV_PART_KNOB | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &switch_knob_pressed_style, LV_PART_KNOB | LV_STATE_PRESSED);
        add_focus_ring(obj);
        lv_obj_add_style(obj, &disabled_style, LV_PART_MAIN | LV_STATE_DISABLED);
        lv_obj_add_style(obj, &disabled_style, LV_PART_INDICATOR | LV_STATE_DISABLED);
        lv_obj_add_style(obj, &disabled_part_style, LV_PART_KNOB | LV_STATE_DISABLED);
        return;
    }
#endif
#if LV_USE_CHECKBOX
    if(lv_obj_check_type(obj, &lv_checkbox_class)) {
        lv_obj_add_style(obj, &checkbox_style, LV_PART_MAIN);
        lv_obj_add_style(obj, &checkbox_box_style, LV_PART_INDICATOR);
        lv_obj_add_style(obj, &checkbox_box_checked_style, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &check_mark_style, LV_PART_INDICATOR | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &pressed_reset_style, LV_PART_INDICATOR | LV_STATE_PRESSED);
        add_focus_ring(obj);
        lv_obj_add_style(obj, &disabled_text_style, LV_PART_MAIN | LV_STATE_DISABLED);
        lv_obj_add_style(obj, &checkbox_box_disabled_style, LV_PART_INDICATOR | LV_STATE_DISABLED);
        lv_obj_add_style(obj, &disabled_part_style, LV_PART_INDICATOR | LV_STATE_CHECKED | LV_STATE_DISABLED);
        return;
    }
#endif
#if LV_USE_SLIDER
    if(lv_obj_check_type(obj, &lv_slider_class)) {
        add_track_styles(obj);
        lv_obj_add_style(obj, &indicator_style, LV_PART_KNOB);
        lv_obj_add_style(obj, &round_style, LV_PART_KNOB);
        lv_obj_add_style(obj, &knob_style, LV_PART_KNOB);
        lv_obj_add_style(obj, &pressed_reset_style, LV_PART_KNOB | LV_STATE_PRESSED);
        lv_obj_add_style(obj, &disabled_part_style, LV_PART_KNOB | LV_STATE_DISABLED);
        add_focus_ring(obj);
        return;
    }
#endif
#if LV_USE_BAR
    if(lv_obj_check_type(obj, &lv_bar_class)) {
        add_track_styles(obj);
        return;
    }
#endif
#if LV_USE_SPINNER
    /* Before the arc: a spinner is an arc. */
    if(lv_obj_check_type(obj, &lv_spinner_class)) {
        add_arc_styles(obj);
        return;
    }
#endif
#if LV_USE_ARC
    if(lv_obj_check_type(obj, &lv_arc_class)) {
        add_arc_styles(obj);
        lv_obj_add_style(obj, &indicator_style, LV_PART_KNOB);
        lv_obj_add_style(obj, &round_style, LV_PART_KNOB);
        lv_obj_add_style(obj, &knob_style, LV_PART_KNOB);
        lv_obj_add_style(obj, &disabled_part_style, LV_PART_KNOB | LV_STATE_DISABLED);
        add_focus_ring(obj);
        return;
    }
#endif
#if LV_USE_DROPDOWN
    if(lv_obj_check_type(obj, &lv_dropdown_class)) {
        add_field_styles(obj);
        return;
    }
    if(lv_obj_check_type(obj, &lv_dropdownlist_class)) {
        lv_obj_add_style(obj, &menu_style, LV_PART_MAIN);
        lv_obj_add_style(obj, &menu_option_style, LV_PART_SELECTED);
        lv_obj_add_style(obj, &menu_option_checked_style, LV_PART_SELECTED | LV_STATE_CHECKED);
        lv_obj_add_style(obj, &menu_option_pressed_style, LV_PART_SELECTED | LV_STATE_PRESSED);
        return;
    }
#endif
#if LV_USE_TEXTAREA
    if(lv_obj_check_type(obj, &lv_textarea_class)) {
        add_field_styles(obj);
        lv_obj_add_style(obj, &cursor_style, LV_PART_CURSOR | LV_STATE_FOCUSED);
        lv_obj_add_style(obj, &placeholder_style, LV_PART_TEXTAREA_PLACEHOLDER);
        return;
    }
#endif
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
    trlv_surface_styles_build(color, layout);

#if TRLV_USE_THEME
    theme_styles_build(disp, color, layout);
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
    trlv_surface_styles_build(color, current_layout);

#if TRLV_USE_THEME
    theme_styles_build(current_display, color, current_layout);
    theme_recolor(current_display, color, current_layout);
#endif

    /* Every object redraws with the re-set shared styles. TreLVis never
     * walks the object tree. */
    lv_obj_report_style_change(NULL);
}

const trlv_color_tokens_t * trlv_get_color_tokens(void)
{
    return current_color;
}

const trlv_layout_tokens_t * trlv_get_layout_tokens(void)
{
    return current_layout;
}
