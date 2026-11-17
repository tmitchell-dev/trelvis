/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo_list.c
 * The List section: a settings list built from rows and dividers, the
 * recommended replacement for lv_list (deprecated in LVGL 9.6 and removed
 * in v10). Each item is an MD3 list item: a leading icon, a headline with
 * supporting text, and a trailing switch, value or chevron. The list
 * scrolls with the demo's content area; in a screen of its own, make its
 * parent scrollable.
 *
 * The layout, as a Figma designer would draw it:
 *
 *     section          column, no gap, fills the width
 *     ├─ heading       Title Medium, padding SM (in line with the icons)
 *     └─ list          column, no gap, fills the width
 *        ├─ item       row, gap LG, padding SM, fills the width, items
 *        │  │          centred   (× 8)
 *        │  ├─ icon    an LVGL symbol, Title Large, centred in a fixed width
 *        │  ├─ text    column, no gap, fills (pushes the trailing item right)
 *        │  │  ├─ headline    Body Large
 *        │  │  └─ supporting  Body Medium, fills the width so it wraps
 *        │  └─ trailing  lv_switch, a Label Large value, or a chevron
 *        └─ divider    between items, the full width
 *
 * The items are plain rows, so the list holds no state of its own and costs
 * nothing per item beyond its objects. The rows themselves are not
 * clickable: clickable MD3 list items, with state layers, are not part of
 * v1. The trailing switches take focus, and the items' padding leaves room
 * for the focus ring.
 *
 * tests/layout/test_demo.c checks this section; keep the two in step.
 */

#include "trlv_demo.h"

typedef enum {
    TRAILING_SWITCH_ON,
    TRAILING_SWITCH_OFF,
    TRAILING_VALUE,
    TRAILING_CHEVRON,
} trailing_t;

static const struct {
    const char * icon;
    const char * headline;
    const char * supporting;
    trailing_t trailing;
    const char * value;   /* for TRAILING_VALUE */
} items[] = {
    { LV_SYMBOL_WIFI,         "Wi-Fi",       "Connected to Workshop",            TRAILING_SWITCH_ON,  NULL },
    { LV_SYMBOL_BLUETOOTH,    "Bluetooth",   "Off",                              TRAILING_SWITCH_OFF, NULL },
    { LV_SYMBOL_BELL,         "Alarm sound", "Plays until the alarm is acknowledged", TRAILING_VALUE, "60 %" },
    { LV_SYMBOL_EYE_OPEN,     "Display",     "Brightness and sleep",             TRAILING_CHEVRON,    NULL },
    { LV_SYMBOL_BATTERY_FULL, "Battery",     "About 6 hours left",               TRAILING_VALUE,      "84 %" },
    { LV_SYMBOL_SETTINGS,     "Calibration", "Last run 12 days ago",             TRAILING_CHEVRON,    NULL },
    { LV_SYMBOL_DOWNLOAD,     "Updates",     "Version 2.4 is ready to install",  TRAILING_CHEVRON,    NULL },
    { LV_SYMBOL_FILE,         "Logs",        "1204 entries since the last export", TRAILING_CHEVRON,  NULL },
};

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void item(lv_obj_t * list, uint32_t index)
{
    lv_obj_t * row = trlv_row(list, TRLV_SPACE_LG);
    trlv_pad(row, TRLV_SPACE_SM);
    trlv_fill_width(row);
    trlv_align_items(row, TRLV_ALIGN_ITEMS_CENTER);

    /* Symbols differ in width; as in MD3, each icon sits in a square (here
     * as wide as its line is tall), so the text lines up from item to item.
     * Width and centring are raw LVGL. */
    lv_obj_t * icon = trlv_text(row, items[index].icon, TRLV_TEXT_TITLE_LARGE);
    lv_obj_set_width(icon, lv_font_get_line_height(lv_obj_get_style_text_font(icon, LV_PART_MAIN)));
    lv_obj_set_style_text_align(icon, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    lv_obj_t * text = trlv_column(row, TRLV_SPACE_NONE);
    trlv_fill_width(text);
    trlv_text(text, items[index].headline, TRLV_TEXT_BODY_LARGE);
    lv_obj_t * supporting = trlv_text(text, items[index].supporting, TRLV_TEXT_BODY_MEDIUM);
    trlv_fill_width(supporting);

    switch(items[index].trailing) {
        case TRAILING_SWITCH_ON:
            lv_obj_add_state(lv_switch_create(row), LV_STATE_CHECKED);
            break;
        case TRAILING_SWITCH_OFF:
            lv_switch_create(row);
            break;
        case TRAILING_VALUE:
            trlv_text(row, items[index].value, TRLV_TEXT_LABEL_LARGE);
            break;
        case TRAILING_CHEVRON:
            trlv_text(row, LV_SYMBOL_RIGHT, TRLV_TEXT_LABEL_LARGE);
            break;
    }
}

lv_obj_t * trlv_demo_list_create(lv_obj_t * parent)
{
    lv_obj_t * section = trlv_column(parent, TRLV_SPACE_NONE);
    trlv_fill_width(section);
    lv_obj_t * heading = trlv_text(section, "Settings", TRLV_TEXT_TITLE_MEDIUM);
    trlv_pad(heading, TRLV_SPACE_SM);   /* in line with the items' icons */

    lv_obj_t * list = trlv_column(section, TRLV_SPACE_NONE);
    trlv_fill_width(list);
    for(uint32_t i = 0; i < COUNT(items); i++) {
        if(i > 0) trlv_divider(list);
        item(list, i);
    }

    return section;
}
