/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * The demo screen (examples/demo) on a fixed-size display: key objects at the
 * expected positions, nothing overflowing its parent, the body text wrapping,
 * and no TreLVis warnings while it is built. Text sizes are computed from the
 * font with lv_text_get_size, so the tests don't depend on LV_FONT_DEFAULT's
 * metrics. The object tree is described at the top of trlv_demo.c.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"
#include "trlv_demo.h"

#include <string.h>

/* Token values from the template's Regular layout. */
#define PAGE_PAD     24   /* TRLV_SPACE_XL */
#define PAGE_GAP     16   /* TRLV_SPACE_LG */
#define READOUTS_GAP 8    /* TRLV_SPACE_SM */
#define READOUT_GAP  12   /* TRLV_SPACE_MD */
#define ABOUT_GAP    4    /* TRLV_SPACE_XS */
#define ACTIONS_GAP  8    /* TRLV_SPACE_SM */

/* Children of the page, in order. */
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

static lv_obj_t * part(lv_obj_t * page, int32_t index)
{
    return lv_obj_get_child(page, index);
}

/* The tests build LVGL with its default lv_conf, where Montserrat 14 is the
 * only font. Every text role then uses LV_FONT_DEFAULT, so a one-line text
 * is that font's line height whatever its role. */
static int32_t line_height(void)
{
    return lv_font_get_line_height(LV_FONT_DEFAULT);
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

/* Every descendant of `obj` lies inside its parent's outer box. */
static void assert_nothing_overflows(lv_obj_t * obj, const char * path)
{
    uint32_t count = lv_obj_get_child_count(obj);
    for(uint32_t i = 0; i < count; i++) {
        lv_obj_t * child = lv_obj_get_child(obj, (int32_t)i);
        char child_path[64];
        char what[96];
        lv_snprintf(child_path, sizeof(child_path), "%s/%" LV_PRIu32, path, i);

        lv_snprintf(what, sizeof(what), "%s is inside on the left and top", child_path);
        trlv_test_check_int(__FILE__, __LINE__, what, true,
                            trlv_test_x(child) >= 0 && trlv_test_y(child) >= 0);
        lv_snprintf(what, sizeof(what), "%s is inside on the right", child_path);
        trlv_test_check_int(__FILE__, __LINE__, what, true,
                            trlv_test_x(child) + trlv_test_width(child) <= trlv_test_width(obj));
        lv_snprintf(what, sizeof(what), "%s is inside at the bottom", child_path);
        trlv_test_check_int(__FILE__, __LINE__, what, true,
                            trlv_test_y(child) + trlv_test_height(child) <= trlv_test_height(obj));

        assert_nothing_overflows(child, child_path);
    }
}

/* The page fills the screen, with the four sections stacked inside its padding. */
static void page_fills_the_screen(lv_obj_t * screen)
{
    lv_obj_t * page = build(screen);
    int32_t content_width = 800 - 2 * PAGE_PAD;
    int32_t y = PAGE_PAD;

    TRLV_ASSERT_RECT(page, 0, 0, 800, 480);

    for(int32_t i = HEADER; i <= ACTIONS; i++) {
        lv_obj_t * section = part(page, i);
        TRLV_ASSERT_X(section, PAGE_PAD);
        TRLV_ASSERT_Y(section, y);
        y += trlv_test_height(section) + PAGE_GAP;
    }
    TRLV_ASSERT_WIDTH(part(page, HEADER), content_width);
    TRLV_ASSERT_WIDTH(part(page, READOUTS), content_width);
    TRLV_ASSERT_WIDTH(part(page, ABOUT), content_width);
}

/* The header box hugs the title's height; the title is on the left and the
 * status on the right. */
static void header_places_title_and_status(lv_obj_t * screen)
{
    lv_obj_t * header = part(build(screen), HEADER);
    lv_obj_t * title = lv_obj_get_child(header, 0);
    lv_obj_t * status = lv_obj_get_child(header, 1);
    int32_t width = 800 - 2 * PAGE_PAD;

    TRLV_ASSERT_HEIGHT(header, line_height());
    TRLV_ASSERT_RECT(title, 0, 0, text_width(title), line_height());
    TRLV_ASSERT_RECT(status, width - text_width(status), 0, text_width(status), line_height());
}

/* Each readout fills the width: the name grows, so the value sits on the right. */
static void readouts_put_values_on_the_right(lv_obj_t * screen)
{
    lv_obj_t * readouts = part(build(screen), READOUTS);
    int32_t width = 800 - 2 * PAGE_PAD;

    TRLV_ASSERT_INT(lv_obj_get_child_count(readouts), 3);
    TRLV_ASSERT_HEIGHT(readouts, 3 * line_height() + 2 * READOUTS_GAP);

    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * row = lv_obj_get_child(readouts, i);
        lv_obj_t * name = lv_obj_get_child(row, 0);
        lv_obj_t * value = lv_obj_get_child(row, 1);
        int32_t value_width = text_width(value);

        TRLV_ASSERT_RECT(row, 0, i * (line_height() + READOUTS_GAP), width, line_height());
        TRLV_ASSERT_RECT(name, 0, 0, width - READOUT_GAP - value_width, line_height());
        TRLV_ASSERT_RECT(value, width - value_width, 0, value_width, line_height());
    }
}

/* The body text fills the width and wraps onto more lines. */
static void body_text_wraps(lv_obj_t * screen)
{
    lv_obj_t * about = part(build(screen), ABOUT);
    lv_obj_t * heading = lv_obj_get_child(about, 0);
    lv_obj_t * body = lv_obj_get_child(about, 1);
    int32_t width = 800 - 2 * PAGE_PAD;

    TRLV_ASSERT_RECT(heading, 0, 0, text_width(heading), line_height());
    TRLV_ASSERT_RECT(body, 0, line_height() + ABOUT_GAP, width, wrapped_height(body, width));
    TRLV_ASSERT_INT(trlv_test_height(body) > line_height(), true);
}

/* The buttons hug their labels and sit side by side, the gap apart. */
static void buttons_sit_in_a_row(lv_obj_t * screen)
{
    lv_obj_t * actions = part(build(screen), ACTIONS);
    int32_t x = 0;

    TRLV_ASSERT_INT(lv_obj_get_child_count(actions), 3);
    for(int32_t i = 0; i < 3; i++) {
        lv_obj_t * btn = lv_obj_get_child(actions, i);
        lv_obj_t * label = lv_obj_get_child(btn, 0);
        TRLV_ASSERT_X(btn, x);
        TRLV_ASSERT_Y(btn, 0);
        TRLV_ASSERT_WIDTH(label, text_width(label));
        TRLV_ASSERT_HEIGHT(btn, trlv_test_height(actions));
        x += trlv_test_width(btn) + ACTIONS_GAP;
    }
    TRLV_ASSERT_WIDTH(actions, x - ACTIONS_GAP);
}

/* Nothing overflows its parent, and the demo raises no TreLVis warnings
 * (it sizes every parent before filling its children). */
static void fits_and_builds_quietly(lv_obj_t * screen)
{
    lv_obj_t * page = build(screen);

    assert_nothing_overflows(page, "page");
    TRLV_ASSERT_INT(trelvis_warnings, 0);
}

/* On a narrow, portrait display the body wraps onto more lines and
 * everything still fits across. */
static void narrow_display(lv_obj_t * screen)
{
    lv_obj_t * page = build(screen);
    lv_obj_t * body = lv_obj_get_child(part(page, ABOUT), 1);
    int32_t width = 320 - 2 * PAGE_PAD;

    TRLV_ASSERT_RECT(page, 0, 0, 320, 480);
    TRLV_ASSERT_WIDTH(body, width);
    TRLV_ASSERT_HEIGHT(body, wrapped_height(body, width));
    assert_nothing_overflows(page, "page");
    TRLV_ASSERT_INT(trelvis_warnings, 0);
}

int main(void)
{
    TRLV_TEST_RUN(page_fills_the_screen);
    TRLV_TEST_RUN(header_places_title_and_status);
    TRLV_TEST_RUN(readouts_put_values_on_the_right);
    TRLV_TEST_RUN(body_text_wraps);
    TRLV_TEST_RUN(buttons_sit_in_a_row);
    TRLV_TEST_RUN(fits_and_builds_quietly);

    trlv_test_set_display_size(320, 480);
    TRLV_TEST_RUN(narrow_display);
    return trlv_test_finish();
}
