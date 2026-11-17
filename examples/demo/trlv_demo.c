/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo.c
 * The demo's frame: a top bar with one button per section, and a content
 * area that shows the selected section. The sections are in their own
 * files, trlv_demo_<section>.c.
 *
 * The layout, as a Figma designer would draw it:
 *
 *     demo           column, no gap, fills the parent
 *     ├─ bar         column, gap SM, padding LG, fills the width,
 *     │  │           Surface Container
 *     │  ├─ title    Title Large
 *     │  └─ nav      row, gap XS, fills the width, scrolls horizontally
 *     │     └─ button  one per section: tonal when selected, text otherwise
 *     └─ content     column, padding LG, fills the width and the rest of
 *        │           the height, scrolls vertically
 *        └─ section  the selected section (trlv_demo_<section>_create)
 *
 * Navigation is built from rows and buttons rather than lv_tabview, and
 * the selected section is rebuilt on each choice, so only one section
 * exists at a time.
 *
 * tests/layout/test_demo.c checks this layout; keep the two in step.
 */

#include "trlv_demo.h"

/* One entry per section, in the order of the nav buttons. Constant, so the
 * table lives in flash. */
typedef struct {
    const char * name;
    lv_obj_t * (*create)(lv_obj_t * parent);
} demo_section_t;

static const demo_section_t sections[] = {
    { "Readouts", trlv_demo_readouts_create },
    { "Surfaces", trlv_demo_surfaces_create },
    { "Cards",    trlv_demo_cards_create },
};

#define SECTION_COUNT (sizeof(sections) / sizeof(sections[0]))

/* Children of the demo and of its bar, in order. */
enum { DEMO_BAR, DEMO_CONTENT };
enum { BAR_TITLE, BAR_NAV };

/* Mark the nav button of section `index` as selected, and build that
 * section in place of the previous one. */
static void show_section(lv_obj_t * demo, uint32_t index)
{
    lv_obj_t * nav = lv_obj_get_child(lv_obj_get_child(demo, DEMO_BAR), BAR_NAV);
    lv_obj_t * content = lv_obj_get_child(demo, DEMO_CONTENT);

    for(uint32_t i = 0; i < SECTION_COUNT; i++) {
        trlv_button_variant(lv_obj_get_child(nav, (int32_t)i),
                            i == index ? TRLV_BUTTON_TONAL : TRLV_BUTTON_TEXT);
    }
    /* On a narrow display the nav row scrolls: bring the choice into view. */
    lv_obj_scroll_to_view(lv_obj_get_child(nav, (int32_t)index), LV_ANIM_OFF);

    lv_obj_clean(content);
    lv_obj_scroll_to_y(content, 0, LV_ANIM_OFF);
    sections[index].create(content);
}

static void nav_clicked(lv_event_t * e)
{
    lv_obj_t * btn = lv_event_get_current_target_obj(e);
    lv_obj_t * demo = (lv_obj_t *)lv_event_get_user_data(e);

    show_section(demo, (uint32_t)lv_obj_get_index(btn));
}

lv_obj_t * trlv_demo_create(lv_obj_t * parent)
{
    lv_obj_t * demo = trlv_column(parent, TRLV_SPACE_NONE);
    trlv_fill_width(demo);
    trlv_fill_height(demo);

    lv_obj_t * bar = trlv_column(demo, TRLV_SPACE_SM);
    trlv_pad(bar, TRLV_SPACE_LG);
    trlv_fill_width(bar);
    trlv_surface(bar, TRLV_SURFACE_CONTAINER);
    trlv_text(bar, "TreLVis", TRLV_TEXT_TITLE_LARGE);

    /* The nav row scrolls sideways when the buttons don't fit (a narrow
     * display). Scrolling is raw LVGL; a row turns it off by default. */
    lv_obj_t * nav = trlv_row(bar, TRLV_SPACE_XS);
    trlv_fill_width(nav);
    lv_obj_set_scrollable(nav, true);
    lv_obj_set_scroll_dir(nav, LV_DIR_HOR);
    for(uint32_t i = 0; i < SECTION_COUNT; i++) {
        lv_obj_t * btn = lv_button_create(nav);
        trlv_text(btn, sections[i].name, TRLV_TEXT_LABEL_LARGE);
        lv_obj_add_event_cb(btn, nav_clicked, LV_EVENT_CLICKED, demo);
    }

    /* The content takes the rest of the height (fill in a column grows) and
     * scrolls when the section is taller. */
    lv_obj_t * content = trlv_column(demo, TRLV_SPACE_NONE);
    trlv_pad(content, TRLV_SPACE_LG);
    trlv_fill_width(content);
    trlv_fill_height(content);
    lv_obj_set_scrollable(content, true);
    lv_obj_set_scroll_dir(content, LV_DIR_VER);

    show_section(demo, 0);
    return demo;
}
