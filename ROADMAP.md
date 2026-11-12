# Roadmap

TreLVis is built in milestones. Each milestone is one branch and one pull
request (`m<N>-<topic>`).

| # | Milestone | Status |
|---|---|---|
| 1 | **Scaffold**: repository structure, CMake target, `trelvis.mk`, configuration mechanism, version header and LVGL version check, changelog, NOTICE, SPDX headers, CI (ci / nightly / release workflows, Dependabot), C++ include test. Placeholder headers that compile. | in progress |
| 2 | **API review**: full public headers with doc comments and mapping tables. The API is reviewed before implementation starts. | next |
| 3 | **Tokens, styles, theme**: `trlv_init`, constant and token styles, MD3 theme over the default parent theme, widget coverage, state layers, focus ring, colour-mode switching, line-height conversion. | |
| 4 | **Primitives and modifiers v1**: constructors, layout modifiers incl. axis-aware fill, surfaces, cards, button variants, text roles, `trlv_page`. | |
| 5 | **Demo and simulator**: `examples/demo` exercising every primitive; surface container ladder; filled / elevated / outlined cards; all button variants and covered widgets in each state; light/dark toggle; layout mode chosen by CLI argument; visible keypad/encoder focus; a list built from rows (no `lv_list`). `examples/sim`: SDL simulator fetching pinned LVGL 9.6. `examples/tokens_example.c/.h`: MD3 baseline tokens. | |
| 6 | **Docs**: README quick start, `docs/concepts.md`, `docs/figma-handoff.md` brought up to date with the implementation. | |
| 7 | **v1.1** (see below) | |

## v1 acceptance criteria

- The simulator runs the demo. Toggling light/dark recolours everything,
  including built-in widgets, with no per-screen code.
- The same demo is correct in compact and regular layout modes.
- Text inside surfaces and cards gets the correct `on_` colour purely through
  inheritance.
- Builds warning-free (including LVGL deprecations) with
  `-Wall -Wextra -Wpedantic -Werror` on LVGL 9.6; `trelvis.h` compiles from C++17.
- No direct `malloc` / `lv_malloc` in library code (CI grep). No allocation
  outside `trlv_init` and colour-mode switches other than LVGL's own object
  creation.
- Builds by adding `src/` + `include/` alone (CI step).
- Every public function is documented with its Figma / MD3 / CSS equivalent.

## v1.1

- `trlv_icon(parent, symbol, TRLV_SIZE_*)`: an icon-font glyph sized by the
  `icon_size` token, with inherited colour.
- Subject-bound theme switching (`TRLV_USE_SUBJECT_THEME`,
  `trlv_theme_bind_subject`).
- Motion tokens (`duration_short` / `duration_medium`) and style transitions
  for state changes.
- Elevated button variant; selected / activated state styles.
- `trlv_debug_dump(obj)`: prints the object tree with sizes, padding, gaps and
  styles for comparison with Figma (`TRLV_USE_DEBUG`).

## Later (designed for, not yet built)

- **C++ facade:** header-only `trelvis.hpp` with non-owning handles, chainable
  setters and a lambda event helper (heap use switchable off). C++17, no
  exceptions or RTTI.
- **Token generator:** DTCG (2025.10) JSON and Material Theme Builder JSON →
  the same token structs.
- **Responsive helpers:** e.g. switching between row and column by layout mode.
- **LVGL v10:** TreLVis already follows v10 forward-compatibility rules and
  builds nightly against LVGL master. If the move changes the TreLVis API, it
  becomes a new major version, with an LVGL 9 maintenance branch if needed.

## Related project: `trelvis-starter`

A separate template repository for new products is planned. It will pin a
TreLVis release and provide the project skeleton: LVGL and TreLVis
submodules, CMake presets, platform folders (simulator / Linux / STM32), an
LVGL-Pro-parallel `ui/` folder (globals, tokens, fonts, images, components,
widgets, screens), font tooling, and reference components. Scaffolding and
templates belong there, not in this library.

To keep that possible, TreLVis keeps primitives usable from `_create()`-style
functions, keeps every modifier working on any `lv_obj_t`, and keeps token
values and subjects out of the library.

## Not planned

These are deliberately excluded:

- Ripples, MD3 motion specs and MD3 Expressive additions; full MD3 component
  specs.
- Per-corner radii, corner smoothing.
- Gradient tokens, blend modes, inner shadows, background blur.
- Aspect-ratio locking; Figma "scale" constraints.
- Breakpoints / media queries (layout modes are used instead).
- Separate `overlay` / `center` / `fixed` helpers (`trlv_box` + `trlv_absolute`
  and `lv_obj_center` cover these).
- LVGL colour filters for state layers.
- Wrapping LVGL widgets, macro DSLs, varargs builders.
- LVGL Pro artefacts or tooling (XML, `globals.xml`, runtime loading, editor
  previews).
