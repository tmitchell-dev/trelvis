/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Compile-only test: the public API must be usable from C++17, warning-free.
 */

#include <trelvis/trelvis.h>

static_assert(TRLV_VERSION_MAJOR >= 0, "TreLVis headers must be usable from C++");
