/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file product_trlv_conf.h
 * A TreLVis configuration with another name, outside the include path,
 * given with TRLV_CONF_PATH. It takes precedence over the trlv_conf.h on
 * the include path. main.c checks that it was used.
 */

#ifndef PRODUCT_TRLV_CONF_H
#define PRODUCT_TRLV_CONF_H

#define CONSUMER_TRLV_CONF 2

/* Observable in the library's own build (trlv_style_box gets an outline). */
#define TRLV_USE_DEBUG_OUTLINE 1

#endif /* PRODUCT_TRLV_CONF_H */
