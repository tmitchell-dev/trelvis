# Changelog

All notable changes to TreLVis are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the version is 0.x, minor releases may contain breaking changes.

## [Unreleased]

### Added

- Project scaffold: CMake target `trelvis`, `trelvis.mk` for non-CMake builds,
  `trlv_conf.h` configuration mechanism with defaults in `trlv_conf_default.h`,
  `trlv_version.h` with a compile-time check for LVGL 9.6+ (9.x).
- Placeholder public headers (`trelvis.h` and modules), compiling from C99 and C++17.
- Provisional MD3 token types in `trlv_tokens.h`: spacing, radius, size,
  elevation and type-scale enums, and the colour and layout token structs.
- Token template in `templates/` for projects to copy: MD3 baseline light
  and dark colour schemes with example success and warning colours, and
  Regular layout values with `LV_FONT_DEFAULT` for every text role.
- Provisional `trlv_init(disp, color, layout)` (no theme yet) and the token
  accessors `trlv_color()` and `trlv_layout()`. A `NULL` display or token
  pointer is rejected with a warning. Calling `trlv_init` again (also after
  `lv_deinit` / `lv_init`) updates objects that already use token styles.
- Provisional `trlv_box(parent)`: a transparent container with no border or
  padding that hugs its children. Unlike a plain `lv_obj`, it is not
  scrollable or clickable. Its style, `trlv_style_box`, is exposed in
  `trlv_styles.h`.
- Provisional `trlv_gap(obj, space)`: the gap between flex or grid children
  from the spacing scale, as a shared style per step. Works on any object;
  calling it again replaces the previous gap.
- Provisional `trlv_pad(obj, space)`: padding on all four sides from the
  spacing scale, as a shared style per step. Works on any object; calling it
  again replaces the previous padding.
- Provisional `trlv_row(parent, gap)` and `trlv_column(parent, gap)`: a box
  with a horizontal or vertical flex flow and a gap from the spacing scale.
  Their styles, `trlv_style_row` and `trlv_style_column`, are exposed in
  `trlv_styles.h`.
- Provisional `trlv_hug_width/height` and `trlv_fill_width/height`. Fill
  grows along the parent's main axis and is 100 % across it (or without a
  flex parent), decided from the parent's layout when it is called. Hug and
  fill on one axis replace each other. Growing along the main axis of a
  parent that hugs that axis logs a warning naming the object, since the
  object always collapses there; filling across a hugging parent stretches
  to the widest sibling and doesn't warn.
- Provisional `trlv_text(parent, text, role)`: an `lv_label` with a shared
  style per MD3 type-scale role (font, line spacing and letter spacing from
  the layout tokens). Like any LVGL label, it hugs its text and wraps once it
  has a width (for example from `trlv_fill_width`), and its colour is
  inherited. A token `line_height` in px is converted to LVGL line spacing;
  `0` keeps the font's own line height.
- Demo screen in `examples/demo`: `trlv_demo_create(parent)` builds one
  realistic screen with the slice, for host projects to show in their own
  simulator. A headless test checks its layout in CI.
- CI: GCC and Clang builds against LVGL 9.6 with warnings as errors, C++17
  include test, non-CMake build, allocation and SPDX checks; weekly build
  against LVGL master; tag-driven release workflow.
