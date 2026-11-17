# Changelog

All notable changes to TreLVis are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the version is 0.x, minor releases may contain breaking changes.

## [Unreleased]

### Added

- `trlv_justify`: main-axis distribution (CSS `justify-content`) with
  shared constant styles, one per value; a second call replaces the first.
- `trlv_align_items`: cross-axis placement (CSS `align-items`). It sets
  LVGL's `flex_track_place` as well as `flex_cross_place`, so items are
  placed in the container as in Figma, not only within their line.
- `trlv_spacer`: an empty box that takes the free space along its parent's
  main axis, warning (as `trlv_spacer`) in a parent that hugs that axis.
- `trlv_text_role`: sets an MD3 text role on any object; on a container it
  reaches every label inside through inheritance.
- `trlv_surface`: fills any object with an MD3 surface or container colour
  and sets the matching `on_` content colour, inherited by everything
  inside. Shared token styles, recoloured on a colour-mode switch.
- `trlv_surface_custom`: fills any object with one of the project's custom
  colours, or its container, and sets the matching content colour. It is in
  the same group as `trlv_surface`. `TRLV_CUSTOM_COLOR_MAX` (at most 8)
  sets how many indexes have styles.
- `trlv_card`: MD3 filled, elevated and outlined cards as shared token
  styles (tone, radius MD, border or level 1 shadow), in the same group as
  the surfaces, recoloured on a colour-mode switch. Padding stays explicit.
- `trlv_button_variant`: restyles an `lv_button` as an MD3 filled, tonal,
  outlined or text button, with pressed and focus state layers and the
  disabled treatment; shape, height and text role stay the theme's.
- `trlv_divider`: an MD3 divider, a `border_width` line in
  `outline_variant`, horizontal in a column and vertical in a row, filling
  across the parent's main axis.
- `TRLV_USE_DEBUG_OUTLINE`: a 1 px magenta outline outside every box, row,
  column and spacer, to compare a layout with its Figma frames.
- The MD3 theme, installed by `trlv_init` over LVGL's default theme (seeded
  with the MD3 `primary` and `secondary` colours, a dark flag from the
  luminance of `surface`, and the Body Medium font), or over no parent with
  `TRLV_THEME_PARENT_NONE`. `TRLV_USE_THEME 0` installs none. So far it
  covers the screen (`surface`, `on_surface`, Body Medium, inherited), plain
  `lv_obj` containers (MD3 colours on LVGL's shape), labels (inherit only)
  and `lv_button` as an MD3 filled button whose content is a centred row.
  `trlv_init` restyles the display's active screen while it is still empty.
  Calling `trlv_init` again reuses the theme and allocates nothing.
- MD3 states on the filled button: pressed and keypad/encoder focus state
  layers pre-mixed from the `state` opacities (LVGL's recolour and grow on
  press are switched off), a focus-key ring in the `focus` colour,
  `focus_width` thick and 2 px outside, and the disabled treatment
  (`on_surface` at 12 % for the container, 38 % for the content).
- MD3 treatment of LVGL's switch, checkbox, slider, bar, arc, spinner,
  dropdown (with its list as an MD3 menu) and textarea: track, indicator,
  handle, box and field colours from the tokens, checked and focused
  states, the shared disabled treatment and focus ring, and LVGL's default
  recolour, grow and focus outline switched off. Fields show focus with a
  `primary` border `focus_width` thick. All recolour on a colour-mode
  switch. The tab buttons of an `lv_tabview` keep the parent theme's look.
- `trlv_theme_set_color_mode(color)`: switches the colour mode at run time.
  The shared styles, including the pre-mixed state colours, are re-set in
  place, LVGL's default theme is re-initialised with the new colours and
  dark flag, and `lv_obj_report_style_change(NULL)` redraws everything.
  Existing objects, built-in widgets included, recolour with no per-screen
  code, and repeated switches use no more memory.
- The v1 public API, settled in the milestone 3 review: `trelvis.h` and its
  module headers, each documented with its Figma / MD3 / CSS equivalents
  and a mapping table, compiling from C99 and C++17. The C++ include test
  pins every public signature. Declared but not implemented yet (calling
  them fails to link until they land): `trlv_spacer`, `trlv_divider`, `trlv_justify`,
  `trlv_align_items`, `trlv_text_role`, `trlv_surface`,
  `trlv_surface_custom`, `trlv_card` and `trlv_button_variant`
  (milestone 5).
- Custom colours: `trlv_color_tokens_t` has a project-defined table of
  `trlv_custom_color_t` entries (`custom`, `custom_count`), named and
  indexed by the project's own enum, for status colours, alarm priorities
  or extra brand accents. `trlv_surface_custom(obj, index, surface)` fills an
  object with an entry's colour or its container, with the matching content
  colour. The `TRLV_CUSTOM_COLOR_MAX` option
  (default 4) sets how many entries it can apply; the token structs have
  the same shape for every configuration.
- Project scaffold: CMake target `trelvis`, `trelvis.mk` for non-CMake builds,
  `trlv_conf.h` configuration mechanism with defaults in `trlv_conf_default.h`,
  `trlv_version.h` with a compile-time check for LVGL 9.6+ (9.x).
- MD3 token types in `trlv_tokens.h`: spacing, radius, size,
  elevation and type-scale enums, and the colour and layout token structs.
- Token template in `templates/` for projects to copy: MD3 baseline light
  and dark colour schemes with example success and warning custom colours, and
  Regular and Compact layout values (Compact: denser spacing, smaller
  controls and corners, tone-only elevation, a smaller type scale). Each
  text role uses the nearest Montserrat size enabled in `lv_conf.h`, with
  its line height, falling back to smaller sizes and then
  `LV_FONT_DEFAULT`, so no font files are needed.
- Example HMI colour scheme in `templates/trlv_tokens_hmi.c`: grey,
  low-saturation light and dark colours in the high-performance HMI style,
  with three alarm priorities as custom colours. Example values only.
- `trlv_init(disp, color, layout)` and the token
  accessors `trlv_color()` and `trlv_layout()`. A `NULL` display or token
  pointer is rejected with a warning. Calling `trlv_init` again (also after
  `lv_deinit` / `lv_init`) updates objects that already use token styles.
- `trlv_box(parent)`: a transparent container with no border or
  padding that hugs its children. Unlike a plain `lv_obj`, it is not
  scrollable or clickable. Its style, `trlv_style_box`, is exposed in
  `trlv_styles.h`.
- `trlv_gap(obj, space)`: the gap between flex or grid children
  from the spacing scale, as a shared style per step. Works on any object;
  calling it again replaces the previous gap.
- `trlv_pad(obj, space)`: padding on all four sides from the
  spacing scale, as a shared style per step. Works on any object; calling it
  again replaces the previous padding.
- `trlv_row(parent, gap)` and `trlv_column(parent, gap)`: a box
  with a horizontal or vertical flex flow and a gap from the spacing scale.
  Their styles, `trlv_style_row` and `trlv_style_column`, are exposed in
  `trlv_styles.h`.
- `trlv_hug_width/height` and `trlv_fill_width/height`. Fill
  grows along the parent's main axis and is 100 % across it (or without a
  flex parent), decided from the parent's layout when it is called. Hug and
  fill on one axis replace each other. Growing along the main axis of a
  parent that hugs that axis logs a warning naming the object, since the
  object always collapses there; filling across a hugging parent stretches
  to the widest sibling and doesn't warn.
- `trlv_text(parent, text, role)`: an `lv_label` with a shared
  style per MD3 type-scale role (font, line spacing and letter spacing from
  the layout tokens). Like any LVGL label, it hugs its text and wraps once it
  has a width (for example from `trlv_fill_width`), and its colour is
  inherited. A token `line_height` in px is converted to LVGL line spacing;
  `0` keeps the font's own line height.
- Demo in `examples/demo`: `trlv_demo_create(parent, light, dark)` builds a
  demo in sections, with a top bar to move between them and a light/dark
  switch, for host projects to show in their own simulator. Each section is
  also a function of its own (`trlv_demo_readouts_create`, ...). Its
  widgets join LVGL's default group, so a keypad or encoder can move
  through them. A headless test checks its layout, colour switching and
  keypad focus in CI.
- CI: GCC and Clang builds against LVGL 9.6 with warnings as errors, C++17
  include test, non-CMake build, allocation and SPDX checks; weekly build
  against LVGL master; tag-driven release workflow.
