/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_conf.h
 * The consumer project's TreLVis configuration, next to its lv_conf.h and
 * found on the include path. main.c checks that it was used.
 */

#ifndef TRLV_CONF_H
#define TRLV_CONF_H

#define CONSUMER_TRLV_CONF 1

/* Observable in the library's own build (trlv_style_box gets an outline). */
#define TRLV_USE_DEBUG_OUTLINE 1

#endif /* TRLV_CONF_H */
