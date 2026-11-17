/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * The demo (examples/demo) on a fixed-size display: the frame (bar, nav and
 * content), moving between sections, each section's layout, nothing
 * overflowing its parent and no TreLVis warnings while it is built. Text
 * sizes are computed from the font with lv_text_get_size, so the tests
 * don't depend on the fonts' metrics. The object trees are described at the
 * top of trlv_demo.c and of each section's file.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"
#include "trlv_demo.h"

#include <string.h>

/* Token values from the template's Regular layout. */
#define FRAME_PAD    16   /* TRLV_SPACE_LG: bar and content padding */
#define BAR_GAP      8    /* TRLV_SPACE_SM */
#define NAV_GAP      4    /* TRLV_SPACE_XS */
#define SECTION_GAP  16   /* TRLV_SPACE_LG: between a section's parts */
#define READOUTS_GAP 8    /* TRLV_SPACE_SM */
#define READOUT_GAP  12   /* TRLV_SPACE_MD */
#define ABOUT_GAP    4    /* TRLV_SPACE_XS */
#define ACTIONS_GAP  8    /* TRLV_SPACE_SM */

/* The sections, in the order of the nav buttons. */
static const struct {
    const char * name;
    lv_obj_t * (*create)(lv_obj_t * parent);
} sections[] = {
    { "Readouts", trlv_demo_readouts_create },
    { "Surfaces", trlv_demo_surfaces_create },
    { "Cards", trlv_demo_cards_create },
};

/* Indexes into sections[]. */
enum { SECTION_READOUTS, SECTION_SURFACES, SECTION_CARDS };

#define SECTION_COUNT ((int32_t)(sizeof(sections) / sizeof(sections[0])))

/* Children of the demo, of its bar and of the Readouts section, in order. */
enum { DEMO_BAR, DEMO_CONTENT };
enum { BAR_TITLE, BAR_NAV };
enum { HEADER, READOUTS, ABOUT, ACTIONS };

static int trelvis_warnings;

static void log_cb(lv_log_level_t level, const char * buf)
{
    if(level >= LV_LOG_LEVEL_WARN && strstr(buf, "trlv_") != NULL) trelvis_warnings++;
}

/* lv_deinit clears the log callback, so it is registered in every test. */
static lv_obj_t * build(lv_obj_t * screen)
{
    trelvis_warnings = 0;
    lv_log_register_print_cb(log_cb);
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    return trlv_demo_create(screen);
}

static lv_obj_t * bar(lv_obj_t * demo)
{
    return lv_obj_get_child(demo, DEMO_BAR);
}

static lv_obj_t * nav(lv_obj_t * demo)
{
    return lv_obj_get_child(bar(demo), BAR_NAV);
}

static lv_obj_t * content(lv_obj_t * demo)
{
    return lv_obj_get_child(demo, DEMO_CONTENT);
}

/* The section on show. */
static lv_obj_t * section(lv_obj_t * demo)
{
    return lv_obj_get_child(content(demo), 0);
}

/* Choose a section the way a user does, by clicking its nav button. */
static void select_section(lv_obj_t * demo, int32_t index)
{
    lv_obj_send_event(lv_obj_get_child(nav(demo), index), LV_EVENT_CLICKED, NULL);
}

/* Height of a one-line text: its font's line height (LVGL adds line
 * spacing only between lines). Roles use different fonts, so it is
 * measured per label. */
static int32_t line_height(lv_obj_t * label)
{
    return lv_font_get_line_height(lv_obj_get_style_text_font(label, LV_PART_MAIN));
}

static int32_t text_width(lv_obj_t * label)
{
    lv_point_t size;
    lv_text_get_size(&size, lv_label_get_text(label), lv_obj_get_style_text_font(label, LV_PART_MAIN),
                     lv_obj_get_style_text_letter_space(label, LV_PART_MAIN), 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);
    return size.x;
}

static int32_t wrapped_height(lv_obj_t * label, int32_t max_width)
{
    lv_point_t size;
    lv_text_get_size(&size, lv_label_get_text(label), lv_obj_get_style_text_font(label, LV_PART_MAIN),
                     lv_obj_get_style_text_letter_space(label, LV_PART_MAIN),
                     lv_obj_get_style_text_line_space(label, LV_PART_MAIN), max_width, LV_TEXT_FLAG_NONE);
    return size.y;
}

static bool same_color(lv_color_t a, lv_color_t b)
{
    return lv_color_eq(a, b);
}

/* Every descendant of `obj` lies inside its parent's outer box, except
 * along a direction the parent scrolls in. */
static void assert_nothing_overflows(lv_obj_t * obj, const char * path)
{
    uint32_t count = lv_obj_get_child_count(obj);
    lv_dir_t scroll = lv_obj_is_scrollable(obj) ? lv_obj_get_scroll_dir(obj) : LV_DIR_NONE;

    for(uint32_t i = 0; i < count; i++) {
        lv_obj_t * child = lv_obj_get_child(obj, (int32_t)i);
        char child_path[64];
        char what[96];
        lv_snprintf(child_path, sizeof(child_path), "%s/%" LV_PRIu32, path, i);

        lv_snprintf(what, sizeof(what), "%s is inside on the left and top", child_path);
        trlv_test_check_int(__FILE__, __LINE__, what, true,
                            trlv_test_x(child) >= 0 && trlv_test_y(child) >= 0);
        if(!(scroll & LV_DIR_HOR)) {
            lv_snprintf(what, sizeof(what), "%s is inside on the right", child_path);
            trlv_test_check_int(__FILE__, __LINE__, what, true,
                                trlv_test_x(child) + trlv_test_width(child) <= trlv_test_width(obj));
        }
        if(!(scroll & LV_DIR_VER)) {
            lv_snprintf(what, sizeof(what), "%s is inside at the bottom", child_path);
            trlv_test_check_int(__FILE__, __LINE__, what, true,
                                trlv_test_y(child) + trlv_test_height(child) <= trlv_test_height(obj));
        }

        assert_nothing_overflows(child, child_path);
    }
}

/*---------------------------------------------------------------------------
 * The frame
 *-------------------------------------------------------------------------*/

/* The demo fills the screen: the bar across the top hugs its content, and
 * the content takes the rest of the height. */
static void frame_fills_the_screen(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    lv_obj_t * title = lv_obj_get_child(bar(demo), BAR_TITLE);
    int32_t bar_height = FRAME_PAD + line_height(title) + BAR_GAP + trlv_test_height(nav(demo)) + FRAME_PAD;

    TRLV_ASSERT_RECT(demo, 0, 0, 800, 480);
    TRLV_ASSERT_RECT(bar(demo), 0, 0, 800, bar_height);
    TRLV_ASSERT_RECT(title, FRAME_PAD, FRAME_PAD, text_width(title), line_height(title));
    TRLV_ASSERT_RECT(nav(demo), FRAME_PAD, FRAME_PAD + line_height(title) + BAR_GAP,
                     800 - 2 * FRAME_PAD, trlv_test_height(lv_obj_get_child(nav(demo), 0)));
    TRLV_ASSERT_RECT(content(demo), 0, bar_height, 800, 480 - bar_height);
    TRLV_ASSERT_RECT(section(demo), FRAME_PAD, FRAME_PAD, 800 - 2 * FRAME_PAD, trlv_test_height(section(demo)));
}

/* The bar is a Surface Container, and its text gets on_surface by
 * inheritance. */
static void bar_is_a_surface_container(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    lv_obj_t * title = lv_obj_get_child(bar(demo), BAR_TITLE);

    TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(bar(demo), LV_PART_MAIN),
                               trlv_color()->surface_container), true);
    TRLV_ASSERT_INT(same_color(lv_obj_get_style_text_color(title, LV_PART_MAIN),
                               trlv_color()->on_surface), true);
}

/* One nav button per section, named after it and side by side; the
 * selected one is tonal (secondary_container), the others are text
 * buttons (no fill). The first section is shown at the start. */
static void nav_has_a_button_per_section(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    int32_t x = 0;

    TRLV_ASSERT_INT(lv_obj_get_child_count(nav(demo)), SECTION_COUNT);
    for(int32_t i = 0; i < SECTION_COUNT; i++) {
        lv_obj_t * btn = lv_obj_get_child(nav(demo), i);
        lv_obj_t * label = lv_obj_get_child(btn, 0);

        TRLV_ASSERT_INT(strcmp(lv_label_get_text(label), sections[i].name), 0);
        TRLV_ASSERT_X(btn, x);
        TRLV_ASSERT_Y(btn, 0);
        x += trlv_test_width(btn) + NAV_GAP;
    }
    TRLV_ASSERT_INT(lv_obj_get_child_count(content(demo)), 1);
}

static void assert_selected(lv_obj_t * demo, int32_t selected)
{
    for(int32_t i = 0; i < SECTION_COUNT; i++) {
        lv_obj_t * btn = lv_obj_get_child(nav(demo), i);
        if(i == selected) {
            TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(btn, LV_PART_MAIN), LV_OPA_COVER);
            TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(btn, LV_PART_MAIN),
                                       trlv_color()->secondary_container), true);
        }
        else {
            TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(btn, LV_PART_MAIN), LV_OPA_TRANSP);
        }
    }
}

/* Clicking a nav button selects it and replaces the section with the one
 * it names (compared with a copy built on its own). The content scrolls
 * back to the top. */
static void nav_shows_the_chosen_section(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    lv_obj_t * reference_parent = trlv_box(screen);
    lv_obj_set_size(reference_parent, 800, 480);

    assert_selected(demo, 0);
    for(int32_t i = SECTION_COUNT - 1; i >= 0; i--) {
        lv_obj_t * reference;

        lv_obj_scroll_to_y(content(demo), 40, LV_ANIM_OFF);
        select_section(demo, i);
        lv_obj_clean(reference_parent);
        reference = sections[i].create(reference_parent);

        assert_selected(demo, i);
        TRLV_ASSERT_INT(lv_obj_get_child_count(content(demo)), 1);
        TRLV_ASSERT_INT(lv_obj_get_child_count(section(demo)), lv_obj_get_child_count(reference));
        TRLV_ASSERT_INT(lv_obj_get_scroll_y(content(demo)), 0);
    }
}

/* Every section fits, nothing overflows its parent (except into a scroll
 * direction) and the demo raises no TreLVis warnings: it sizes every parent
 * before filling its children. Run on each display size. */
static void every_section_fits_and_builds_quietly(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);

    for(int32_t i = 0; i < SECTION_COUNT; i++) {
        char path[32];
        select_section(demo, i);
        lv_snprintf(path, sizeof(path), "demo[%s]", sections[i].name);
        assert_nothing_overflows(demo, path);
        TRLV_ASSERT_WIDTH(section(demo), trlv_test_width(screen) - 2 * FRAME_PAD);
    }
    TRLV_ASSERT_INT(trelvis_warnings, 0);
}

/*---------------------------------------------------------------------------
 * Readouts
 *-------------------------------------------------------------------------*/

static lv_obj_t * readouts_part(lv_obj_t * demo, int32_t index)
{
    return lv_obj_get_child(section(demo), index);
}

/* The section's four parts are stacked the gap apart; all but the actions
 * fill the width. */
static void readouts_stack_their_parts(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    int32_t width = 800 - 2 * FRAME_PAD;
    int32_t y = 0;

    for(int32_t i = HEADER; i <= ACTIONS; i++) {
        lv_obj_t * part = readouts_part(demo, i);
        TRLV_ASSERT_X(part, 0);
        TRLV_ASSERT_Y(part, y);
        y += trlv_test_height(part) + SECTION_GAP;
    }
    TRLV_ASSERT_HEIGHT(section(demo), y - SECTION_GAP);
    TRLV_ASSERT_WIDTH(readouts_part(demo, HEADER), width);
    TRLV_ASSERT_WIDTH(readouts_part(demo, READOUTS), width);
    TRLV_ASSERT_WIDTH(readouts_part(demo, ABOUT), width);
}

/* The header box hugs the title's height; the title is on the left and the
 * status on the right. */
static void readouts_header_places_title_and_status(lv_obj_t * screen)
{
    lv_obj_t * header = readouts_part(build(screen), HEADER);
    lv_obj_t * title = lv_obj_get_child(header, 0);
    lv_obj_t * status = lv_obj_get_child(header, 1);
    int32_t width = 800 - 2 * FRAME_PAD;
    int32_t height = LV_MAX(line_height(title), line_height(status));

    TRLV_ASSERT_HEIGHT(header, height);
    TRLV_ASSERT_RECT(title, 0, (height - line_height(title)) / 2, text_width(title), line_height(title));
    TRLV_ASSERT_RECT(status, width - text_width(status), (height - line_height(status)) / 2,
                     text_width(status), line_height(status));
}

/* Each readout fills the width: the name grows, so the value sits on the right. */
static void readouts_put_values_on_the_right(lv_obj_t * screen)
{
    lv_obj_t * readouts = readouts_part(build(screen), READOUTS);
    int32_t width = 800 - 2 * FRAME_PAD;
    int32_t y = 0;

    TRLV_ASSERT_INT(lv_obj_get_child_count(readouts), 3);

    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * row = lv_obj_get_child(readouts, i);
        lv_obj_t * name = lv_obj_get_child(row, 0);
        lv_obj_t * value = lv_obj_get_child(row, 1);
        int32_t value_width = text_width(value);
        int32_t height = LV_MAX(line_height(name), line_height(value));

        TRLV_ASSERT_RECT(row, 0, y, width, height);
        TRLV_ASSERT_RECT(name, 0, 0, width - READOUT_GAP - value_width, line_height(name));
        TRLV_ASSERT_RECT(value, width - value_width, 0, value_width, line_height(value));
        y += height + READOUTS_GAP;
    }
    TRLV_ASSERT_HEIGHT(readouts, y - READOUTS_GAP);
}

/* The body text fills the width and wraps onto more lines. */
static void readouts_body_text_wraps(lv_obj_t * screen)
{
    lv_obj_t * about = readouts_part(build(screen), ABOUT);
    lv_obj_t * heading = lv_obj_get_child(about, 0);
    lv_obj_t * body = lv_obj_get_child(about, 1);
    int32_t width = 800 - 2 * FRAME_PAD;

    TRLV_ASSERT_RECT(heading, 0, 0, text_width(heading), line_height(heading));
    TRLV_ASSERT_RECT(body, 0, line_height(heading) + ABOUT_GAP, width, wrapped_height(body, width));
    TRLV_ASSERT_INT(trlv_test_height(body) > line_height(body), true);
}

/* The buttons hug their labels and sit side by side, the gap apart, in a
 * row that fills the width. */
static void readouts_buttons_sit_in_a_row(lv_obj_t * screen)
{
    lv_obj_t * actions = readouts_part(build(screen), ACTIONS);
    int32_t x = 0;

    TRLV_ASSERT_INT(lv_obj_get_child_count(actions), 3);
    TRLV_ASSERT_WIDTH(actions, 800 - 2 * FRAME_PAD);
    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * btn = lv_obj_get_child(actions, i);
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        TRLV_ASSERT_X(btn, x);
        TRLV_ASSERT_Y(btn, 0);
        TRLV_ASSERT_WIDTH(label, text_width(label));
        TRLV_ASSERT_HEIGHT(btn, trlv_test_height(actions));
        x += trlv_test_width(btn) + ACTIONS_GAP;
    }
}

/* Buttons that don't fit across move to the next line, the gap below. */
static void readouts_buttons_wrap(lv_obj_t * screen)
{
    lv_obj_t * actions = readouts_part(build(screen), ACTIONS);
    int32_t width = trlv_test_width(actions);
    int32_t x = 0;
    int32_t y = 0;
    int32_t lines = 1;

    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * btn = lv_obj_get_child(actions, i);
        if(x > 0 && x + trlv_test_width(btn) > width) {
            x = 0;
            y += trlv_test_height(btn) + ACTIONS_GAP;
            lines++;
        }
        TRLV_ASSERT_X(btn, x);
        TRLV_ASSERT_Y(btn, y);
        x += trlv_test_width(btn) + ACTIONS_GAP;
    }
    TRLV_ASSERT_INT(lines > 1, true);   /* they need to wrap at this width */
    TRLV_ASSERT_HEIGHT(actions, y + trlv_test_height(lv_obj_get_child(actions, 2)));
}

/* On a narrow, portrait display the body wraps onto more lines. */
static void readouts_on_a_narrow_display(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    lv_obj_t * body = lv_obj_get_child(readouts_part(demo, ABOUT), 1);
    int32_t width = 320 - 2 * FRAME_PAD;

    TRLV_ASSERT_RECT(demo, 0, 0, 320, 480);
    TRLV_ASSERT_WIDTH(body, width);
    TRLV_ASSERT_HEIGHT(body, wrapped_height(body, width));
}

/* A section works on its own too: on a bare screen it fills the width. */
static void readouts_stand_alone(lv_obj_t * screen)
{
    lv_obj_t * section_obj;

    trelvis_warnings = 0;
    lv_log_register_print_cb(log_cb);
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    section_obj = trlv_demo_readouts_create(screen);

    TRLV_ASSERT_RECT(section_obj, 0, 0, 800, trlv_test_height(section_obj));
    TRLV_ASSERT_INT(trelvis_warnings, 0);
}

/*---------------------------------------------------------------------------
 * Surfaces
 *-------------------------------------------------------------------------*/

/* Groups of the Surfaces section, in order. */
enum { SURFACES_LADDER, SURFACES_CONTAINERS, SURFACES_CUSTOM };

typedef struct {
    lv_color_t fill;
    lv_color_t content;
} color_pair_t;

/* The swatch column of a group (its child 1, after the heading). */
static lv_obj_t * swatches(lv_obj_t * demo, int32_t group)
{
    return lv_obj_get_child(lv_obj_get_child(section(demo), group), 1);
}

/* The swatches of a group are stacked with no gap, fill the width, and
 * have the expected fill. The name on each has the matching content colour
 * through inheritance only: no local text colour. */
static void assert_swatches(lv_obj_t * demo, int32_t group, const color_pair_t * pairs, int32_t count)
{
    lv_obj_t * column = swatches(demo, group);
    int32_t y = 0;

    TRLV_ASSERT_INT(lv_obj_get_child_count(column), count);
    for(int32_t i = 0; i < count; i++) {
        lv_obj_t * swatch = lv_obj_get_child(column, i);
        lv_obj_t * name = lv_obj_get_child(swatch, 0);
        lv_style_value_t local;

        TRLV_ASSERT_RECT(swatch, 0, y, trlv_test_width(column), trlv_test_height(swatch));
        TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(swatch, LV_PART_MAIN), LV_OPA_COVER);
        TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(swatch, LV_PART_MAIN), pairs[i].fill), true);
        TRLV_ASSERT_INT(same_color(lv_obj_get_style_text_color(name, LV_PART_MAIN), pairs[i].content), true);
        TRLV_ASSERT_INT(lv_obj_get_local_style_prop(name, LV_STYLE_TEXT_COLOR, &local, LV_PART_MAIN),
                        LV_STYLE_RES_NOT_FOUND);
        y += trlv_test_height(swatch);
    }
    TRLV_ASSERT_HEIGHT(column, y);
}

/* The ladder runs from Surface to Surface Container Highest, all with
 * on_surface text. */
static void surfaces_show_the_ladder(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    const trlv_color_tokens_t * c = trlv_color();
    const color_pair_t ladder[] = {
        { c->surface, c->on_surface },
        { c->surface_container_lowest, c->on_surface },
        { c->surface_container_low, c->on_surface },
        { c->surface_container, c->on_surface },
        { c->surface_container_high, c->on_surface },
        { c->surface_container_highest, c->on_surface },
    };

    select_section(demo, SECTION_SURFACES);
    assert_swatches(demo, SURFACES_LADDER, ladder, 6);
}

/* The accent containers and the inverse surface, each with its own
 * content colour. */
static void surfaces_show_the_containers(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    const trlv_color_tokens_t * c = trlv_color();
    const color_pair_t containers[] = {
        { c->primary_container, c->on_primary_container },
        { c->secondary_container, c->on_secondary_container },
        { c->tertiary_container, c->on_tertiary_container },
        { c->error_container, c->on_error_container },
        { c->inverse_surface, c->inverse_on_surface },
    };

    select_section(demo, SECTION_SURFACES);
    assert_swatches(demo, SURFACES_CONTAINERS, containers, 5);
}

/* Each of the template's custom colours shows as its strong colour and its
 * container, and together with the two groups above every
 * TRLV_SURFACE_* is shown. */
static void surfaces_show_the_custom_colors(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    const trlv_custom_color_t * custom = trlv_color()->custom;
    const color_pair_t pairs[] = {
        { custom[TOKEN_CUSTOM_SUCCESS].color, custom[TOKEN_CUSTOM_SUCCESS].on_color },
        { custom[TOKEN_CUSTOM_SUCCESS].color_container, custom[TOKEN_CUSTOM_SUCCESS].on_color_container },
        { custom[TOKEN_CUSTOM_WARNING].color, custom[TOKEN_CUSTOM_WARNING].on_color },
        { custom[TOKEN_CUSTOM_WARNING].color_container, custom[TOKEN_CUSTOM_WARNING].on_color_container },
    };

    select_section(demo, SECTION_SURFACES);
    TRLV_ASSERT_INT(lv_obj_get_child_count(section(demo)), 3);
    assert_swatches(demo, SURFACES_CUSTOM, pairs, 4);
    TRLV_ASSERT_INT(lv_obj_get_child_count(swatches(demo, SURFACES_LADDER)) +
                    lv_obj_get_child_count(swatches(demo, SURFACES_CONTAINERS)), TRLV_SURFACE_COUNT);
}

/* With no custom colours in the tokens, the custom group is left out. */
static void surfaces_without_custom_colors(lv_obj_t * screen)
{
    trlv_color_tokens_t no_custom = tokens_light;
    lv_obj_t * demo;

    no_custom.custom = NULL;
    no_custom.custom_count = 0;
    trelvis_warnings = 0;
    lv_log_register_print_cb(log_cb);
    trlv_init(lv_obj_get_display(screen), &no_custom, &tokens_regular);
    demo = trlv_demo_create(screen);

    select_section(demo, SECTION_SURFACES);
    TRLV_ASSERT_INT(lv_obj_get_child_count(section(demo)), 2);
    TRLV_ASSERT_INT(trelvis_warnings, 0);
}

/*---------------------------------------------------------------------------
 * Cards
 *-------------------------------------------------------------------------*/

#define CARD_PAD 16   /* TRLV_SPACE_LG */
#define CARD_GAP 8    /* TRLV_SPACE_SM */

/* Children of a card, in order. */
enum { CARD_TYPE, CARD_TITLE, CARD_BODY, CARD_ACTIONS };

/* The three cards are stacked the section gap apart and fill the width:
 * filled (surface_container_highest, no border or shadow), elevated
 * (surface_container_low with a shadow) and outlined (surface with an
 * outline_variant border). */
static void cards_show_each_type(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    const trlv_color_tokens_t * c = trlv_color();
    const lv_color_t fills[] = { c->surface_container_highest, c->surface_container_low, c->surface };
    const int32_t borders[] = { 0, 0, trlv_layout()->border_width };
    int32_t y = 0;

    select_section(demo, SECTION_CARDS);
    TRLV_ASSERT_INT(lv_obj_get_child_count(section(demo)), 3);
    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * card = lv_obj_get_child(section(demo), i);

        TRLV_ASSERT_RECT(card, 0, y, 800 - 2 * FRAME_PAD, trlv_test_height(card));
        TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(card, LV_PART_MAIN), fills[i]), true);
        TRLV_ASSERT_INT(lv_obj_get_style_border_width(card, LV_PART_MAIN), borders[i]);
        TRLV_ASSERT_INT(lv_obj_get_style_shadow_width(card, LV_PART_MAIN) > 0, i == 1);
        y += trlv_test_height(card) + SECTION_GAP;
    }
    TRLV_ASSERT_INT(same_color(lv_obj_get_style_border_color(lv_obj_get_child(section(demo), 2), LV_PART_MAIN),
                               c->outline_variant), true);
}

/* Inside each card, the content sits in the padding (and inside the
 * outlined card's border), stacked the card gap apart. The body fills the
 * width and wraps, and the text is on_surface by inheritance only. */
static void cards_lay_out_their_content(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);

    select_section(demo, SECTION_CARDS);
    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * card = lv_obj_get_child(section(demo), i);
        lv_obj_t * body = lv_obj_get_child(card, CARD_BODY);
        int32_t inset = CARD_PAD + lv_obj_get_style_border_width(card, LV_PART_MAIN);
        int32_t width = trlv_test_width(card) - 2 * inset;
        int32_t y = inset;

        for(int32_t part = CARD_TYPE; part <= CARD_ACTIONS; part++) {
            lv_obj_t * child = lv_obj_get_child(card, part);
            lv_style_value_t local;

            TRLV_ASSERT_X(child, inset);
            TRLV_ASSERT_Y(child, y);
            y += trlv_test_height(child) + CARD_GAP;
            if(part != CARD_ACTIONS) {
                TRLV_ASSERT_INT(same_color(lv_obj_get_style_text_color(child, LV_PART_MAIN),
                                           trlv_color()->on_surface), true);
                TRLV_ASSERT_INT(lv_obj_get_local_style_prop(child, LV_STYLE_TEXT_COLOR, &local, LV_PART_MAIN),
                                LV_STYLE_RES_NOT_FOUND);
            }
        }
        TRLV_ASSERT_HEIGHT(card, y - CARD_GAP + inset);
        TRLV_ASSERT_RECT(body, inset, trlv_test_y(body), width, wrapped_height(body, width));
        TRLV_ASSERT_WIDTH(lv_obj_get_child(card, CARD_ACTIONS), width);
    }
}

/* The actions are packed at the end: a text button, then the main action
 * as a filled button against the right edge. */
static void cards_put_actions_at_the_end(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);

    select_section(demo, SECTION_CARDS);
    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * actions = lv_obj_get_child(lv_obj_get_child(section(demo), i), CARD_ACTIONS);
        lv_obj_t * secondary = lv_obj_get_child(actions, 0);
        lv_obj_t * main_action = lv_obj_get_child(actions, 1);

        TRLV_ASSERT_INT(lv_obj_get_child_count(actions), 2);
        TRLV_ASSERT_X(main_action, trlv_test_width(actions) - trlv_test_width(main_action));
        TRLV_ASSERT_X(secondary, trlv_test_x(main_action) - CARD_GAP - trlv_test_width(secondary));
        TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(secondary, LV_PART_MAIN), LV_OPA_TRANSP);
        TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(main_action, LV_PART_MAIN),
                                   trlv_color()->primary), true);
    }
}

int main(void)
{
    TRLV_TEST_RUN(frame_fills_the_screen);
    TRLV_TEST_RUN(bar_is_a_surface_container);
    TRLV_TEST_RUN(nav_has_a_button_per_section);
    TRLV_TEST_RUN(nav_shows_the_chosen_section);
    TRLV_TEST_RUN(every_section_fits_and_builds_quietly);

    TRLV_TEST_RUN(readouts_stack_their_parts);
    TRLV_TEST_RUN(readouts_header_places_title_and_status);
    TRLV_TEST_RUN(readouts_put_values_on_the_right);
    TRLV_TEST_RUN(readouts_body_text_wraps);
    TRLV_TEST_RUN(readouts_buttons_sit_in_a_row);
    TRLV_TEST_RUN(readouts_stand_alone);

    TRLV_TEST_RUN(surfaces_show_the_ladder);
    TRLV_TEST_RUN(surfaces_show_the_containers);
    TRLV_TEST_RUN(surfaces_show_the_custom_colors);
    TRLV_TEST_RUN(surfaces_without_custom_colors);

    TRLV_TEST_RUN(cards_show_each_type);
    TRLV_TEST_RUN(cards_lay_out_their_content);
    TRLV_TEST_RUN(cards_put_actions_at_the_end);

    trlv_test_set_display_size(320, 480);
    TRLV_TEST_RUN(every_section_fits_and_builds_quietly);
    TRLV_TEST_RUN(readouts_on_a_narrow_display);
    TRLV_TEST_RUN(readouts_buttons_wrap);
    return trlv_test_finish();
}
