/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file lv_conf.h
 * The consumer project's LVGL configuration: LVGL's defaults (every widget,
 * the default theme), RGB565, and two more Montserrat sizes for the token
 * template's type scale. Found through LV_CONF_INCLUDE_SIMPLE.
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#define LV_COLOR_FORMAT_DEFAULT LV_COLOR_FORMAT_RGB565

#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_24 1

/* lvgl.mk compiles LVGL's examples; this project doesn't need them. */
#define LV_BUILD_EXAMPLES 0

#endif /* LV_CONF_H */
