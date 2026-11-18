/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_demo_surfaces.c
 * The Surfaces section: every MD3 surface and container role, and the
 * project's custom colours, as swatches with their name on them. Each
 * swatch sets only its fill with trlv_apply_surface or trlv_apply_surface_custom; the
 * text on it gets the matching `on_` colour by inheritance, with no colour
 * of its own.
 *
 * The layout, as a Figma designer would draw it:
 *
 *     section        column, gap LG, fills the width
 *     └─ group       column, gap SM, fills the width   (× 2, or 3 with custom colours)
 *        ├─ heading  Title Medium
 *        └─ swatches column, no gap, fills the width
 *           └─ swatch  row, padding MD, fills the width, a surface
 *              └─ name   Body Large, fills the width (wraps if narrow)
 *
 * The groups are the surface ladder (Surface up to Surface Container
 * Highest, the tones of a screen's layers), the accent containers with the
 * inverse surface, and the custom colours, each as its strong colour and
 * its container. The custom colour group lists the tokens' custom colours
 * up to TRLV_CUSTOM_COLOR_MAX, and is left out if there are none.
 *
 * tests/layout/test_demo.c checks this section; keep the two in step.
 */

#include "trlv_demo.h"

typedef struct {
    trlv_surface_t surface;
    const char * name;
} surface_swatch_t;

static const surface_swatch_t ladder[] = {
    { TRLV_SURFACE_SURFACE,           "Surface" },
    { TRLV_SURFACE_CONTAINER_LOWEST,  "Surface Container Lowest" },
    { TRLV_SURFACE_CONTAINER_LOW,     "Surface Container Low" },
    { TRLV_SURFACE_CONTAINER,         "Surface Container" },
    { TRLV_SURFACE_CONTAINER_HIGH,    "Surface Container High" },
    { TRLV_SURFACE_CONTAINER_HIGHEST, "Surface Container Highest" },
};

static const surface_swatch_t containers[] = {
    { TRLV_SURFACE_PRIMARY_CONTAINER,   "Primary Container" },
    { TRLV_SURFACE_SECONDARY_CONTAINER, "Secondary Container" },
    { TRLV_SURFACE_TERTIARY_CONTAINER,  "Tertiary Container" },
    { TRLV_SURFACE_ERROR_CONTAINER,     "Error Container" },
    { TRLV_SURFACE_INVERSE,             "Inverse Surface" },
};

#define COUNT(array) (sizeof(array) / sizeof((array)[0]))

/* A heading and an empty column for the swatches; returns the column. */
static lv_obj_t * group(lv_obj_t * section, const char * heading)
{
    lv_obj_t * obj = trlv_column_create(section);
    trlv_set_gap(obj, TRLV_SPACE_SM);
    trlv_set_width_fill(obj);
    trlv_text_create(obj, heading, TRLV_TEXT_TITLE_MEDIUM);

    lv_obj_t * swatches = trlv_column_create(obj);
    trlv_set_width_fill(swatches);
    return swatches;
}

/* A swatch with its name; the caller gives it its fill. */
static lv_obj_t * swatch(lv_obj_t * swatches, const char * name)
{
    lv_obj_t * row = trlv_row_create(swatches);
    trlv_set_pad(row, TRLV_SPACE_MD);
    trlv_set_width_fill(row);

    lv_obj_t * text = trlv_text_create(row, name, TRLV_TEXT_BODY_LARGE);
    trlv_set_width_fill(text);   /* wraps a long name on a narrow display */
    return row;
}

static void surface_group(lv_obj_t * section, const char * heading,
                          const surface_swatch_t * list, uint32_t count)
{
    lv_obj_t * swatches = group(section, heading);
    for(uint32_t i = 0; i < count; i++) {
        trlv_apply_surface(swatch(swatches, list[i].name), list[i].surface);
    }
}

lv_obj_t * trlv_demo_surfaces_create(lv_obj_t * parent)
{
    lv_obj_t * section = trlv_column_create(parent);
    trlv_set_gap(section, TRLV_SPACE_LG);
    trlv_set_width_fill(section);

    surface_group(section, "Surface ladder", ladder, COUNT(ladder));
    surface_group(section, "Containers", containers, COUNT(containers));

    /* Custom colours have no names in the tokens (the project's enum names
     * them), so they are numbered. */
    uint32_t custom_count = LV_MIN(trlv_get_color_tokens()->custom_count, (uint32_t)TRLV_CUSTOM_COLOR_MAX);
    if(custom_count > 0) {
        lv_obj_t * swatches = group(section, "Custom colours");
        for(uint32_t i = 0; i < custom_count; i++) {
            char name[32];
            lv_snprintf(name, sizeof(name), "Custom %" LV_PRIu32, i + 1);
            trlv_apply_surface_custom(swatch(swatches, name), i, TRLV_CUSTOM_SURFACE_COLOR);
            lv_snprintf(name, sizeof(name), "Custom %" LV_PRIu32 " Container", i + 1);
            trlv_apply_surface_custom(swatch(swatches, name), i, TRLV_CUSTOM_SURFACE_CONTAINER);
        }
    }

    return section;
}
