/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * The first screen of docs/getting-started.md, built as the page shows it.
 * screen_init and settings_create are the page's code: if you change either,
 * change the page in the same PR.
 *
 * It checks what the page says about the screen: the layout tokens chosen
 * by display width, the card and its row laid out as described, the switch
 * and the button reachable by keypad with room for the focus ring, the
 * light/dark button recolouring the card, and no TreLVis warnings.
 */

#include <string.h>

#include "trlv_test.h"
#include "trlv_tokens_template.h"

/*===========================================================================
 * The page's code
 *=========================================================================*/

static void dark_mode_changed(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target_obj(e);
    trlv_theme_set_color_mode(lv_obj_has_state(sw, LV_STATE_CHECKED) ? &tokens_dark : &tokens_light);
}

static lv_obj_t * settings_create(lv_obj_t * parent)
{
    lv_obj_t * page = trlv_column_create(parent);
    trlv_set_gap(page, TRLV_SPACE_LG);
    trlv_set_pad(page, TRLV_SPACE_LG);
    trlv_set_width_fill(page);
    trlv_set_height_fill(page);
    lv_obj_set_scrollable(page, true);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);

    trlv_text_create(page, "Settings", TRLV_TEXT_HEADLINE_SMALL);

    /* A card that is a row: text on the left, a switch on the right. */
    lv_obj_t * card = trlv_row_create(page);
    trlv_set_gap(card, TRLV_SPACE_LG);
    trlv_set_pad(card, TRLV_SPACE_LG);
    trlv_set_width_fill(card);
    trlv_set_align_items(card, TRLV_ALIGN_ITEMS_CENTER);
    trlv_apply_card(card, TRLV_CARD_FILLED);

    lv_obj_t * text = trlv_column_create(card);
    trlv_set_gap(text, TRLV_SPACE_2XS);
    trlv_set_width_fill(text);                       /* grows: the switch goes to the end */
    trlv_text_create(text, "Dark mode", TRLV_TEXT_TITLE_MEDIUM);
    lv_obj_t * hint = trlv_text_create(text, "Easier on the eyes at night.", TRLV_TEXT_BODY_MEDIUM);
    trlv_set_width_fill(hint);                       /* a width makes text wrap */
    trlv_apply_text_emphasis(hint, TRLV_TEXT_EMPHASIS_LOW);

    lv_obj_t * sw = lv_switch_create(card);
    lv_obj_add_event_cb(sw, dark_mode_changed, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * btn = lv_button_create(page);
    trlv_apply_button_type(btn, TRLV_BUTTON_TONAL);
    trlv_text_create(btn, "Restart", TRLV_TEXT_LABEL_LARGE);

    return page;
}

static void screen_init(lv_display_t * disp, lv_indev_t * keypad)
{
    bool compact = lv_display_get_horizontal_resolution(disp) < 480;
    trlv_init(disp, &tokens_light, compact ? &tokens_compact : &tokens_regular);

    lv_group_t * group = lv_group_create();
    lv_group_set_default(group);                     /* before the widgets: they join it */
    lv_indev_set_group(keypad, group);

    settings_create(lv_display_get_screen_active(disp));
}

/*===========================================================================
 * The checks
 *=========================================================================*/

enum { PAGE_TITLE, PAGE_CARD, PAGE_BUTTON };
enum { CARD_TEXT, CARD_SWITCH };

static int trelvis_warnings;

static void log_cb(lv_log_level_t level, const char * buf)
{
    if(level >= LV_LOG_LEVEL_WARN && strstr(buf, "trlv_") != NULL) trelvis_warnings++;
}

static void keypad_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    LV_UNUSED(indev);
    data->state = LV_INDEV_STATE_RELEASED;
}

/* Build the page on `screen`'s display, as the page says, and return it. */
static lv_obj_t * build(lv_obj_t * screen)
{
    lv_indev_t * keypad = lv_indev_create();
    lv_indev_set_type(keypad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(keypad, keypad_read);

    trelvis_warnings = 0;
    lv_log_register_print_cb(log_cb);
    screen_init(lv_obj_get_display(screen), keypad);
    lv_obj_update_layout(screen);
    return lv_obj_get_child(screen, 0);
}

static int32_t space(trlv_space_t step)
{
    return trlv_get_layout_tokens()->space[step];
}

/* `obj` is at least the focus ring's reach inside `ancestor` on every side. */
static void assert_ring_room(lv_obj_t * obj, lv_obj_t * ancestor)
{
    int32_t ring = trlv_get_layout_tokens()->focus_width + space(TRLV_SPACE_2XS);
    lv_area_t a;
    lv_area_t clip;

    lv_obj_get_coords(obj, &a);
    lv_obj_get_coords(ancestor, &clip);
    TRLV_ASSERT_INT(a.x1 - ring >= clip.x1 && a.x2 + ring <= clip.x2, true);
    TRLV_ASSERT_INT(a.y1 - ring >= clip.y1 && a.y2 + ring <= clip.y2, true);
}

/* The page fills the screen; the card fills the page's content width; the
 * text column grows and pushes the switch to the card's inner edge. */
static void check_layout(lv_obj_t * screen)
{
    lv_obj_t * page = build(screen);
    lv_obj_t * card = lv_obj_get_child(page, PAGE_CARD);
    lv_obj_t * text = lv_obj_get_child(card, CARD_TEXT);
    lv_obj_t * sw = lv_obj_get_child(card, CARD_SWITCH);
    int32_t pad = space(TRLV_SPACE_LG);

    TRLV_ASSERT_RECT(page, 0, 0, lv_obj_get_width(screen), lv_obj_get_height(screen));
    TRLV_ASSERT_X(card, pad);
    TRLV_ASSERT_WIDTH(card, lv_obj_get_width(screen) - 2 * pad);
    TRLV_ASSERT_X(text, pad);
    TRLV_ASSERT_INT(trlv_test_x(sw) + lv_obj_get_width(sw), lv_obj_get_width(card) - pad);
    TRLV_ASSERT_WIDTH(text, lv_obj_get_width(card) - 2 * pad - space(TRLV_SPACE_LG) - lv_obj_get_width(sw));
    TRLV_ASSERT_INT(trelvis_warnings, 0);
}

static void layout_regular(lv_obj_t * screen)
{
    check_layout(screen);
    TRLV_ASSERT_INT(trlv_get_layout_tokens() == &tokens_regular, true);
}

static void layout_compact(lv_obj_t * screen)
{
    check_layout(screen);
    TRLV_ASSERT_INT(trlv_get_layout_tokens() == &tokens_compact, true);
}

/* The keypad reaches the switch and the button, each with room for its
 * ring inside the card and the page. */
static void keypad_reaches_both_controls(lv_obj_t * screen)
{
    lv_obj_t * page = build(screen);
    lv_obj_t * card = lv_obj_get_child(page, PAGE_CARD);
    lv_obj_t * sw = lv_obj_get_child(card, CARD_SWITCH);
    lv_obj_t * btn = lv_obj_get_child(page, PAGE_BUTTON);
    lv_group_t * group = lv_group_get_default();

    TRLV_ASSERT_INT((int32_t)lv_group_get_obj_count(group), 2);
    lv_group_focus_obj(sw);
    TRLV_ASSERT_INT(lv_obj_has_state(sw, LV_STATE_FOCUSED), true);
    lv_group_focus_next(group);
    TRLV_ASSERT_INT(lv_group_get_focused(group) == btn, true);

    assert_ring_room(sw, card);
    assert_ring_room(btn, page);
}

/* Turning the switch on recolours the card to the dark scheme, and off
 * again to the light one. */
static void switch_changes_color_mode(lv_obj_t * screen)
{
    lv_obj_t * page = build(screen);
    lv_obj_t * card = lv_obj_get_child(page, PAGE_CARD);
    lv_obj_t * sw = lv_obj_get_child(card, CARD_SWITCH);

    lv_obj_set_state(sw, LV_STATE_CHECKED, true);
    lv_obj_send_event(sw, LV_EVENT_VALUE_CHANGED, NULL);
    TRLV_ASSERT_INT((int32_t)lv_color_to_u32(lv_obj_get_style_bg_color(card, LV_PART_MAIN)),
                    (int32_t)lv_color_to_u32(tokens_dark.surface_container_highest));

    lv_obj_set_state(sw, LV_STATE_CHECKED, false);
    lv_obj_send_event(sw, LV_EVENT_VALUE_CHANGED, NULL);
    TRLV_ASSERT_INT((int32_t)lv_color_to_u32(lv_obj_get_style_bg_color(card, LV_PART_MAIN)),
                    (int32_t)lv_color_to_u32(tokens_light.surface_container_highest));
}

int main(void)
{
    trlv_test_set_display_size(800, 480);
    TRLV_TEST_RUN(layout_regular);
    TRLV_TEST_RUN(keypad_reaches_both_controls);
    TRLV_TEST_RUN(switch_changes_color_mode);

    trlv_test_set_display_size(320, 240);
    TRLV_TEST_RUN(layout_compact);
    TRLV_TEST_RUN(keypad_reaches_both_controls);
    return trlv_test_finish();
}
