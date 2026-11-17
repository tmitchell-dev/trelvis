/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Fill modifiers: trlv_surface. Colours come from the token template's
 * light and dark schemes.
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

int main(void)
{
    TRLV_TEST_RUN(every_surface_and_its_content);
    TRLV_TEST_RUN(surfaces_recolour);
    TRLV_TEST_RUN(tone_only);
    TRLV_TEST_RUN(a_second_call_replaces_the_first);
    TRLV_TEST_RUN(own_text_colour_wins);
    TRLV_TEST_RUN(works_on_a_themed_object);
    TRLV_TEST_RUN(invalid_surface_changes_nothing);
    return trlv_test_finish();
}
