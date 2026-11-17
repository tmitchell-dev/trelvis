/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * The README's "A taste of the API" card, built twice: with TreLVis, and
 * with plain LVGL 9.6 calls under LVGL's default theme, as the README's
 * comparison shows them. Both trees must lay out the same and resolve the
 * same colours and fonts, in light and in dark.
 *
 * The two builders below are the README's code blocks. If you change one,
 * change the README (and the line counts under it) in the same PR.
 *
 * Each test runs on a fresh LVGL, so the bare-LVGL tests record what they
 * measured and the TreLVis tests compare against it. The bare side runs
 * without trlv_init, under LVGL's default theme, as a project without
 * TreLVis would.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

/*===========================================================================
 * With TreLVis (README block 1, after trlv_init)
 *=========================================================================*/

static lv_obj_t * card_trelvis(lv_obj_t * parent)
{
    lv_obj_t * card = trlv_column(parent, TRLV_SPACE_SM);   /* Figma: auto layout, vertical */
    trlv_card(card, TRLV_CARD_FILLED);                      /* MD3 filled card               */
    trlv_pad(card, TRLV_SPACE_LG);
    trlv_fill_width(card);                                  /* Figma: fill container         */

    lv_obj_t * header = trlv_row(card, TRLV_SPACE_SM);
    trlv_fill_width(header);
    trlv_justify(header, TRLV_JUSTIFY_SPACE_BETWEEN);       /* CSS: justify-content          */
    trlv_align_items(header, TRLV_ALIGN_ITEMS_CENTER);      /* CSS: align-items              */
    trlv_text(header, "Temperature", TRLV_TEXT_TITLE_MEDIUM);
    trlv_text(header, "37.2 °C",     TRLV_TEXT_HEADLINE_SMALL);

    lv_obj_t * btn = lv_button_create(card);                /* plain LVGL; MD3 filled button via the theme */
    trlv_button_variant(btn, TRLV_BUTTON_TONAL);
    trlv_text(btn, "Calibrate", TRLV_TEXT_LABEL_LARGE);

    return card;
}

/*===========================================================================
 * Plain LVGL 9.6 (README block 2)
 *=========================================================================*/

/* ---- Once per project ---- */

#define SPACE_SM       8      /* the project's spacing, shape and sizes */
#define SPACE_LG       16
#define SPACE_XL       24
#define RADIUS_MD      12
#define BUTTON_HEIGHT  40
#define FOCUS_WIDTH    3

typedef struct {              /* the colours this card uses, per colour mode */
    lv_color_t surface_container_highest, on_surface;
    lv_color_t secondary_container, on_secondary_container;
    lv_color_t focus;
} palette_t;

static const palette_t palette_light = {
    LV_COLOR_MAKE(0xE6, 0xE0, 0xE9), LV_COLOR_MAKE(0x1D, 0x1B, 0x20),
    LV_COLOR_MAKE(0xE8, 0xDE, 0xF8), LV_COLOR_MAKE(0x1D, 0x19, 0x2B),
    LV_COLOR_MAKE(0x62, 0x5B, 0x71),
};
static const palette_t palette_dark = {
    LV_COLOR_MAKE(0x36, 0x34, 0x3B), LV_COLOR_MAKE(0xE6, 0xE0, 0xE9),
    LV_COLOR_MAKE(0x4A, 0x44, 0x58), LV_COLOR_MAKE(0xE8, 0xDE, 0xF8),
    LV_COLOR_MAKE(0xCC, 0xC2, 0xDC),
};

static lv_style_t style_card;
static lv_style_t style_title_medium, style_headline_small, style_label_large;
static lv_style_t style_button, style_tonal, style_tonal_pressed, style_tonal_focus,
       style_tonal_disabled;

static void text_style_init(lv_style_t * style, const lv_font_t * font, int32_t line_height)
{
    lv_style_init(style);
    lv_style_set_text_font(style, font);
    lv_style_set_text_line_space(style, line_height - lv_font_get_line_height(font));
}

/* At start-up, and on every light/dark switch followed by
 * lv_obj_report_style_change(NULL). */
static void styles_set_colors(const palette_t * p)
{
    lv_color_t layer = lv_color_mix(p->on_secondary_container, p->secondary_container, LV_OPA_10);

    lv_style_set_bg_color(&style_card, p->surface_container_highest);
    lv_style_set_text_color(&style_card, p->on_surface);
    lv_style_set_bg_color(&style_tonal, p->secondary_container);
    lv_style_set_text_color(&style_tonal, p->on_secondary_container);
    lv_style_set_bg_color(&style_tonal_pressed, layer);
    lv_style_set_bg_color(&style_tonal_focus, layer);
    lv_style_set_outline_color(&style_tonal_focus, p->focus);
    lv_style_set_bg_color(&style_tonal_disabled, p->on_surface);
    lv_style_set_text_color(&style_tonal_disabled, p->on_surface);
}

static void styles_init(const palette_t * p)
{
    lv_style_init(&style_card);
    lv_style_set_bg_opa(&style_card, LV_OPA_COVER);
    lv_style_set_radius(&style_card, RADIUS_MD);

    text_style_init(&style_title_medium, &lv_font_montserrat_16, 24);
    text_style_init(&style_headline_small, &lv_font_montserrat_24, 32);
    text_style_init(&style_label_large, &lv_font_montserrat_14, 20);

    /* On top of the default theme's button: MD3 shape, no shadow. */
    lv_style_init(&style_button);
    lv_style_set_height(&style_button, BUTTON_HEIGHT);
    lv_style_set_radius(&style_button, LV_RADIUS_CIRCLE);
    lv_style_set_pad_hor(&style_button, SPACE_XL);
    lv_style_set_pad_ver(&style_button, 0);
    lv_style_set_shadow_width(&style_button, 0);

    lv_style_init(&style_tonal);
    lv_style_init(&style_tonal_pressed);            /* state layer instead of the */
    lv_style_set_recolor_opa(&style_tonal_pressed, LV_OPA_TRANSP);   /* theme's dark */
    lv_style_set_transform_width(&style_tonal_pressed, 0);           /* recolour and */
    lv_style_set_transform_height(&style_tonal_pressed, 0);          /* grow         */
    lv_style_init(&style_tonal_focus);
    lv_style_set_outline_width(&style_tonal_focus, FOCUS_WIDTH);
    lv_style_set_outline_opa(&style_tonal_focus, LV_OPA_COVER);
    lv_style_init(&style_tonal_disabled);
    lv_style_set_bg_opa(&style_tonal_disabled, 31);                  /* MD3: 12 % */
    lv_style_set_text_opa(&style_tonal_disabled, 97);                /* MD3: 38 % */
    lv_style_set_recolor_opa(&style_tonal_disabled, LV_OPA_TRANSP);

    styles_set_colors(p);
}

/* ---- Per card ---- */

static lv_obj_t * card_lvgl(lv_obj_t * parent)
{
    lv_obj_t * card = lv_obj_create(parent);
    lv_obj_remove_style_all(card);                      /* no theme padding, border, scrollbar */
    lv_obj_add_style(card, &style_card, 0);
    lv_obj_set_scrollable(card, false);
    lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);  /* in a row: lv_obj_set_flex_grow */
    lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(card, SPACE_LG, 0);
    lv_obj_set_style_pad_row(card, SPACE_SM, 0);        /* a column's gap is pad_row */

    lv_obj_t * header = lv_obj_create(card);
    lv_obj_remove_style_all(header);
    lv_obj_set_scrollable(header, false);
    lv_obj_set_size(header, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);   /* track too */
    lv_obj_set_style_pad_column(header, SPACE_SM, 0);   /* a row's gap is pad_column */

    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "Temperature");
    lv_obj_add_style(title, &style_title_medium, 0);
    lv_obj_t * value = lv_label_create(header);
    lv_label_set_text(value, "37.2 °C");
    lv_obj_add_style(value, &style_headline_small, 0);

    lv_obj_t * btn = lv_button_create(card);
    lv_obj_add_style(btn, &style_button, 0);
    lv_obj_add_style(btn, &style_tonal, 0);
    lv_obj_add_style(btn, &style_tonal_pressed, LV_STATE_PRESSED);
    lv_obj_add_style(btn, &style_tonal_focus, LV_STATE_FOCUS_KEY);
    lv_obj_add_style(btn, &style_tonal_disabled, LV_STATE_DISABLED);
    lv_obj_t * label = lv_label_create(btn);
    lv_label_set_text(label, "Calibrate");
    lv_obj_add_style(label, &style_label_large, 0);
    lv_obj_center(label);

    return card;
}

/*===========================================================================
 * The comparison
 *=========================================================================*/

/* The card's objects: the card, the header and its two labels, the button
 * and its label. */
enum { CARD, HEADER, TITLE, VALUE, BUTTON, BUTTON_LABEL, PART_COUNT };

typedef struct {
    int32_t x, y, width, height;
} rect_t;

/* What one colour mode resolves to. */
typedef struct {
    uint32_t card_bg, title_text, value_text, button_bg, button_text, button_pressed_bg;
} colors_t;

static rect_t lvgl_rects[PART_COUNT];
static colors_t lvgl_light, lvgl_dark;
static bool lvgl_recorded;

static void collect(lv_obj_t * card, lv_obj_t * parts[PART_COUNT])
{
    lv_obj_t * header = lv_obj_get_child(card, 0);
    lv_obj_t * btn = lv_obj_get_child(card, 1);

    parts[CARD] = card;
    parts[HEADER] = header;
    parts[TITLE] = lv_obj_get_child(header, 0);
    parts[VALUE] = lv_obj_get_child(header, 1);
    parts[BUTTON] = btn;
    parts[BUTTON_LABEL] = lv_obj_get_child(btn, 0);
}

static colors_t measure_colors(lv_obj_t * parts[PART_COUNT])
{
    colors_t c;
    lv_obj_t * btn = parts[BUTTON];

    c.card_bg = lv_color_to_u32(lv_obj_get_style_bg_color(parts[CARD], LV_PART_MAIN));
    c.title_text = lv_color_to_u32(lv_obj_get_style_text_color(parts[TITLE], LV_PART_MAIN));
    c.value_text = lv_color_to_u32(lv_obj_get_style_text_color(parts[VALUE], LV_PART_MAIN));
    c.button_bg = lv_color_to_u32(lv_obj_get_style_bg_color(btn, LV_PART_MAIN));
    c.button_text = lv_color_to_u32(lv_obj_get_style_text_color(parts[BUTTON_LABEL], LV_PART_MAIN));

    lv_obj_add_state(btn, LV_STATE_PRESSED);
    c.button_pressed_bg = lv_color_to_u32(lv_obj_get_style_bg_color(btn, LV_PART_MAIN));
    lv_obj_remove_state(btn, LV_STATE_PRESSED);
    return c;
}

static void assert_same_colors(const colors_t * expected, const colors_t * actual)
{
    TRLV_ASSERT_INT((int32_t)actual->card_bg, (int32_t)expected->card_bg);
    TRLV_ASSERT_INT((int32_t)actual->title_text, (int32_t)expected->title_text);
    TRLV_ASSERT_INT((int32_t)actual->value_text, (int32_t)expected->value_text);
    TRLV_ASSERT_INT((int32_t)actual->button_bg, (int32_t)expected->button_bg);
    TRLV_ASSERT_INT((int32_t)actual->button_text, (int32_t)expected->button_text);
    TRLV_ASSERT_INT((int32_t)actual->button_pressed_bg, (int32_t)expected->button_pressed_bg);
}

/* The bare-LVGL card under LVGL's default theme: record its layout and its
 * colours in light and dark. */
static void lvgl_card_records(lv_obj_t * screen)
{
    styles_init(&palette_light);

    lv_obj_t * parts[PART_COUNT];
    collect(card_lvgl(screen), parts);

    for(int i = 0; i < PART_COUNT; i++) {
        lvgl_rects[i].x = trlv_test_x(parts[i]);
        lvgl_rects[i].y = trlv_test_y(parts[i]);
        lvgl_rects[i].width = trlv_test_width(parts[i]);
        lvgl_rects[i].height = trlv_test_height(parts[i]);
    }
    lvgl_light = measure_colors(parts);

    styles_set_colors(&palette_dark);
    lv_obj_report_style_change(NULL);
    lvgl_dark = measure_colors(parts);

    /* Sanity: the card is laid out and the labels have their fonts. */
    TRLV_ASSERT_INT(lvgl_rects[CARD].width, 800);
    TRLV_ASSERT_INT(lvgl_rects[BUTTON].height, BUTTON_HEIGHT);
    TRLV_ASSERT_INT((int32_t)(lv_obj_get_style_text_font(parts[VALUE], LV_PART_MAIN) == &lv_font_montserrat_24), 1);

    lvgl_recorded = true;
}

/* The TreLVis card lays out exactly like the bare-LVGL one and resolves the
 * same fonts and colours, in light and after a switch to dark. */
static void trelvis_card_matches_lvgl(lv_obj_t * screen)
{
    TRLV_ASSERT_INT((int32_t)lvgl_recorded, 1);
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);

    lv_obj_t * parts[PART_COUNT];
    collect(card_trelvis(screen), parts);

    for(int i = 0; i < PART_COUNT; i++) {
        TRLV_ASSERT_X(parts[i], lvgl_rects[i].x);
        TRLV_ASSERT_Y(parts[i], lvgl_rects[i].y);
        TRLV_ASSERT_WIDTH(parts[i], lvgl_rects[i].width);
        TRLV_ASSERT_HEIGHT(parts[i], lvgl_rects[i].height);
    }

    TRLV_ASSERT_INT((int32_t)(lv_obj_get_style_text_font(parts[TITLE], LV_PART_MAIN) == &lv_font_montserrat_16), 1);
    TRLV_ASSERT_INT((int32_t)(lv_obj_get_style_text_font(parts[VALUE], LV_PART_MAIN) == &lv_font_montserrat_24), 1);
    TRLV_ASSERT_INT((int32_t)(lv_obj_get_style_text_font(parts[BUTTON_LABEL], LV_PART_MAIN) == &lv_font_montserrat_14), 1);

    colors_t light = measure_colors(parts);
    assert_same_colors(&lvgl_light, &light);

    trlv_theme_set_color_mode(&tokens_dark);
    colors_t dark = measure_colors(parts);
    assert_same_colors(&lvgl_dark, &dark);
}

int main(void)
{
    TRLV_TEST_RUN(lvgl_card_records);
    TRLV_TEST_RUN(trelvis_card_matches_lvgl);
    return trlv_test_finish();
}
