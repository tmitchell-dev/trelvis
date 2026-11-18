/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo_cards.c
 * The Cards section: the three MD3 card types with realistic content. Each
 * card is a column with trlv_apply_card applied and its padding set with
 * trlv_set_pad, as in Figma, and its text takes on_surface by inheritance.
 *
 * The layout, as a Figma designer would draw it:
 *
 *     section        column, gap LG, fills the width
 *     └─ card        column, gap SM, padding LG, fills the width   (× 3: filled,
 *        │           elevated, outlined)
 *        ├─ type     Label Medium: the card type
 *        ├─ title    Title Medium
 *        ├─ body     Body Medium, fills the width so it wraps
 *        └─ actions  row, gap SM, fills the width, packed at the end, wraps
 *           ├─ button  text button: the secondary action
 *           └─ button  filled button: the main action
 *
 * The outlined card's border sits inside its size and insets the content by
 * its width, on top of the padding (Figma: inside stroke).
 *
 * tests/layout/test_demo.c checks this section; keep the two in step.
 */

#include "trlv_demo.h"

typedef struct {
    trlv_card_t card;
    const char * type;
    const char * title;
    const char * body;
    const char * secondary;   /* text button */
    const char * main;        /* filled button */
} demo_card_t;

static const demo_card_t cards[] = {
    {
        TRLV_CARD_FILLED, "Filled card", "Calibration due",
        "The flow sensor was last calibrated eleven months ago. Book a "
        "calibration before the end of the month to keep readings in tolerance.",
        "Later", "Book"
    },
    {
        TRLV_CARD_ELEVATED, "Elevated card", "Daily summary",
        "Three runs finished today with no alarms. The longest took 2 h 14 min.",
        "Share", "View report"
    },
    {
        TRLV_CARD_OUTLINED, "Outlined card", "Update available",
        "Version 2.4 starts faster and adds two languages. The device "
        "restarts to install it.",
        "Not now", "Install"
    },
};

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

static void button(lv_obj_t * parent, const char * text, trlv_button_t type)
{
    lv_obj_t * btn = lv_button_create(parent);
    trlv_apply_button_type(btn, type);
    trlv_text_create(btn, text, TRLV_TEXT_LABEL_LARGE);
}

static void card(lv_obj_t * section, const demo_card_t * content)
{
    lv_obj_t * obj = trlv_column_create(section);
    trlv_apply_card(obj, content->card);
    trlv_set_pad(obj, TRLV_SPACE_LG);
    trlv_set_gap(obj, TRLV_SPACE_SM);
    trlv_set_width_fill(obj);

    trlv_text_create(obj, content->type, TRLV_TEXT_LABEL_MEDIUM);
    trlv_text_create(obj, content->title, TRLV_TEXT_TITLE_MEDIUM);
    lv_obj_t * body = trlv_text_create(obj, content->body, TRLV_TEXT_BODY_MEDIUM);
    trlv_set_width_fill(body);

    /* Actions at the end of the card (Figma: auto layout packed right);
     * on a narrow display they wrap rather than overflow. The padding is
     * room for the keypad focus ring (docs/concepts.md, "Focus rings need
     * clearance"). */
    lv_obj_t * actions = trlv_row_create(obj);
    trlv_set_gap(actions, TRLV_SPACE_SM);
    trlv_set_pad(actions, TRLV_SPACE_SM);
    trlv_set_width_fill(actions);
    trlv_set_justify(actions, TRLV_JUSTIFY_END);
    lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW_WRAP);
    button(actions, content->secondary, TRLV_BUTTON_TEXT);
    button(actions, content->main, TRLV_BUTTON_FILLED);
}

lv_obj_t * trlv_demo_cards_create(lv_obj_t * parent)
{
    lv_obj_t * section = trlv_column_create(parent);
    trlv_set_gap(section, TRLV_SPACE_LG);
    trlv_set_width_fill(section);

    for(uint32_t i = 0; i < COUNT(cards); i++) card(section, &cards[i]);

    return section;
}
