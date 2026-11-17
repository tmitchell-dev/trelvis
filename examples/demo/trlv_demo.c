/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo.c
 * The demo's frame: a top bar with the light/dark switch and one button per
 * section, and a content area that shows the selected section. The sections
 * are in their own files, trlv_demo_<section>.c.
 *
 * The layout, as a Figma designer would draw it:
 *
 *     demo           column, no gap, fills the parent
 *     ├─ bar         column, no gap, padding SM, fills the width,
 *     │  │           Surface Container
 *     │  ├─ top      row, gap SM, padding SM, fills the width, items centred
 *     │  │  ├─ title   Title Large, fills (pushes the switch right)
 *     │  │  ├─ label   Label Large "Dark"       } only with dark tokens
 *     │  │  └─ switch  lv_switch, on when dark  }
 *     │  └─ nav      row, gap XS, padding SM, fills the width, scrolls
 *     │     │        horizontally
 *     │     └─ button  one per section: tonal when selected, text otherwise
 *     └─ content     column, padding LG, fills the width and the rest of
 *        │           the height, scrolls vertically
 *        └─ section  the selected section (trlv_demo_<section>_create)
 *
 * The top row and the nav row are padded SM because they hold focusable
 * widgets at their edges: LVGL clips children to their parent, and the
 * focus ring is drawn outside the widget (docs/concepts.md, "Focus rings
 * need clearance"). The bar's own SM padding then brings the title and the
 * buttons to 16 px from the edge, in line with the content.
 *
 * Navigation is built from rows and buttons rather than lv_tabview, and
 * the selected section is rebuilt on each choice, so only one section
 * exists at a time. The switch calls trlv_theme_set_color_mode, and the
 * frame and every section recolour with no code of their own.
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
    { "Controls", trlv_demo_controls_create },
};

#define SECTION_COUNT (sizeof(sections) / sizeof(sections[0]))

/* Children of the demo and of its bar, in order. */
enum { DEMO_BAR, DEMO_CONTENT };
enum { BAR_TOP, BAR_NAV };

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

/* The switch has one handler per colour mode, each with its tokens as user
 * data: turning it on applies the dark tokens, turning it off the light. */
static void switched_to_dark(lv_event_t * e)
{
    if(lv_obj_has_state(lv_event_get_current_target_obj(e), LV_STATE_CHECKED)) {
        trlv_theme_set_color_mode((const trlv_color_tokens_t *)lv_event_get_user_data(e));
    }
}

static void switched_to_light(lv_event_t * e)
{
    if(!lv_obj_has_state(lv_event_get_current_target_obj(e), LV_STATE_CHECKED)) {
        trlv_theme_set_color_mode((const trlv_color_tokens_t *)lv_event_get_user_data(e));
    }
}

lv_obj_t * trlv_demo_create(lv_obj_t * parent, const trlv_color_tokens_t * light,
                            const trlv_color_tokens_t * dark)
{
    lv_obj_t * demo = trlv_column(parent, TRLV_SPACE_NONE);
    trlv_fill_width(demo);
    trlv_fill_height(demo);

    lv_obj_t * bar = trlv_column(demo, TRLV_SPACE_NONE);
    trlv_pad(bar, TRLV_SPACE_SM);
    trlv_fill_width(bar);
    trlv_surface(bar, TRLV_SURFACE_CONTAINER);

    lv_obj_t * top = trlv_row(bar, TRLV_SPACE_SM);
    trlv_pad(top, TRLV_SPACE_SM);
    trlv_fill_width(top);
    trlv_align_items(top, TRLV_ALIGN_ITEMS_CENTER);
    lv_obj_t * title = trlv_text(top, "TreLVis", TRLV_TEXT_TITLE_LARGE);
    trlv_fill_width(title);
    if(light != NULL && dark != NULL) {
        trlv_text(top, "Dark", TRLV_TEXT_LABEL_LARGE);
        lv_obj_t * sw = lv_switch_create(top);
        lv_obj_set_state(sw, LV_STATE_CHECKED, trlv_color() == dark);
        /* LVGL's event user data is not const; the handlers cast it back. */
        lv_obj_add_event_cb(sw, switched_to_dark, LV_EVENT_VALUE_CHANGED, (void *)dark);
        lv_obj_add_event_cb(sw, switched_to_light, LV_EVENT_VALUE_CHANGED, (void *)light);
    }

    /* The nav row scrolls sideways when the buttons don't fit (a narrow
     * display). Scrolling is raw LVGL; a row turns it off by default. */
    lv_obj_t * nav = trlv_row(bar, TRLV_SPACE_XS);
    trlv_pad(nav, TRLV_SPACE_SM);
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
