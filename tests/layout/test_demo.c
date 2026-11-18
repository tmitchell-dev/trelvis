/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * The demo (examples/demo) on a fixed-size display: the frame (bar, nav and
 * content), moving between sections, the light/dark switch, keypad focus,
 * each section's layout, nothing overflowing its parent, room for every
 * focus ring and no TreLVis warnings while it is built. Every test runs in
 * each layout mode (Regular and Compact) and colour scheme; expected
 * spacing is read from the tokens in use. Text
 * sizes are computed from the font with lv_text_get_size, so the tests
 * don't depend on the fonts' metrics. The object trees are described at the
 * top of trlv_demo.c and of each section's file.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"
#include "trlv_tokens_hmi.h"
#include "trlv_demo.h"

#include <stdio.h>
#include <string.h>

/* The tokens the tests run with. main() runs every test once per mode:
 * each layout collection, and each colour scheme. */
static const trlv_color_tokens_t * color_light;
static const trlv_color_tokens_t * color_dark;
static const trlv_layout_tokens_t * layout_tokens;

/* A spacing step in the layout in use (valid once a test has called
 * trlv_init). */
static int32_t space(trlv_space_t step)
{
    return trlv_get_layout_tokens()->space[step];
}

#define FRAME_PAD    space(TRLV_SPACE_LG)   /* content padding */
#define BAR_PAD      space(TRLV_SPACE_SM)   /* the bar's padding... */
#define ROW_PAD      space(TRLV_SPACE_SM)   /* ...and that of every row holding
                                               focusable widgets (ring clearance) */
#define TOP_GAP      space(TRLV_SPACE_SM)
#define NAV_GAP      space(TRLV_SPACE_XS)
#define SECTION_GAP  space(TRLV_SPACE_LG)   /* between a section's parts */
#define READOUTS_GAP space(TRLV_SPACE_SM)
#define READOUT_GAP  space(TRLV_SPACE_MD)
#define ABOUT_GAP    space(TRLV_SPACE_XS)
#define ACTIONS_GAP  space(TRLV_SPACE_SM)

/* The sections, in the order of the nav buttons. */
static const struct {
    const char * name;
    lv_obj_t * (*create)(lv_obj_t * parent);
} sections[] = {
    { "Readouts", trlv_demo_readouts_create },
    { "Surfaces", trlv_demo_surfaces_create },
    { "Cards", trlv_demo_cards_create },
    { "Controls", trlv_demo_controls_create },
    { "List", trlv_demo_list_create },
};

/* Indexes into sections[]. */
enum { SECTION_READOUTS, SECTION_SURFACES, SECTION_CARDS, SECTION_CONTROLS, SECTION_LIST };

#define SECTION_COUNT ((int32_t)(sizeof(sections) / sizeof(sections[0])))

/* Children of the demo, of its bar, of the bar's top row and of the
 * Readouts section, in order. */
enum { DEMO_BAR, DEMO_CONTENT };
enum { BAR_TOP, BAR_NAV };
enum { TOP_TITLE, TOP_LABEL, TOP_SWITCH };
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
    trlv_init(lv_obj_get_display(screen), color_light, layout_tokens);
    return trlv_demo_create(screen, color_light, color_dark);
}

static lv_obj_t * bar(lv_obj_t * demo)
{
    return lv_obj_get_child(demo, DEMO_BAR);
}

static lv_obj_t * top(lv_obj_t * demo)
{
    return lv_obj_get_child(bar(demo), BAR_TOP);
}

static lv_obj_t * dark_switch(lv_obj_t * demo)
{
    return lv_obj_get_child(top(demo), TOP_SWITCH);
}

static void set_dark(lv_obj_t * demo, bool dark);

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

static void assert_color(const char * what, int line, lv_color_t actual, lv_color_t expected)
{
    trlv_test_check_int(__FILE__, line, what, (int32_t)lv_color_to_u32(expected), (int32_t)lv_color_to_u32(actual));
}

#define ASSERT_COLOR(actual, expected) assert_color(#actual, __LINE__, (actual), (expected))

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

/* The demo fills the screen: the bar across the top hugs its two rows,
 * and the content takes the rest of the height. The bar's padding plus
 * each row's brings the title and the first nav button 16 px in, in line
 * with the content. */
static void frame_fills_the_screen(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    int32_t row_width = 800 - 2 * BAR_PAD;
    int32_t top_height = trlv_test_height(top(demo));
    int32_t nav_height = trlv_test_height(lv_obj_get_child(nav(demo), 0)) + 2 * ROW_PAD;
    int32_t bar_height = BAR_PAD + top_height + nav_height + BAR_PAD;

    TRLV_ASSERT_RECT(demo, 0, 0, 800, 480);
    TRLV_ASSERT_RECT(bar(demo), 0, 0, 800, bar_height);
    TRLV_ASSERT_RECT(top(demo), BAR_PAD, BAR_PAD, row_width, top_height);
    TRLV_ASSERT_RECT(nav(demo), BAR_PAD, BAR_PAD + top_height, row_width, nav_height);
    TRLV_ASSERT_X(lv_obj_get_child(nav(demo), 0), ROW_PAD);
    TRLV_ASSERT_RECT(content(demo), 0, bar_height, 800, 480 - bar_height);
    TRLV_ASSERT_RECT(section(demo), FRAME_PAD, FRAME_PAD, 800 - 2 * FRAME_PAD, trlv_test_height(section(demo)));
}

/* The top row: the title fills, pushing the "Dark" label and the switch to
 * the right; all are centred on the row. */
static void top_row_puts_the_switch_on_the_right(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    lv_obj_t * title = lv_obj_get_child(top(demo), TOP_TITLE);
    lv_obj_t * label = lv_obj_get_child(top(demo), TOP_LABEL);
    lv_obj_t * sw = dark_switch(demo);
    int32_t inner_width = 800 - 2 * BAR_PAD - 2 * ROW_PAD;
    int32_t inner_height = LV_MAX(line_height(title), trlv_test_height(sw));

    TRLV_ASSERT_INT(lv_obj_check_type(sw, &lv_switch_class), true);
    TRLV_ASSERT_INT(strcmp(lv_label_get_text(label), "Dark"), 0);
    TRLV_ASSERT_HEIGHT(top(demo), inner_height + 2 * ROW_PAD);
    TRLV_ASSERT_RECT(sw, ROW_PAD + inner_width - trlv_test_width(sw),
                     ROW_PAD + (inner_height - trlv_test_height(sw)) / 2,
                     trlv_test_width(sw), trlv_test_height(sw));
    TRLV_ASSERT_X(label, trlv_test_x(sw) - TOP_GAP - text_width(label));
    TRLV_ASSERT_Y(label, ROW_PAD + (inner_height - line_height(label)) / 2);
    TRLV_ASSERT_RECT(title, ROW_PAD, ROW_PAD + (inner_height - line_height(title)) / 2,
                     trlv_test_x(label) - TOP_GAP - ROW_PAD, line_height(title));
}

/* Without dark tokens the switch is left out. */
static void no_switch_without_dark_tokens(lv_obj_t * screen)
{
    lv_obj_t * demo;

    trlv_init(lv_obj_get_display(screen), color_light, layout_tokens);
    demo = trlv_demo_create(screen, color_light, NULL);
    TRLV_ASSERT_INT(lv_obj_get_child_count(top(demo)), 1);
}

/* The bar is a Surface Container, and its text gets on_surface by
 * inheritance. */
static void bar_is_a_surface_container(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    lv_obj_t * title = lv_obj_get_child(top(demo), TOP_TITLE);

    TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(bar(demo), LV_PART_MAIN),
                               trlv_get_color_tokens()->surface_container), true);
    TRLV_ASSERT_INT(same_color(lv_obj_get_style_text_color(title, LV_PART_MAIN),
                               trlv_get_color_tokens()->on_surface), true);
}

/* One nav button per section, named after it and side by side; the
 * selected one is tonal (secondary_container), the others are text
 * buttons (no fill). The first section is shown at the start. */
static void nav_has_a_button_per_section(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    int32_t x = ROW_PAD;

    TRLV_ASSERT_INT(lv_obj_get_child_count(nav(demo)), SECTION_COUNT);
    for(int32_t i = 0; i < SECTION_COUNT; i++) {
        lv_obj_t * btn = lv_obj_get_child(nav(demo), i);
        lv_obj_t * label = lv_obj_get_child(btn, 0);

        TRLV_ASSERT_INT(strcmp(lv_label_get_text(label), sections[i].name), 0);
        TRLV_ASSERT_X(btn, x);
        TRLV_ASSERT_Y(btn, ROW_PAD);
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
                                       trlv_get_color_tokens()->secondary_container), true);
        }
        else {
            TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(btn, LV_PART_MAIN), LV_OPA_TRANSP);
        }
    }
}

/* Clicking a nav button selects it and replaces the section with the one
 * it names (compared with a copy built on its own first: two copies of the
 * largest section don't fit LVGL's default 64 KB pool). The content
 * scrolls back to the top. */
static void nav_shows_the_chosen_section(lv_obj_t * screen)
{
    uint32_t reference_children[SECTION_COUNT];
    lv_obj_t * demo;

    trlv_init(lv_obj_get_display(screen), color_light, layout_tokens);
    for(int32_t i = 0; i < SECTION_COUNT; i++) {
        lv_obj_t * reference = sections[i].create(screen);
        reference_children[i] = lv_obj_get_child_count(reference);
        lv_obj_delete(reference);
    }

    demo = build(screen);
    assert_selected(demo, 0);
    for(int32_t i = SECTION_COUNT - 1; i >= 0; i--) {
        lv_obj_scroll_to_y(content(demo), 40, LV_ANIM_OFF);
        select_section(demo, i);

        assert_selected(demo, i);
        TRLV_ASSERT_INT(lv_obj_get_child_count(content(demo)), 1);
        TRLV_ASSERT_INT(lv_obj_get_child_count(section(demo)), reference_children[i]);
        TRLV_ASSERT_INT(lv_obj_get_scroll_y(content(demo)), 0);
    }
}

static uint32_t memory_in_use(void)
{
    lv_mem_monitor_t monitor;
    lv_mem_monitor(&monitor);
    return monitor.total_size - monitor.free_size;
}

/* Choosing a section frees the previous one: after rounds of every section
 * and of the light/dark switch, the demo uses what it did at the start.
 * The allocator's own bookkeeping varies by a few dozen bytes with
 * fragmentation; a section left behind would be kilobytes. */
static void sections_are_freed_when_left(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    uint32_t start = memory_in_use();

    for(int32_t round = 0; round < 3; round++) {
        for(int32_t i = 0; i < SECTION_COUNT; i++) select_section(demo, i);
        set_dark(demo, true);
        set_dark(demo, false);
        select_section(demo, 0);
        TRLV_ASSERT_INT(memory_in_use() <= start + 256, true);
    }
}

/* Whether a nav button is wholly inside the nav row's visible area. The
 * harness measures layout positions, which ignore scrolling, so the row's
 * scroll offset is taken off here. */
static bool nav_button_in_view(lv_obj_t * demo, int32_t index)
{
    lv_obj_t * btn = lv_obj_get_child(nav(demo), index);
    int32_t x = trlv_test_x(btn) - lv_obj_get_scroll_x(nav(demo));
    return x >= 0 && x + trlv_test_width(btn) <= trlv_test_width(nav(demo));
}

/* On a narrow display the nav buttons don't all fit; the nav row scrolls
 * so that the chosen one is fully in view. */
static void nav_scrolls_the_choice_into_view(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);

    TRLV_ASSERT_INT(nav_button_in_view(demo, SECTION_COUNT - 1), false);   /* the case to test */

    select_section(demo, SECTION_COUNT - 1);
    TRLV_ASSERT_INT(nav_button_in_view(demo, SECTION_COUNT - 1), true);

    select_section(demo, 0);
    TRLV_ASSERT_INT(nav_button_in_view(demo, 0), true);
    TRLV_ASSERT_INT(lv_obj_get_scroll_x(nav(demo)), 0);
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
 * row that fills the width, inside its focus ring padding. */
static void readouts_buttons_sit_in_a_row(lv_obj_t * screen)
{
    lv_obj_t * actions = readouts_part(build(screen), ACTIONS);
    int32_t x = ROW_PAD;

    TRLV_ASSERT_INT(lv_obj_get_child_count(actions), 3);
    TRLV_ASSERT_WIDTH(actions, 800 - 2 * FRAME_PAD);
    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * btn = lv_obj_get_child(actions, i);
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        TRLV_ASSERT_X(btn, x);
        TRLV_ASSERT_Y(btn, ROW_PAD);
        TRLV_ASSERT_WIDTH(label, text_width(label));
        TRLV_ASSERT_HEIGHT(btn, trlv_test_height(actions) - 2 * ROW_PAD);
        x += trlv_test_width(btn) + ACTIONS_GAP;
    }
}

/* Buttons that don't fit across move to the next line, the gap below;
 * they wrap if, and only if, they don't fit on one line. */
static void readouts_buttons_wrap(lv_obj_t * screen)
{
    lv_obj_t * actions = readouts_part(build(screen), ACTIONS);
    int32_t width = trlv_test_width(actions) - ROW_PAD;   /* the right edge of the content */
    int32_t x = ROW_PAD;
    int32_t y = ROW_PAD;
    int32_t lines = 1;
    int32_t one_line = ROW_PAD + 2 * ACTIONS_GAP;

    for(int32_t i = 0; i < 3; i++) one_line += trlv_test_width(lv_obj_get_child(actions, i));

    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * btn = lv_obj_get_child(actions, i);
        if(x > ROW_PAD && x + trlv_test_width(btn) > width) {
            x = ROW_PAD;
            y += trlv_test_height(btn) + ACTIONS_GAP;
            lines++;
        }
        TRLV_ASSERT_X(btn, x);
        TRLV_ASSERT_Y(btn, y);
        x += trlv_test_width(btn) + ACTIONS_GAP;
    }
    TRLV_ASSERT_INT(lines > 1, one_line > width);
    TRLV_ASSERT_HEIGHT(actions, y + trlv_test_height(lv_obj_get_child(actions, 2)) + ROW_PAD);
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
    trlv_init(lv_obj_get_display(screen), color_light, layout_tokens);
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
    const trlv_color_tokens_t * c = trlv_get_color_tokens();
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
    const trlv_color_tokens_t * c = trlv_get_color_tokens();
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

/* Each of the tokens' custom colours (up to TRLV_CUSTOM_COLOR_MAX) shows
 * as its strong colour and its container, and together with the two groups
 * above every TRLV_SURFACE_* is shown. */
static void surfaces_show_the_custom_colors(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    const trlv_color_tokens_t * c = trlv_get_color_tokens();
    int32_t count = (int32_t)LV_MIN(c->custom_count, (uint32_t)TRLV_CUSTOM_COLOR_MAX);
    color_pair_t pairs[2 * TRLV_CUSTOM_COLOR_MAX];

    for(int32_t i = 0; i < count; i++) {
        pairs[2 * i].fill = c->custom[i].color;
        pairs[2 * i].content = c->custom[i].on_color;
        pairs[2 * i + 1].fill = c->custom[i].color_container;
        pairs[2 * i + 1].content = c->custom[i].on_color_container;
    }

    select_section(demo, SECTION_SURFACES);
    TRLV_ASSERT_INT(count > 0, true);   /* every colour scheme tested has some */
    TRLV_ASSERT_INT(lv_obj_get_child_count(section(demo)), 3);
    assert_swatches(demo, SURFACES_CUSTOM, pairs, 2 * count);
    TRLV_ASSERT_INT(lv_obj_get_child_count(swatches(demo, SURFACES_LADDER)) +
                    lv_obj_get_child_count(swatches(demo, SURFACES_CONTAINERS)), TRLV_SURFACE_COUNT);
}

/* With no custom colours in the tokens, the custom group is left out. */
static void surfaces_without_custom_colors(lv_obj_t * screen)
{
    trlv_color_tokens_t no_custom = *color_light;
    lv_obj_t * demo;

    no_custom.custom = NULL;
    no_custom.custom_count = 0;
    trelvis_warnings = 0;
    lv_log_register_print_cb(log_cb);
    trlv_init(lv_obj_get_display(screen), &no_custom, layout_tokens);
    demo = trlv_demo_create(screen, &no_custom, NULL);

    select_section(demo, SECTION_SURFACES);
    TRLV_ASSERT_INT(lv_obj_get_child_count(section(demo)), 2);
    TRLV_ASSERT_INT(trelvis_warnings, 0);
}

/*---------------------------------------------------------------------------
 * Cards
 *-------------------------------------------------------------------------*/

#define CARD_PAD space(TRLV_SPACE_LG)
#define CARD_GAP space(TRLV_SPACE_SM)

/* Children of a card, in order. */
enum { CARD_TYPE, CARD_TITLE, CARD_BODY, CARD_ACTIONS };

/* The three cards are stacked the section gap apart and fill the width:
 * filled (surface_container_highest, no border or shadow), elevated
 * (surface_container_low with the level 1 shadow, none in Compact) and
 * outlined (surface with an outline_variant border). */
static void cards_show_each_type(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    const trlv_color_tokens_t * c = trlv_get_color_tokens();
    const lv_color_t fills[] = { c->surface_container_highest, c->surface_container_low, c->surface };
    const int32_t borders[] = { 0, 0, trlv_get_layout_tokens()->border_width };
    int32_t y = 0;

    select_section(demo, SECTION_CARDS);
    TRLV_ASSERT_INT(lv_obj_get_child_count(section(demo)), 3);
    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * card = lv_obj_get_child(section(demo), i);

        TRLV_ASSERT_RECT(card, 0, y, 800 - 2 * FRAME_PAD, trlv_test_height(card));
        TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(card, LV_PART_MAIN), fills[i]), true);
        TRLV_ASSERT_INT(lv_obj_get_style_border_width(card, LV_PART_MAIN), borders[i]);
        TRLV_ASSERT_INT(lv_obj_get_style_shadow_width(card, LV_PART_MAIN),
                        i == 1 ? trlv_get_layout_tokens()->elevation[TRLV_ELEVATION_1].blur : 0);
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
                                           trlv_get_color_tokens()->on_surface), true);
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
 * as a filled button against the right edge of the row's content (inside
 * its focus ring padding). */
static void cards_put_actions_at_the_end(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);

    select_section(demo, SECTION_CARDS);
    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * actions = lv_obj_get_child(lv_obj_get_child(section(demo), i), CARD_ACTIONS);
        lv_obj_t * secondary = lv_obj_get_child(actions, 0);
        lv_obj_t * main_action = lv_obj_get_child(actions, 1);

        TRLV_ASSERT_INT(lv_obj_get_child_count(actions), 2);
        TRLV_ASSERT_X(main_action, trlv_test_width(actions) - ROW_PAD - trlv_test_width(main_action));
        TRLV_ASSERT_Y(main_action, ROW_PAD);
        TRLV_ASSERT_X(secondary, trlv_test_x(main_action) - CARD_GAP - trlv_test_width(secondary));
        TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(secondary, LV_PART_MAIN), LV_OPA_TRANSP);
        TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(main_action, LV_PART_MAIN),
                                   trlv_get_color_tokens()->primary), true);
    }
}

/*---------------------------------------------------------------------------
 * Controls
 *-------------------------------------------------------------------------*/

/* Groups of the Controls section, in order; child 0 of each is its heading
 * and the rest are rows. */
enum { CONTROLS_BUTTONS, CONTROLS_SELECTION, CONTROLS_RANGE, CONTROLS_FIELDS };

static lv_obj_t * controls_row(lv_obj_t * demo, int32_t group, int32_t row)
{
    return lv_obj_get_child(lv_obj_get_child(section(demo), group), row + 1);
}

/* Two rows of the four button types, the second disabled. */
static void controls_show_every_button_variant(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    const trlv_color_tokens_t * c = trlv_get_color_tokens();

    select_section(demo, SECTION_CONTROLS);
    for(int32_t row = 0; row < 2; row++) {
        lv_obj_t * buttons = controls_row(demo, CONTROLS_BUTTONS, row);
        lv_obj_t * filled = lv_obj_get_child(buttons, 0);
        lv_obj_t * tonal = lv_obj_get_child(buttons, 1);
        lv_obj_t * outlined = lv_obj_get_child(buttons, 2);
        lv_obj_t * text = lv_obj_get_child(buttons, 3);

        TRLV_ASSERT_INT(lv_obj_get_child_count(buttons), 4);
        for(int32_t i = 0; i < 4; i++) {
            TRLV_ASSERT_INT(lv_obj_has_state(lv_obj_get_child(buttons, i), LV_STATE_DISABLED), row == 1);
        }
        if(row == 0) {
            TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(filled, LV_PART_MAIN), c->primary), true);
            TRLV_ASSERT_INT(same_color(lv_obj_get_style_bg_color(tonal, LV_PART_MAIN), c->secondary_container), true);
            TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(outlined, LV_PART_MAIN), LV_OPA_TRANSP);
            TRLV_ASSERT_INT(lv_obj_get_style_border_width(outlined, LV_PART_MAIN), trlv_get_layout_tokens()->border_width);
            TRLV_ASSERT_INT(lv_obj_get_style_bg_opa(text, LV_PART_MAIN), LV_OPA_TRANSP);
            TRLV_ASSERT_INT(lv_obj_get_style_border_width(text, LV_PART_MAIN), 0);
        }
    }
}

/* Switches and checkboxes off, on, off and disabled, on and disabled. */
static void controls_show_switch_and_checkbox_states(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    const lv_state_t states[] = {
        LV_STATE_DEFAULT, LV_STATE_CHECKED, LV_STATE_DISABLED, LV_STATE_CHECKED | LV_STATE_DISABLED
    };

    select_section(demo, SECTION_CONTROLS);
    for(int32_t row = 0; row < 2; row++) {
        lv_obj_t * widgets = controls_row(demo, CONTROLS_SELECTION, row);
        const lv_obj_class_t * expected = row == 0 ? &lv_switch_class : &lv_checkbox_class;

        TRLV_ASSERT_INT(lv_obj_get_child_count(widgets), 4);
        for(int32_t i = 0; i < 4; i++) {
            lv_obj_t * widget = lv_obj_get_child(widgets, i);
            TRLV_ASSERT_INT(lv_obj_check_type(widget, expected), true);
            TRLV_ASSERT_INT(lv_obj_get_state(widget) & (LV_STATE_CHECKED | LV_STATE_DISABLED), states[i]);
        }
    }
}

/* Count the descendants of `obj` of a class, and how many are disabled. */
static void count_class(lv_obj_t * obj, const lv_obj_class_t * cls, int32_t * total, int32_t * disabled)
{
    for(uint32_t i = 0; i < lv_obj_get_child_count(obj); i++) {
        lv_obj_t * child = lv_obj_get_child(obj, (int32_t)i);
        if(lv_obj_check_type(child, cls)) {
            (*total)++;
            if(lv_obj_has_state(child, LV_STATE_DISABLED)) (*disabled)++;
        }
        count_class(child, cls, total, disabled);
    }
}

/* Every widget the theme covers is in the section, and every interactive
 * one is also shown disabled. */
static void controls_cover_every_themed_widget(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    static const struct {
        const lv_obj_class_t * cls;
        bool interactive;
    } widgets[] = {
        { &lv_button_class, true },   { &lv_switch_class, true },   { &lv_checkbox_class, true },
        { &lv_slider_class, true },   { &lv_bar_class, false },     { &lv_arc_class, true },
        { &lv_spinner_class, false }, { &lv_dropdown_class, true }, { &lv_textarea_class, true },
    };

    select_section(demo, SECTION_CONTROLS);
    for(uint32_t i = 0; i < sizeof(widgets) / sizeof(widgets[0]); i++) {
        int32_t total = 0;
        int32_t disabled = 0;
        count_class(section(demo), widgets[i].cls, &total, &disabled);
        TRLV_ASSERT_INT(total > 0, true);
        if(widgets[i].interactive) TRLV_ASSERT_INT(disabled > 0, true);
    }
}

/* Each heading's text lines up with the first control of the row below
 * it: both are inset by the focus ring padding. */
static void controls_line_up_with_their_headings(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);

    select_section(demo, SECTION_CONTROLS);
    for(int32_t group = CONTROLS_BUTTONS; group <= CONTROLS_FIELDS; group++) {
        lv_obj_t * heading = lv_obj_get_child(lv_obj_get_child(section(demo), group), 0);
        lv_obj_t * row = controls_row(demo, group, 0);

        TRLV_ASSERT_X(heading, 0);
        TRLV_ASSERT_INT(lv_obj_get_style_pad_left(heading, LV_PART_MAIN), ROW_PAD);
        TRLV_ASSERT_X(row, 0);
        TRLV_ASSERT_X(lv_obj_get_child(row, 0), ROW_PAD);
    }
}

/*---------------------------------------------------------------------------
 * List
 *-------------------------------------------------------------------------*/

#define LIST_ITEMS 8
#define ITEM_GAP   space(TRLV_SPACE_LG)

/* Children of the section, and of a list item, in order. */
enum { LIST_HEADING, LIST_LIST };
enum { ITEM_ICON, ITEM_TEXT, ITEM_TRAILING };

/* The list alternates items and full-width dividers, stacked with no gap,
 * and its heading lines up with the items' icons. */
static void list_alternates_items_and_dividers(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    lv_obj_t * list;
    lv_obj_t * heading;
    int32_t width = 800 - 2 * FRAME_PAD;
    int32_t y = 0;

    select_section(demo, SECTION_LIST);
    list = lv_obj_get_child(section(demo), LIST_LIST);
    heading = lv_obj_get_child(section(demo), LIST_HEADING);

    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(heading, LV_PART_MAIN), ROW_PAD);
    TRLV_ASSERT_RECT(list, 0, trlv_test_height(heading), width, trlv_test_height(list));
    TRLV_ASSERT_INT(lv_obj_get_child_count(list), 2 * LIST_ITEMS - 1);
    for(int32_t i = 0; i < 2 * LIST_ITEMS - 1; i++) {
        lv_obj_t * child = lv_obj_get_child(list, i);
        if(i % 2 == 1) {   /* a divider */
            TRLV_ASSERT_RECT(child, 0, y, width, trlv_get_layout_tokens()->border_width);
            ASSERT_COLOR(lv_obj_get_style_bg_color(child, LV_PART_MAIN), trlv_get_color_tokens()->outline_variant);
        }
        else {
            TRLV_ASSERT_RECT(child, 0, y, width, trlv_test_height(child));
        }
        y += trlv_test_height(child);
    }
    TRLV_ASSERT_HEIGHT(list, y);
}

/* In each item the icon sits at the start, the trailing item at the end,
 * and the text takes the space between, its supporting text wrapping in
 * it. Everything is centred on the item, inside its padding. */
static void list_items_put_text_between_icon_and_trailing(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);
    lv_obj_t * list;
    int32_t switches = 0;

    select_section(demo, SECTION_LIST);
    list = lv_obj_get_child(section(demo), LIST_LIST);
    for(int32_t i = 0; i < LIST_ITEMS; i++) {
        lv_obj_t * item = lv_obj_get_child(list, 2 * i);
        lv_obj_t * icon = lv_obj_get_child(item, ITEM_ICON);
        lv_obj_t * text = lv_obj_get_child(item, ITEM_TEXT);
        lv_obj_t * trailing = lv_obj_get_child(item, ITEM_TRAILING);
        lv_obj_t * supporting = lv_obj_get_child(text, 1);
        int32_t inner_width = trlv_test_width(item) - 2 * ROW_PAD;
        int32_t text_x = ROW_PAD + trlv_test_width(icon) + ITEM_GAP;
        int32_t trailing_x = ROW_PAD + inner_width - trlv_test_width(trailing);
        int32_t text_width_px = trailing_x - ITEM_GAP - text_x;
        int32_t inner_height = trlv_test_height(item) - 2 * ROW_PAD;

        TRLV_ASSERT_INT(lv_obj_get_child_count(item), 3);
        TRLV_ASSERT_RECT(icon, ROW_PAD, trlv_test_y(icon), line_height(icon), line_height(icon));
        TRLV_ASSERT_X(text, ROW_PAD + line_height(icon) + ITEM_GAP);   /* the same in every item */
        TRLV_ASSERT_RECT(text, text_x, trlv_test_y(text), text_width_px, trlv_test_height(text));
        TRLV_ASSERT_X(trailing, trailing_x);
        TRLV_ASSERT_RECT(supporting, 0, trlv_test_y(supporting), text_width_px,
                         wrapped_height(supporting, text_width_px));
        TRLV_ASSERT_Y(icon, ROW_PAD + (inner_height - trlv_test_height(icon)) / 2);
        TRLV_ASSERT_Y(text, ROW_PAD + (inner_height - trlv_test_height(text)) / 2);
        TRLV_ASSERT_Y(trailing, ROW_PAD + (inner_height - trlv_test_height(trailing)) / 2);
        if(lv_obj_check_type(trailing, &lv_switch_class)) switches++;
    }
    TRLV_ASSERT_INT(switches, 2);
}

/*---------------------------------------------------------------------------
 * Light/dark switch (a v1 acceptance criterion)
 *-------------------------------------------------------------------------*/

/* Flip the switch the way a click does: toggle the state, report the change. */
static void set_dark(lv_obj_t * demo, bool dark)
{
    lv_obj_set_state(dark_switch(demo), LV_STATE_CHECKED, dark);
    lv_obj_send_event(dark_switch(demo), LV_EVENT_VALUE_CHANGED, NULL);
}

/* Colours of the frame and of a few objects of the section on show, all
 * from the tokens `c`. */
static void assert_section_colors(lv_obj_t * demo, int32_t index, const trlv_color_tokens_t * c)
{
    lv_obj_t * s = section(demo);

    ASSERT_COLOR(lv_obj_get_style_bg_color(lv_obj_get_parent(demo), LV_PART_MAIN), c->surface);
    ASSERT_COLOR(lv_obj_get_style_bg_color(bar(demo), LV_PART_MAIN), c->surface_container);
    ASSERT_COLOR(lv_obj_get_style_text_color(lv_obj_get_child(top(demo), TOP_TITLE), LV_PART_MAIN), c->on_surface);
    ASSERT_COLOR(lv_obj_get_style_bg_color(lv_obj_get_child(nav(demo), index), LV_PART_MAIN), c->secondary_container);
    if(lv_obj_has_state(dark_switch(demo), LV_STATE_CHECKED)) {
        ASSERT_COLOR(lv_obj_get_style_bg_color(dark_switch(demo), LV_PART_INDICATOR), c->primary);
    }
    else {
        ASSERT_COLOR(lv_obj_get_style_bg_color(dark_switch(demo), LV_PART_MAIN), c->surface_container_highest);
    }

    if(index == SECTION_READOUTS) {
        lv_obj_t * title = lv_obj_get_child(lv_obj_get_child(s, HEADER), 0);
        lv_obj_t * start = lv_obj_get_child(lv_obj_get_child(s, ACTIONS), 0);
        ASSERT_COLOR(lv_obj_get_style_text_color(title, LV_PART_MAIN), c->on_surface);
        ASSERT_COLOR(lv_obj_get_style_bg_color(start, LV_PART_MAIN), c->primary);
        ASSERT_COLOR(lv_obj_get_style_text_color(lv_obj_get_child(start, 0), LV_PART_MAIN), c->on_primary);
    }
    else if(index == SECTION_SURFACES) {
        lv_obj_t * highest = lv_obj_get_child(swatches(demo, SURFACES_LADDER), 5);
        lv_obj_t * primary = lv_obj_get_child(swatches(demo, SURFACES_CONTAINERS), 0);
        ASSERT_COLOR(lv_obj_get_style_bg_color(highest, LV_PART_MAIN), c->surface_container_highest);
        ASSERT_COLOR(lv_obj_get_style_bg_color(primary, LV_PART_MAIN), c->primary_container);
        ASSERT_COLOR(lv_obj_get_style_text_color(lv_obj_get_child(primary, 0), LV_PART_MAIN), c->on_primary_container);
    }
    else if(index == SECTION_CARDS) {
        ASSERT_COLOR(lv_obj_get_style_bg_color(lv_obj_get_child(s, 0), LV_PART_MAIN), c->surface_container_highest);
        ASSERT_COLOR(lv_obj_get_style_border_color(lv_obj_get_child(s, 2), LV_PART_MAIN), c->outline_variant);
    }
    else if(index == SECTION_LIST) {
        lv_obj_t * list = lv_obj_get_child(s, 1);
        lv_obj_t * wifi = lv_obj_get_child(list, 0);
        ASSERT_COLOR(lv_obj_get_style_bg_color(lv_obj_get_child(list, 1), LV_PART_MAIN), c->outline_variant);
        ASSERT_COLOR(lv_obj_get_style_text_color(lv_obj_get_child(wifi, 0), LV_PART_MAIN), c->on_surface);
        ASSERT_COLOR(lv_obj_get_style_bg_color(lv_obj_get_child(wifi, 2), LV_PART_INDICATOR), c->primary);
    }
    else {
        lv_obj_t * tonal = lv_obj_get_child(controls_row(demo, CONTROLS_BUTTONS, 0), 1);
        lv_obj_t * on = lv_obj_get_child(controls_row(demo, CONTROLS_SELECTION, 0), 1);
        lv_obj_t * slider = lv_obj_get_child(controls_row(demo, CONTROLS_RANGE, 0), 0);
        lv_obj_t * field = lv_obj_get_child(controls_row(demo, CONTROLS_FIELDS, 0), 1);
        ASSERT_COLOR(lv_obj_get_style_bg_color(tonal, LV_PART_MAIN), c->secondary_container);
        ASSERT_COLOR(lv_obj_get_style_bg_color(on, LV_PART_INDICATOR), c->primary);
        ASSERT_COLOR(lv_obj_get_style_bg_color(slider, LV_PART_INDICATOR), c->primary);
        ASSERT_COLOR(lv_obj_get_style_border_color(field, LV_PART_MAIN), c->outline);
    }
}

/* In every section, the switch turns the dark tokens on and off, and the
 * frame, the section and its built-in widgets follow with no code of their
 * own. */
static void switch_recolors_every_section(lv_obj_t * screen)
{
    lv_obj_t * demo = build(screen);

    TRLV_ASSERT_INT(lv_obj_has_state(dark_switch(demo), LV_STATE_CHECKED), false);
    for(int32_t i = 0; i < SECTION_COUNT; i++) {
        select_section(demo, i);
        assert_section_colors(demo, i, color_light);

        set_dark(demo, true);
        TRLV_ASSERT_INT(trlv_get_color_tokens() == color_dark, true);
        assert_section_colors(demo, i, color_dark);

        set_dark(demo, false);
        TRLV_ASSERT_INT(trlv_get_color_tokens() == color_light, true);
        assert_section_colors(demo, i, color_light);
    }
}

/* A section built while dark is dark, and the switch starts on when
 * trlv_init was given the dark tokens. */
static void switch_starts_on_in_dark(lv_obj_t * screen)
{
    lv_obj_t * demo;

    trlv_init(lv_obj_get_display(screen), color_dark, layout_tokens);
    demo = trlv_demo_create(screen, color_light, color_dark);

    TRLV_ASSERT_INT(lv_obj_has_state(dark_switch(demo), LV_STATE_CHECKED), true);
    for(int32_t i = 0; i < SECTION_COUNT; i++) {
        select_section(demo, i);
        assert_section_colors(demo, i, color_dark);
    }
}

/*---------------------------------------------------------------------------
 * Keypad and encoder focus
 *-------------------------------------------------------------------------*/

static void keypad_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    LV_UNUSED(indev);
    data->state = LV_INDEV_STATE_RELEASED;
}

/* Build the demo the way the header says a keypad host does: a default
 * group, given to a keypad, before trlv_demo_create. */
static lv_obj_t * build_with_keypad(lv_obj_t * screen, lv_group_t ** group)
{
    lv_indev_t * keypad;

    *group = lv_group_create();
    lv_group_set_default(*group);
    keypad = lv_indev_create();
    lv_indev_set_type(keypad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(keypad, keypad_read);
    lv_indev_set_group(keypad, *group);
    return build(screen);
}

static bool is_inside(lv_obj_t * obj, lv_obj_t * ancestor)
{
    for(lv_obj_t * o = obj; o != NULL; o = lv_obj_get_parent(o)) {
        if(o == ancestor) return true;
    }
    return false;
}

/* The focus-key look: fields turn their border primary and focus_width
 * thick; everything else gets the focus ring. */
static void assert_focus_style(lv_obj_t * obj, const trlv_color_tokens_t * c)
{
    TRLV_ASSERT_INT(lv_obj_has_state(obj, LV_STATE_FOCUS_KEY), true);
    if(lv_obj_check_type(obj, &lv_dropdown_class) || lv_obj_check_type(obj, &lv_textarea_class)) {
        TRLV_ASSERT_INT(lv_obj_get_style_border_width(obj, LV_PART_MAIN), trlv_get_layout_tokens()->focus_width);
        ASSERT_COLOR(lv_obj_get_style_border_color(obj, LV_PART_MAIN), c->primary);
    }
    else {
        TRLV_ASSERT_INT(lv_obj_get_style_outline_width(obj, LV_PART_MAIN), trlv_get_layout_tokens()->focus_width);
        TRLV_ASSERT_INT(lv_obj_get_style_outline_opa(obj, LV_PART_MAIN), LV_OPA_COVER);
        ASSERT_COLOR(lv_obj_get_style_outline_color(obj, LV_PART_MAIN), c->focus);
    }
}

/* In every section and both colour modes, moving focus with the keypad
 * reaches every enabled widget of the section, skips the disabled ones,
 * and gives each the focus-key look in the mode's colours. */
static void keypad_focus_reaches_every_widget(lv_obj_t * screen)
{
    lv_group_t * group;
    lv_obj_t * demo = build_with_keypad(screen, &group);

    for(int32_t dark = 0; dark < 2; dark++) {
        const trlv_color_tokens_t * c = dark ? color_dark : color_light;
        set_dark(demo, dark);

        for(int32_t i = 0; i < SECTION_COUNT; i++) {
            int32_t enabled = 0;           /* in the whole group */
            int32_t section_enabled = 0;   /* in the section on show */
            int32_t focused = 0;

            select_section(demo, i);
            for(uint32_t k = 0; k < lv_group_get_obj_count(group); k++) {
                lv_obj_t * obj = lv_group_get_obj_by_index(group, k);
                if(lv_obj_has_state(obj, LV_STATE_DISABLED)) continue;
                enabled++;
                if(is_inside(obj, section(demo))) section_enabled++;
            }

            /* Once round the enabled objects (focus skips disabled ones):
             * each once, the section's among them. */
            for(int32_t k = 0; k < enabled; k++) {
                lv_obj_t * obj;
                lv_group_focus_next(group);
                obj = lv_group_get_focused(group);
                TRLV_ASSERT_INT(lv_obj_has_state(obj, LV_STATE_DISABLED), false);
                assert_focus_style(obj, c);
                if(is_inside(obj, section(demo))) focused++;
            }
            TRLV_ASSERT_INT(focused, section_enabled);
        }
    }
    TRLV_ASSERT_INT(trelvis_warnings, 0);
}

/* Room for the ring: every focusable widget is at least the ring's reach
 * (focus_width plus the space[2XS] gap) inside each of its ancestors, so
 * no ancestor clips it. Along a direction an ancestor scrolls in, it and
 * the ancestors above it are not checked: there, clipping is what
 * scrolling does. */
static void every_focus_ring_has_room(lv_obj_t * screen)
{
    lv_group_t * group;
    lv_obj_t * demo = build_with_keypad(screen, &group);
    int32_t ring = trlv_get_layout_tokens()->focus_width + trlv_get_layout_tokens()->space[TRLV_SPACE_2XS];

    for(int32_t i = 0; i < SECTION_COUNT; i++) {
        select_section(demo, i);
        for(uint32_t k = 0; k < lv_group_get_obj_count(group); k++) {
            lv_obj_t * obj = lv_group_get_obj_by_index(group, k);
            lv_area_t area;
            lv_dir_t scroll = LV_DIR_NONE;   /* directions scrolled so far, walking up */

            lv_obj_update_layout(obj);
            lv_obj_get_coords(obj, &area);
            for(lv_obj_t * a = lv_obj_get_parent(obj); a != lv_obj_get_parent(demo); a = lv_obj_get_parent(a)) {
                lv_area_t clip;
                char what[96];

                if(lv_obj_is_scrollable(a)) scroll |= lv_obj_get_scroll_dir(a);
                lv_obj_get_coords(a, &clip);
                lv_snprintf(what, sizeof(what), "%s: ring of child %" LV_PRIu32 " of the group fits across",
                            sections[i].name, k);
                if(!(scroll & LV_DIR_HOR)) {
                    trlv_test_check_int(__FILE__, __LINE__, what, true,
                                        area.x1 - ring >= clip.x1 && area.x2 + ring <= clip.x2);
                }
                lv_snprintf(what, sizeof(what), "%s: ring of child %" LV_PRIu32 " of the group fits down",
                            sections[i].name, k);
                if(!(scroll & LV_DIR_VER)) {
                    trlv_test_check_int(__FILE__, __LINE__, what, true,
                                        area.y1 - ring >= clip.y1 && area.y2 + ring <= clip.y2);
                }
            }
        }
    }
}

/* Every test, on the wide display and then on the narrow one, with the
 * tokens of one mode. */
static void run_all(const char * mode, const trlv_color_tokens_t * light, const trlv_color_tokens_t * dark,
                    const trlv_layout_tokens_t * layout)
{
    printf("-- %s\n", mode);
    color_light = light;
    color_dark = dark;
    layout_tokens = layout;

    trlv_test_set_display_size(800, 480);
    TRLV_TEST_RUN(frame_fills_the_screen);
    TRLV_TEST_RUN(top_row_puts_the_switch_on_the_right);
    TRLV_TEST_RUN(no_switch_without_dark_tokens);
    TRLV_TEST_RUN(bar_is_a_surface_container);
    TRLV_TEST_RUN(nav_has_a_button_per_section);
    TRLV_TEST_RUN(nav_shows_the_chosen_section);
    TRLV_TEST_RUN(sections_are_freed_when_left);
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

    TRLV_TEST_RUN(controls_show_every_button_variant);
    TRLV_TEST_RUN(controls_show_switch_and_checkbox_states);
    TRLV_TEST_RUN(controls_cover_every_themed_widget);
    TRLV_TEST_RUN(controls_line_up_with_their_headings);

    TRLV_TEST_RUN(list_alternates_items_and_dividers);
    TRLV_TEST_RUN(list_items_put_text_between_icon_and_trailing);

    TRLV_TEST_RUN(switch_recolors_every_section);
    TRLV_TEST_RUN(switch_starts_on_in_dark);

    TRLV_TEST_RUN(keypad_focus_reaches_every_widget);
    TRLV_TEST_RUN(every_focus_ring_has_room);

    trlv_test_set_display_size(320, 480);
    TRLV_TEST_RUN(every_section_fits_and_builds_quietly);
    TRLV_TEST_RUN(nav_scrolls_the_choice_into_view);
    TRLV_TEST_RUN(readouts_on_a_narrow_display);
    TRLV_TEST_RUN(readouts_buttons_wrap);
    TRLV_TEST_RUN(every_focus_ring_has_room);
}

int main(void)
{
    run_all("template colours, Regular layout", &tokens_light, &tokens_dark, &tokens_regular);
    run_all("template colours, Compact layout", &tokens_light, &tokens_dark, &tokens_compact);
    run_all("HMI colours, Regular layout", &tokens_hmi_light, &tokens_hmi_dark, &tokens_regular);
    run_all("HMI colours, Compact layout", &tokens_hmi_light, &tokens_hmi_dark, &tokens_compact);
    return trlv_test_finish();
}
