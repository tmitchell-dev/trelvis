/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_surface.c
 * Surfaces: MD3 container colours with their inherited content colour.
 */

#include "trelvis/trlv_surface.h"
#include "trlv_internal.h"
#include "trlv_style_group.h"

/*---------------------------------------------------------------------------
 * Token styles, written by trlv_surface_styles_build on trlv_init and on
 * each colour-mode switch. See trlv_internal.h for how token styles work.
 *-------------------------------------------------------------------------*/

/* Surface: bg_color, bg_opa, text_color (inherited), end marker. */
#define SURFACE_PROPS 4

static lv_style_const_prop_t surface_props[TRLV_SURFACE_COUNT][SURFACE_PROPS];

static LV_STYLE_CONST_INIT(surface_surface, surface_props[TRLV_SURFACE_SURFACE]);
static LV_STYLE_CONST_INIT(surface_container_lowest, surface_props[TRLV_SURFACE_CONTAINER_LOWEST]);
static LV_STYLE_CONST_INIT(surface_container_low, surface_props[TRLV_SURFACE_CONTAINER_LOW]);
static LV_STYLE_CONST_INIT(surface_container, surface_props[TRLV_SURFACE_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_container_high, surface_props[TRLV_SURFACE_CONTAINER_HIGH]);
static LV_STYLE_CONST_INIT(surface_container_highest, surface_props[TRLV_SURFACE_CONTAINER_HIGHEST]);
static LV_STYLE_CONST_INIT(surface_primary_container, surface_props[TRLV_SURFACE_PRIMARY_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_secondary_container, surface_props[TRLV_SURFACE_SECONDARY_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_tertiary_container, surface_props[TRLV_SURFACE_TERTIARY_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_error_container, surface_props[TRLV_SURFACE_ERROR_CONTAINER]);
static LV_STYLE_CONST_INIT(surface_inverse, surface_props[TRLV_SURFACE_INVERSE]);

/* Indexed by trlv_surface_t. */
static const lv_style_t * const surface_styles[] = {
    &surface_surface, &surface_container_lowest, &surface_container_low,
    &surface_container, &surface_container_high, &surface_container_highest,
    &surface_primary_container, &surface_secondary_container,
    &surface_tertiary_container, &surface_error_container, &surface_inverse
};
TRLV_STATIC_ASSERT(sizeof(surface_styles) / sizeof(surface_styles[0]) == TRLV_SURFACE_COUNT,
                   surface_styles_cover_the_enum);

/* The fill group: every style that sets an object's container colour.
 * trlv_surface, trlv_surface_custom and trlv_card replace each other. */
static const lv_style_t * const fill_styles[] = {
    &surface_surface, &surface_container_lowest, &surface_container_low,
    &surface_container, &surface_container_high, &surface_container_highest,
    &surface_primary_container, &surface_secondary_container,
    &surface_tertiary_container, &surface_error_container, &surface_inverse
};
static const trlv_style_group_t fill_group = TRLV_STYLE_GROUP_LIST(fill_styles);

/* Write a tone: an opaque container colour and its inherited content colour.
 * Returns the next free entry. */
static lv_style_const_prop_t * put_tone(lv_style_const_prop_t * entry, lv_color_t container,
                                        lv_color_t content)
{
    trlv_prop_set_color(entry++, LV_STYLE_BG_COLOR, container);
    trlv_prop_set_num(entry++, LV_STYLE_BG_OPA, LV_OPA_COVER);
    trlv_prop_set_color(entry++, LV_STYLE_TEXT_COLOR, content);
    return entry;
}

static void build_surface(trlv_surface_t surface, lv_color_t container, lv_color_t content)
{
    trlv_prop_set_end(put_tone(surface_props[surface], container, content));
}

void trlv_surface_styles_build(const trlv_color_tokens_t * color, const trlv_layout_tokens_t * layout)
{
    LV_UNUSED(layout);

    build_surface(TRLV_SURFACE_SURFACE, color->surface, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER_LOWEST, color->surface_container_lowest, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER_LOW, color->surface_container_low, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER, color->surface_container, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER_HIGH, color->surface_container_high, color->on_surface);
    build_surface(TRLV_SURFACE_CONTAINER_HIGHEST, color->surface_container_highest, color->on_surface);
    build_surface(TRLV_SURFACE_PRIMARY_CONTAINER, color->primary_container, color->on_primary_container);
    build_surface(TRLV_SURFACE_SECONDARY_CONTAINER, color->secondary_container,
                  color->on_secondary_container);
    build_surface(TRLV_SURFACE_TERTIARY_CONTAINER, color->tertiary_container,
                  color->on_tertiary_container);
    build_surface(TRLV_SURFACE_ERROR_CONTAINER, color->error_container, color->on_error_container);
    build_surface(TRLV_SURFACE_INVERSE, color->inverse_surface, color->inverse_on_surface);
}

/*---------------------------------------------------------------------------
 * Modifiers
 *-------------------------------------------------------------------------*/

void trlv_surface(lv_obj_t * obj, trlv_surface_t surface)
{
    LV_ASSERT_NULL(obj);

    if((uint32_t)surface >= TRLV_SURFACE_COUNT) {
        LV_LOG_WARN("trlv_surface: %d is not a trlv_surface_t", (int)surface);
        return;
    }
    trlv_style_group_apply(obj, &fill_group, surface_styles[surface], LV_PART_MAIN);
}
