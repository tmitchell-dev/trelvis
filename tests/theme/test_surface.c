/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Fill modifiers: trlv_surface and trlv_card (custom colours have their own
 * file). Colours come from the token template's light and dark schemes.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    return screen;
}

/* Let LVGL's style transitions finish. */
static void settle(void)
{
    for(int i = 0; i < 10; i++) {
        lv_tick_inc(100);
        lv_timer_handler();
    }
}

static lv_color_t bg_of(lv_obj_t * obj)
{
    return lv_obj_get_style_bg_color(obj, LV_PART_MAIN);
}

static lv_color_t text_of(lv_obj_t * obj)
{
    return lv_obj_get_style_text_color(obj, LV_PART_MAIN);
}

/* A column with a label two levels down, to check inheritance. */
static lv_obj_t * column_with_label(lv_obj_t * parent, lv_obj_t ** label)
{
    lv_obj_t * column = trlv_column(parent, TRLV_SPACE_SM);
    lv_obj_t * row = trlv_row(column, TRLV_SPACE_SM);
    *label = lv_label_create(row);
    return column;
}

/*---------------------------------------------------------------------------
 * trlv_surface
 *-------------------------------------------------------------------------*/

typedef struct {
    trlv_surface_t surface;
    lv_color_t container;
    lv_color_t content;
} surface_case_t;

static void check_every_surface(lv_obj_t * parent, const trlv_color_tokens_t * c)
{
    const surface_case_t cases[] = {
        { TRLV_SURFACE_SURFACE, c->surface, c->on_surface },
        { TRLV_SURFACE_CONTAINER_LOWEST, c->surface_container_lowest, c->on_surface },
        { TRLV_SURFACE_CONTAINER_LOW, c->surface_container_low, c->on_surface },
        { TRLV_SURFACE_CONTAINER, c->surface_container, c->on_surface },
        { TRLV_SURFACE_CONTAINER_HIGH, c->surface_container_high, c->on_surface },
        { TRLV_SURFACE_CONTAINER_HIGHEST, c->surface_container_highest, c->on_surface },
        { TRLV_SURFACE_PRIMARY_CONTAINER, c->primary_container, c->on_primary_container },
        { TRLV_SURFACE_SECONDARY_CONTAINER, c->secondary_container, c->on_secondary_container },
        { TRLV_SURFACE_TERTIARY_CONTAINER, c->tertiary_container, c->on_tertiary_container },
        { TRLV_SURFACE_ERROR_CONTAINER, c->error_container, c->on_error_container },
        { TRLV_SURFACE_INVERSE, c->inverse_surface, c->inverse_on_surface },
    };
    TRLV_ASSERT_INT((int32_t)(sizeof(cases) / sizeof(cases[0])), TRLV_SURFACE_COUNT);

    for(uint32_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        lv_obj_t * label;
        lv_obj_t * column = column_with_label(parent, &label);
        trlv_surface(column, cases[i].surface);

        ASSERT_COLOR(bg_of(column), cases[i].container);
        TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(column, LV_PART_MAIN), LV_OPA_COVER);
        ASSERT_COLOR(text_of(label), cases[i].content);
    }
}

/* Every surface sets its container colour, opaque, and its content colour
 * reaches a label inside. */
static void every_surface_and_its_content(lv_obj_t * screen)
{
    check_every_surface(setup(screen), &tokens_light);
}

/* Existing surfaces recolour on a colour-mode switch. */
static void surfaces_recolour(lv_obj_t * screen)
{
    lv_obj_t * label;
    lv_obj_t * column = column_with_label(setup(screen), &label);
    trlv_surface(column, TRLV_SURFACE_PRIMARY_CONTAINER);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();

    ASSERT_COLOR(bg_of(column), tokens_dark.primary_container);
    ASSERT_COLOR(text_of(label), tokens_dark.on_primary_container);

    /* New surfaces made in dark mode take the dark colours too. */
    check_every_surface(screen, &tokens_dark);
}

/* Tone only: no radius, border or padding of its own. */
static void tone_only(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box(setup(screen));
    lv_obj_set_size(box, 40, 20);
    trlv_surface(box, TRLV_SURFACE_CONTAINER);

    TRLV_ASSERT_INT(lv_obj_get_style_radius(box, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(box, LV_PART_MAIN), 0);
    TRLV_ASSERT_RECT(box, 0, 0, 40, 20);
}

/* A second call replaces the first, in place: a project style added after
 * the surface still wins when the surface is changed. */
static void a_second_call_replaces_the_first(lv_obj_t * screen)
{
    static const lv_style_const_prop_t project_props[] = {
        LV_STYLE_CONST_TEXT_COLOR(LV_COLOR_MAKE(0x12, 0x34, 0x56)),
        LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(project_style, project_props);

    lv_obj_t * label;
    lv_obj_t * column = column_with_label(setup(screen), &label);
    trlv_surface(column, TRLV_SURFACE_ERROR_CONTAINER);
    trlv_surface(column, TRLV_SURFACE_CONTAINER_HIGH);
    ASSERT_COLOR(bg_of(column), tokens_light.surface_container_high);
    ASSERT_COLOR(text_of(label), tokens_light.on_surface);

    lv_obj_add_style(column, &project_style, LV_PART_MAIN);
    trlv_surface(column, TRLV_SURFACE_INVERSE);
    ASSERT_COLOR(bg_of(column), tokens_light.inverse_surface);
    ASSERT_COLOR(text_of(label), lv_color_hex(0x123456));
}

/* A label with its own text colour keeps it. */
static void own_text_colour_wins(lv_obj_t * screen)
{
    lv_obj_t * label;
    lv_obj_t * column = column_with_label(setup(screen), &label);
    lv_obj_set_style_text_color(label, tokens_light.primary, LV_PART_MAIN);
    trlv_surface(column, TRLV_SURFACE_CONTAINER);

    ASSERT_COLOR(text_of(label), tokens_light.primary);
}

/* On a plain lv_obj it wins over the theme's container colour. */
static void works_on_a_themed_object(lv_obj_t * screen)
{
    lv_obj_t * obj = lv_obj_create(setup(screen));
    trlv_surface(obj, TRLV_SURFACE_TERTIARY_CONTAINER);

    ASSERT_COLOR(bg_of(obj), tokens_light.tertiary_container);
}

/* A value outside the enum changes nothing. */
static void invalid_surface_changes_nothing(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box(setup(screen));
    trlv_surface(box, TRLV_SURFACE_CONTAINER);
    trlv_surface(box, TRLV_SURFACE_COUNT);

    ASSERT_COLOR(bg_of(box), tokens_light.surface_container);
}

/*---------------------------------------------------------------------------
 * trlv_card
 *-------------------------------------------------------------------------*/

static void assert_shadow(lv_obj_t * obj, const trlv_shadow_t * s, lv_color_t color)
{
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_width(obj, LV_PART_MAIN), s->blur);
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_spread(obj, LV_PART_MAIN), s->spread);
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_offset_x(obj, LV_PART_MAIN), s->offset_x);
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_offset_y(obj, LV_PART_MAIN), s->offset_y);
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_opa(obj, LV_PART_MAIN), s->opa);
    ASSERT_COLOR(lv_obj_get_style_shadow_color(obj, LV_PART_MAIN), color);
}

static void assert_no_shadow(lv_obj_t * obj)
{
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_width(obj, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_opa(obj, LV_PART_MAIN), LV_OPA_TRANSP);
}

/* Shared checks: tone, radius MD, the on_surface content colour. */
static void assert_card_tone(lv_obj_t * card, lv_obj_t * label, lv_color_t container,
                             const trlv_color_tokens_t * c)
{
    ASSERT_COLOR(bg_of(card), container);
    TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(card, LV_PART_MAIN), LV_OPA_COVER);
    TRLV_ASSERT_INT(lv_obj_get_style_radius(card, LV_PART_MAIN), tokens_regular.radius[TRLV_RADIUS_MD]);
    ASSERT_COLOR(text_of(label), c->on_surface);
}

static void card_filled(lv_obj_t * screen)
{
    lv_obj_t * label;
    lv_obj_t * card = column_with_label(setup(screen), &label);
    trlv_card(card, TRLV_CARD_FILLED);

    assert_card_tone(card, label, tokens_light.surface_container_highest, &tokens_light);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(card, LV_PART_MAIN), 0);
    assert_no_shadow(card);
}

static void card_elevated(lv_obj_t * screen)
{
    lv_obj_t * label;
    lv_obj_t * card = column_with_label(setup(screen), &label);
    trlv_card(card, TRLV_CARD_ELEVATED);

    assert_card_tone(card, label, tokens_light.surface_container_low, &tokens_light);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(card, LV_PART_MAIN), 0);
    assert_shadow(card, &tokens_regular.elevation[TRLV_ELEVATION_1], tokens_light.shadow);
}

static void card_outlined(lv_obj_t * screen)
{
    lv_obj_t * label;
    lv_obj_t * card = column_with_label(setup(screen), &label);
    trlv_card(card, TRLV_CARD_OUTLINED);

    assert_card_tone(card, label, tokens_light.surface, &tokens_light);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(card, LV_PART_MAIN), tokens_regular.border_width);
    ASSERT_COLOR(lv_obj_get_style_border_color(card, LV_PART_MAIN), tokens_light.outline_variant);
    TRLV_ASSERT_INT(lv_obj_get_style_border_opa(card, LV_PART_MAIN), LV_OPA_COVER);
    assert_no_shadow(card);
}

/* Padding stays explicit: a filled card adds none. */
static void card_adds_no_padding(lv_obj_t * screen)
{
    lv_obj_t * card = trlv_column(setup(screen), TRLV_SPACE_NONE);
    lv_obj_t * child = trlv_box(card);
    lv_obj_set_size(child, 30, 20);
    trlv_card(card, TRLV_CARD_FILLED);

    TRLV_ASSERT_RECT(card, 0, 0, 30, 20);
    TRLV_ASSERT_RECT(child, 0, 0, 30, 20);
}

/* An outlined card's border is a Figma inside stroke included in layout
 * (CSS border-box): it insets the content by its width, on top of the
 * padding, and a hugging card grows by it. */
static void card_border_insets_the_content(lv_obj_t * screen)
{
    int32_t b = tokens_regular.border_width;
    int32_t p = tokens_regular.space[TRLV_SPACE_LG];
    lv_obj_t * card = trlv_column(setup(screen), TRLV_SPACE_NONE);
    lv_obj_t * child = trlv_box(card);
    lv_obj_set_size(child, 30, 20);
    trlv_card(card, TRLV_CARD_OUTLINED);
    trlv_pad(card, TRLV_SPACE_LG);

    TRLV_ASSERT_INT(b > 0, 1);
    TRLV_ASSERT_RECT(card, 0, 0, 30 + 2 * (b + p), 20 + 2 * (b + p));
    TRLV_ASSERT_RECT(child, b + p, b + p, 30, 20);
}

/* Cards recolour on a colour-mode switch, shadow and border included. */
static void cards_recolour(lv_obj_t * screen)
{
    lv_obj_t * label;
    lv_obj_t * elevated = column_with_label(setup(screen), &label);
    trlv_card(elevated, TRLV_CARD_ELEVATED);
    lv_obj_t * outlined = trlv_column(screen, TRLV_SPACE_SM);
    trlv_card(outlined, TRLV_CARD_OUTLINED);

    trlv_theme_set_color_mode(&tokens_dark);
    settle();

    assert_card_tone(elevated, label, tokens_dark.surface_container_low, &tokens_dark);
    ASSERT_COLOR(lv_obj_get_style_shadow_color(elevated, LV_PART_MAIN), tokens_dark.shadow);
    ASSERT_COLOR(bg_of(outlined), tokens_dark.surface);
    ASSERT_COLOR(lv_obj_get_style_border_color(outlined, LV_PART_MAIN), tokens_dark.outline_variant);
}

/* Cards, surfaces and custom colours are one group and replace each other;
 * a card replaced by a filled card or a surface loses its border and
 * shadow. */
static void cards_share_the_fill_group(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box(setup(screen));
    trlv_card(box, TRLV_CARD_OUTLINED);
    trlv_card(box, TRLV_CARD_ELEVATED);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(box, LV_PART_MAIN), 0);
    ASSERT_COLOR(bg_of(box), tokens_light.surface_container_low);

    trlv_surface(box, TRLV_SURFACE_PRIMARY_CONTAINER);
    ASSERT_COLOR(bg_of(box), tokens_light.primary_container);
    TRLV_ASSERT_INT(lv_obj_get_style_shadow_width(box, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_radius(box, LV_PART_MAIN), 0);

#if TRLV_CUSTOM_COLOR_MAX > 0
    trlv_card(box, TRLV_CARD_FILLED);
    trlv_surface_custom(box, TOKEN_CUSTOM_WARNING, TRLV_CUSTOM_SURFACE_COLOR);
    ASSERT_COLOR(bg_of(box), tokens_light.custom[TOKEN_CUSTOM_WARNING].color);
    TRLV_ASSERT_INT(lv_obj_get_style_radius(box, LV_PART_MAIN), 0);

    trlv_card(box, TRLV_CARD_OUTLINED);
    ASSERT_COLOR(bg_of(box), tokens_light.surface);
    ASSERT_COLOR(text_of(box), tokens_light.on_surface);
#endif
}

/* On a plain lv_obj a filled card overrides the theme's border. */
static void card_on_a_themed_object(lv_obj_t * screen)
{
    lv_obj_t * obj = lv_obj_create(setup(screen));
    trlv_card(obj, TRLV_CARD_FILLED);

    ASSERT_COLOR(bg_of(obj), tokens_light.surface_container_highest);
    TRLV_ASSERT_INT(lv_obj_get_style_border_width(obj, LV_PART_MAIN), 0);
    TRLV_ASSERT_INT(lv_obj_get_style_radius(obj, LV_PART_MAIN), tokens_regular.radius[TRLV_RADIUS_MD]);
}

/* A value outside the enum changes nothing. */
static void invalid_card_changes_nothing(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box(setup(screen));
    trlv_card(box, TRLV_CARD_OUTLINED);
    trlv_card(box, TRLV_CARD_COUNT);

    TRLV_ASSERT_INT(lv_obj_get_style_border_width(box, LV_PART_MAIN), tokens_regular.border_width);
}

int main(void)
{
    TRLV_TEST_RUN(card_filled);
    TRLV_TEST_RUN(card_elevated);
    TRLV_TEST_RUN(card_outlined);
    TRLV_TEST_RUN(card_adds_no_padding);
    TRLV_TEST_RUN(card_border_insets_the_content);
    TRLV_TEST_RUN(cards_recolour);
    TRLV_TEST_RUN(cards_share_the_fill_group);
    TRLV_TEST_RUN(card_on_a_themed_object);
    TRLV_TEST_RUN(invalid_card_changes_nothing);
    TRLV_TEST_RUN(every_surface_and_its_content);
    TRLV_TEST_RUN(surfaces_recolour);
    TRLV_TEST_RUN(tone_only);
    TRLV_TEST_RUN(a_second_call_replaces_the_first);
    TRLV_TEST_RUN(own_text_colour_wins);
    TRLV_TEST_RUN(works_on_a_themed_object);
    TRLV_TEST_RUN(invalid_surface_changes_nothing);
    return trlv_test_finish();
}
