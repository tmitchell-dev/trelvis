/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file lv_conf.h
 * A minimal LVGL configuration, in the shape a small product uses: RGB565,
 * no logging, one font, no default theme and no widgets besides the label.
 * CI builds TreLVis and the tests that don't need widgets against it
 * (cmake -DTRLV_TEST_CONFIG=minimal), so every `#if LV_USE_*` guard and the
 * TRLV_THEME_PARENT_NONE path compile. The Cortex-M footprint job uses it
 * too.
 *
 * Options not set here keep LVGL's defaults (lv_conf_internal.h).
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_FORMAT_DEFAULT LV_COLOR_FORMAT_RGB565

#define LV_USE_LOG 0
#define LV_USE_OBSERVER 0
#define LV_USE_GRID 0

/* Fonts: only LVGL's default Montserrat 14. */
#define LV_FONT_MONTSERRAT_14 1

/* Themes: TreLVis is the only one (TRLV_THEME_PARENT_NONE in trlv_conf.h). */
#define LV_USE_THEME_DEFAULT 0
#define LV_USE_THEME_SIMPLE 0
#define LV_USE_THEME_MONO 0

/* Widgets: only the label, which TreLVis text needs. Every widget the
 * TreLVis theme styles is off, and so is everything built from them. */
#define LV_USE_ANIMIMG 0
#define LV_USE_ARC 0
#define LV_USE_ARCLABEL 0
#define LV_USE_BAR 0
#define LV_USE_BUTTON 0
#define LV_USE_BUTTONMATRIX 0
#define LV_USE_CALENDAR 0
#define LV_USE_CANVAS 0
#define LV_USE_CHART 0
#define LV_USE_CHECKBOX 0
#define LV_USE_DROPDOWN 0
#define LV_USE_IMAGE 0
#define LV_USE_IMAGEBUTTON 0
#define LV_USE_KEYBOARD 0
#define LV_USE_LABEL 1
#define LV_USE_LED 0
#define LV_USE_LINE 0
#define LV_USE_LIST 0
#define LV_USE_MENU 0
#define LV_USE_MSGBOX 0
#define LV_USE_ROLLER 0
#define LV_USE_SCALE 0
#define LV_USE_SLIDER 0
#define LV_USE_SPAN 0
#define LV_USE_SPINBOX 0
#define LV_USE_SPINNER 0
#define LV_USE_SWITCH 0
#define LV_USE_TABLE 0
#define LV_USE_TABVIEW 0
#define LV_USE_TEXTAREA 0
#define LV_USE_TILEVIEW 0
#define LV_USE_WIN 0

#endif /* LV_CONF_H */
