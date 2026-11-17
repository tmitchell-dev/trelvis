/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo_readouts.c
 * The Readouts section: the milestone 2 screen, built with box, row,
 * column, gap, fill / hug and text. Raw LVGL is used where the mapping table
 * in docs/concepts.md says so (aligning inside a box, buttons).
 *
 * The layout, as a Figma designer would draw it:
 *
 *     section       column, gap LG, fills the width
 *     ├─ header     frame (no auto layout), fills the width
 *     │  ├─ title   Headline Small, left
 *     │  └─ status  Label Large, right
 *     ├─ readouts   column, gap SM, fills the width
 *     │  └─ readout row, gap MD, fills the width   (× 3)
 *     │     ├─ name   Body Large, fills (pushes the value right)
 *     │     └─ value  Title Medium, hugs
 *     ├─ about      column, gap XS, fills the width
 *     │  ├─ heading Title Medium, hugs
 *     │  └─ body    Body Medium, fills the width so it wraps
 *     └─ actions    row, gap SM, fills the width, wraps
 *        └─ button  lv_button with a Label Large text  (× 3)
 *
 * tests/layout/test_demo.c checks this layout; keep the two in step.
 */

#include "trlv_demo.h"

static void readout(lv_obj_t * parent, const char * name, const char * value)
{
    lv_obj_t * row = trlv_row(parent, TRLV_SPACE_MD);
    trlv_fill_width(row);

    lv_obj_t * name_text = trlv_text(row, name, TRLV_TEXT_BODY_LARGE);
    trlv_fill_width(name_text);
    trlv_text(row, value, TRLV_TEXT_TITLE_MEDIUM);
}

static void button(lv_obj_t * parent, const char * text)
{
    lv_obj_t * btn = lv_button_create(parent);
    trlv_text(btn, text, TRLV_TEXT_LABEL_LARGE);
}

lv_obj_t * trlv_demo_readouts_create(lv_obj_t * parent)
{
    lv_obj_t * section = trlv_column(parent, TRLV_SPACE_LG);
    trlv_fill_width(section);

    /* Header: a box with its children placed by alignment (Figma
     * constraints left and right), so the title and status share a line. */
    lv_obj_t * header = trlv_box(section);
    trlv_fill_width(header);
    lv_obj_t * title = trlv_text(header, "Readouts", TRLV_TEXT_HEADLINE_SMALL);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_t * status = trlv_text(header, "Running", TRLV_TEXT_LABEL_LARGE);
    lv_obj_align(status, LV_ALIGN_RIGHT_MID, 0, 0);

    lv_obj_t * readouts = trlv_column(section, TRLV_SPACE_SM);
    trlv_fill_width(readouts);
    readout(readouts, "Temperature", "21.4 \xC2\xB0" "C");
    readout(readouts, "Pressure", "1013 hPa");
    readout(readouts, "Flow", "2.30 L/min");

    lv_obj_t * about = trlv_column(section, TRLV_SPACE_XS);
    trlv_fill_width(about);
    trlv_text(about, "About", TRLV_TEXT_TITLE_MEDIUM);
    lv_obj_t * body = trlv_text(about,
                                "Readings update once a second. Values outside their normal range "
                                "are reported, and a run can be stopped at any time from this screen "
                                "or from the device.",
                                TRLV_TEXT_BODY_MEDIUM);
    trlv_fill_width(body);   /* a width makes the text wrap (Figma "auto height") */

    /* The buttons wrap onto a second line on a narrow display (Figma auto
     * layout "wrap"); wrapping is raw LVGL and needs a width. */
    lv_obj_t * actions = trlv_row(section, TRLV_SPACE_SM);
    trlv_fill_width(actions);
    lv_obj_set_flex_flow(actions, LV_FLEX_FLOW_ROW_WRAP);
    button(actions, "Start");
    button(actions, "Stop");
    button(actions, "Settings");

    return section;
}
