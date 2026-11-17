/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file main.c
 * A product's start-up, headless: create a display, call trlv_init with the
 * copied token template, build the demo, render it in light and dark, and
 * check that TreLVis was built with the product's trlv_conf.h.
 */

#include <stdio.h>

#include <trelvis/trelvis.h>
#include "tokens.h"
#include "trlv_demo.h"

/* Which trlv_conf.h the build should find: 1 on the include path, 2
 * through TRLV_CONF_PATH. Each sets CONSUMER_TRLV_CONF to its number. */
#ifndef CONSUMER_EXPECT_CONF
#  error "build with -DCONSUMER_EXPECT_CONF=1 or 2"
#endif
#if !defined(CONSUMER_TRLV_CONF) || CONSUMER_TRLV_CONF != CONSUMER_EXPECT_CONF
#  error "the product's trlv_conf.h was not found"
#endif

#define WIDTH  800
#define HEIGHT 480
#define BUF_LINES 20

static uint16_t buf[WIDTH * BUF_LINES];   /* RGB565, as in lv_conf.h */
static uint32_t flushes;

static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    flushes++;
    lv_display_flush_ready(disp);
}

static int fail(const char * what)
{
    printf("FAIL: %s\n", what);
    return 1;
}

int main(void)
{
    lv_init();

    lv_display_t * disp = lv_display_create(WIDTH, HEIGHT);
    lv_display_set_flush_cb(disp, flush_cb);
    lv_display_set_buffers(disp, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    trlv_init(disp, &tokens_light, &tokens_regular);

    /* Both trlv_conf.h files turn the debug outline on: the library's own
     * translation units must have seen it, not just this file. */
    lv_style_value_t outline;
    if(lv_style_get_prop(&trlv_style_box, LV_STYLE_OUTLINE_WIDTH, &outline) != LV_STYLE_RES_FOUND) {
        return fail("the library was built without the product's trlv_conf.h");
    }

    lv_obj_t * screen = lv_display_get_screen_active(disp);
    trlv_demo_create(screen, &tokens_light, &tokens_dark);
    if(lv_obj_get_child_count(screen) == 0) return fail("the demo created nothing");

    lv_refr_now(disp);
    if(flushes == 0) return fail("nothing was rendered in light mode");

    flushes = 0;
    trlv_theme_set_color_mode(&tokens_dark);
    lv_refr_now(disp);
    if(flushes == 0) return fail("nothing was rendered in dark mode");

    lv_deinit();
    printf("ok: TreLVis %s, demo rendered in light and dark\n", TRLV_VERSION_STRING);
    return 0;
}
