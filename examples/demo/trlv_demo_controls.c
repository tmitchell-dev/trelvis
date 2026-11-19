/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo_controls.c
 * The Controls section: the four MD3 button types, the three toggle
 * button types, and every LVGL widget the theme covers, in their default,
 * checked and disabled states where they apply. Pressed and focused states come from using them, with a pointer or
 * with a keypad or encoder.
 *
 * The widgets are plain LVGL widgets: the theme styles them as they are
 * created. TreLVis only places them, and trlv_apply_button_type picks the
 * button type.
 *
 * The layout, as a Figma designer would draw it:
 *
 *     section        column, gap SM, fills the width
 *     └─ group       column, no gap, fills the width   (× 4)
 *        ├─ heading  Title Medium, padding SM (in line with the controls)
 *        └─ row      row, gap SM, padding SM, fills the width, wraps,
 *                    items centred   (one or more per group; the field
 *                    rows don't wrap: their two fields fill half each)
 *
 * Every row that holds widgets is padded SM: LVGL clips children to their
 * parent, and the focus ring is drawn outside the widget, so the padding
 * keeps it visible (docs/concepts.md, "Focus rings need clearance"). The
 * heading has the same padding so that it lines up with the controls.
 *
 * tests/layout/test_demo.c checks this section; keep the two in step.
 */

#include "trlv_demo.h"

/* Arcs have no size of their own in LVGL's theme; this is the dial's size. */
#define ARC_SIZE 96

static const struct {
    trlv_button_type_t type;
    const char * name;
} buttons[] = {
    { TRLV_BUTTON_FILLED,   "Filled" },
    { TRLV_BUTTON_TONAL,    "Tonal" },
    { TRLV_BUTTON_OUTLINED, "Outlined" },
    { TRLV_BUTTON_TEXT,     "Text" },
};

/* Toggle buttons, each shown unselected and selected. */
static const struct {
    trlv_button_type_t type;
    const char * name;
} toggles[] = {
    { TRLV_BUTTON_FILLED_TOGGLE,   "Filled" },
    { TRLV_BUTTON_TONAL_TOGGLE,    "Tonal" },
    { TRLV_BUTTON_OUTLINED_TOGGLE, "Outlined" },
};

/* The states shown for switches and checkboxes. */
static const struct {
    lv_state_t state;
    const char * name;
} states[] = {
    { LV_STATE_DEFAULT,                     "Off" },
    { LV_STATE_CHECKED,                     "On" },
    { LV_STATE_DISABLED,                    "Off, disabled" },
    { LV_STATE_CHECKED | LV_STATE_DISABLED, "On, disabled" },
};

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A group: a heading and the column that holds its rows. */
static lv_obj_t * group(lv_obj_t * section, const char * heading)
{
    lv_obj_t * obj = trlv_column_create(section);
    trlv_set_width_fill(obj);
    lv_obj_t * text = trlv_text_create(obj, heading, TRLV_TEXT_TITLE_MEDIUM);
    trlv_set_pad(text, TRLV_SPACE_SM);
    return obj;
}

/* A row of widgets that wraps on a narrow display, with room for the
 * focus ring around them. */
static lv_obj_t * widget_row(lv_obj_t * group_obj, trlv_space_t gap)
{
    lv_obj_t * row = trlv_row_create(group_obj);
    trlv_set_gap(row, gap);
    trlv_set_pad(row, TRLV_SPACE_SM);
    trlv_set_width_fill(row);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW_WRAP);
    trlv_set_align_items(row, TRLV_ALIGN_ITEMS_CENTER);
    return row;
}

static void button_group(lv_obj_t * section)
{
    lv_obj_t * obj = group(section, "Buttons");

    /* One row enabled, one disabled (disabled widgets don't take focus). */
    for(uint32_t disabled = 0; disabled < 2; disabled++) {
        lv_obj_t * row = widget_row(obj, TRLV_SPACE_SM);
        for(uint32_t i = 0; i < COUNT(buttons); i++) {
            lv_obj_t * btn = lv_button_create(row);
            trlv_apply_button_type(btn, buttons[i].type);
            trlv_text_create(btn, buttons[i].name, TRLV_TEXT_LABEL_LARGE);
            if(disabled) lv_obj_add_state(btn, LV_STATE_DISABLED);
        }
    }

    /* Toggle buttons: checkable, unselected then selected. Clicking one
     * toggles it. */
    lv_obj_t * row = widget_row(obj, TRLV_SPACE_SM);
    for(uint32_t i = 0; i < COUNT(toggles); i++) {
        for(uint32_t selected = 0; selected < 2; selected++) {
            lv_obj_t * btn = lv_button_create(row);
            lv_obj_set_checkable(btn, true);
            trlv_apply_button_type(btn, toggles[i].type);
            trlv_text_create(btn, toggles[i].name, TRLV_TEXT_LABEL_LARGE);
            if(selected) lv_obj_add_state(btn, LV_STATE_CHECKED);
        }
    }
}

static void selection_group(lv_obj_t * section)
{
    lv_obj_t * obj = group(section, "Switches and checkboxes");

    lv_obj_t * row = widget_row(obj, TRLV_SPACE_LG);
    for(uint32_t i = 0; i < COUNT(states); i++) {
        lv_obj_add_state(lv_switch_create(row), states[i].state);
    }

    row = widget_row(obj, TRLV_SPACE_LG);
    for(uint32_t i = 0; i < COUNT(states); i++) {
        lv_obj_t * box = lv_checkbox_create(row);
        lv_checkbox_set_text(box, states[i].name);
        lv_obj_add_state(box, states[i].state);
    }
}

static void range_group(lv_obj_t * section)
{
    lv_obj_t * obj = group(section, "Sliders and progress");

    /* Sliders and the bar fill the width, one per row. */
    lv_obj_t * row = widget_row(obj, TRLV_SPACE_SM);
    lv_obj_t * slider = lv_slider_create(row);
    trlv_set_width_fill(slider);
    lv_slider_set_value(slider, 60, LV_ANIM_OFF);

    row = widget_row(obj, TRLV_SPACE_SM);
    slider = lv_slider_create(row);
    trlv_set_width_fill(slider);
    lv_slider_set_value(slider, 30, LV_ANIM_OFF);
    lv_obj_add_state(slider, LV_STATE_DISABLED);

    row = widget_row(obj, TRLV_SPACE_SM);
    lv_obj_t * bar = lv_bar_create(row);
    trlv_set_width_fill(bar);
    lv_bar_set_value(bar, 70, LV_ANIM_OFF);

    row = widget_row(obj, TRLV_SPACE_LG);
    lv_obj_t * arc = lv_arc_create(row);
    lv_obj_set_size(arc, ARC_SIZE, ARC_SIZE);
    lv_arc_set_value(arc, 60);
    arc = lv_arc_create(row);
    lv_obj_set_size(arc, ARC_SIZE, ARC_SIZE);
    lv_arc_set_value(arc, 40);
    lv_obj_add_state(arc, LV_STATE_DISABLED);
    lv_obj_t * spinner = lv_spinner_create(row);
    int32_t spinner_size = trlv_get_control_height(TRLV_SIZE_MD);
    lv_obj_set_size(spinner, spinner_size, spinner_size);
}

static void field_group(lv_obj_t * section)
{
    lv_obj_t * obj = group(section, "Fields");

    /* Two fields per row, each filling half (fill in a row grows). Filling
     * items don't wrap in LVGL, so these rows don't wrap. */
    for(uint32_t disabled = 0; disabled < 2; disabled++) {
        lv_obj_t * row = trlv_row_create(obj);
        trlv_set_gap(row, TRLV_SPACE_SM);
        trlv_set_pad(row, TRLV_SPACE_SM);
        trlv_set_width_fill(row);

        lv_obj_t * dropdown = lv_dropdown_create(row);
        lv_dropdown_set_options_static(dropdown, "Metric\nImperial");
        trlv_set_width_fill(dropdown);

        lv_obj_t * field = lv_textarea_create(row);
        lv_textarea_set_one_line(field, true);
        trlv_set_width_fill(field);
        if(disabled) {
            lv_textarea_set_text(field, "Read only");
            lv_obj_add_state(dropdown, LV_STATE_DISABLED);
            lv_obj_add_state(field, LV_STATE_DISABLED);
        }
        else {
            lv_textarea_set_placeholder_text(field, "Device name");
        }
    }
}

lv_obj_t * trlv_demo_controls_create(lv_obj_t * parent)
{
    lv_obj_t * section = trlv_column_create(parent);
    trlv_set_gap(section, TRLV_SPACE_SM);
    trlv_set_width_fill(section);

    button_group(section);
    selection_group(section);
    range_group(section);
    field_group(section);

    return section;
}
