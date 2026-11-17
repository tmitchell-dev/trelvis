/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_version.h
 * TreLVis version, and the compile-time check for a supported LVGL version.
 *
 * This is the only place the version is defined: CMakeLists.txt reads it
 * from here, and the release workflow checks the git tag against it.
 */

#ifndef TRLV_VERSION_H
#define TRLV_VERSION_H

#include "trlv_conf_default.h"
#include TRLV_LVGL_INCLUDE

/*---------------------------------------------------------------------------
 * TreLVis version (semantic versioning)
 *-------------------------------------------------------------------------*/
#define TRLV_VERSION_MAJOR  0
#define TRLV_VERSION_MINOR  1
#define TRLV_VERSION_PATCH  0
#define TRLV_VERSION_STRING "0.1.0"

/*---------------------------------------------------------------------------
 * Supported LVGL: 9.x with x >= 6
 *-------------------------------------------------------------------------*/
#if !TRLV_IGNORE_LVGL_VERSION
#  if !defined(LVGL_VERSION_MAJOR) || LVGL_VERSION_MAJOR != 9 || LVGL_VERSION_MINOR < 6
#    error "TreLVis requires LVGL 9.6 or a later 9.x release. See the compatibility table in README.md."
#  endif
#endif

#endif /* TRLV_VERSION_H */
