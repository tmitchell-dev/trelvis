# Roadmap

TreLVis is a small toolset for starting LVGL projects quickly and keeping them
consistent. It is built in milestones. Each milestone has a branch
(`m<N>-<topic>`) and usually one pull request; milestones 4 and 5 may be split
into several small PRs.

## What earns a place

TreLVis stays small on purpose. Its main job is to turn the blocks of LVGL
calls that every project repeats into one standard call. A public function is
added only if it does at least one of these:

- **Replaces a block of LVGL calls that projects repeat**, so it is written
  once and the same way everywhere (`trlv_row(parent, gap)`: create the
  object, reset it, set the flow, set the gap).
- **Takes a design token**, so screens hold no magic numbers
  (`trlv_pad(obj, TRLV_SPACE_LG)`).
- **Fixes an LVGL default that surprises** people coming from Figma or CSS
  (`trlv_box`: not scrollable, not clickable, no theme padding).
- **Makes a decision LVGL leaves to the caller** (`trlv_fill_width`: grow or
  100 %, depending on the parent's direction).

A function that only renames a single clear LVGL call does not qualify. For
those, the [mapping table](docs/concepts.md#10-mapping-figma--md3--css--trelvis--lvgl)
names the LVGL call to use.

**No new surprises for LVGL users.** Where TreLVis has no reason to differ,
LVGL's natural behaviour stands. Every deliberate difference (such as
`trlv_box` not scrolling) is documented on the function.

The v1 budget is about 20 layout and styling functions, plus four for
initialisation and token access.

## Milestones

| # | Milestone | Status |
|---|---|---|
| 1 | **Scaffold**: repository structure, CMake target, `trelvis.mk`, configuration mechanism, version header and LVGL version check, changelog, NOTICE, SPDX headers, CI (ci / weekly / release workflows, Dependabot), C++ include test. Placeholder headers that compile. | done |
| 2 | **Vertical slice**: a first working slice of the library: the token template (`templates/`, copied into a project at setup and compiled in CI), a minimal `trlv_init` (no theme yet), `trlv_box`, `trlv_row`, `trlv_column`, `trlv_gap`, `trlv_pad`, `trlv_fill_*` / `trlv_hug_*` and `trlv_text`. One realistic screen is built with it as `examples/demo`: a plain `trlv_demo_create(parent)` function that any host project's simulator or target can call. A headless layout test harness (build a tree, update the layout, assert coordinates) starts running in CI. The slice is provisional: its job is to prove the approach and settle defaults before the API is fixed. | next |
| 3 | **API review**: full public headers for the v1 API, with doc comments and mapping tables, informed by the slice. The API is reviewed before further implementation. | |
| 4 | **Tokens and theme**: full token types, the complete token template, the MD3 theme over the default parent theme, state layers, focus ring, colour-mode switching, line-height conversion. Widgets are covered in order: screen, label and button first, then one widget at a time. | |
| 5 | **Surfaces, cards and remaining primitives**: `trlv_surface`, `trlv_card`, `trlv_button_variant`, `trlv_text_role`, `trlv_spacer`, `trlv_divider`, `trlv_justify`, `trlv_align_items`, `trlv_absolute`, `trlv_gap_xy`, `trlv_pad_xy`. Layout tests for every primitive and modifier. | |
| 6 | **Demo and docs**: `examples/demo` grows from the slice screen to exercise every primitive: surface container ladder; filled / elevated / outlined cards; all button variants and covered widgets in each state; light/dark toggle; an example Compact token set next to the template values; visible keypad/encoder focus; a list built from rows (no `lv_list`). README quick start, `docs/concepts.md` and `docs/figma-handoff.md` brought up to date with the implementation. | |
| 7 | **v1.1** (see below) | |

## To confirm in the slice

The API questions are decided (see [docs/concepts.md](docs/concepts.md)). Two
behaviours are checked on a real screen in milestone 2:

- **Fill inside a parent that hugs on the same axis.** The child asks to be
  as big as its parent while the parent asks to be as big as its children.
  TreLVis adds no behaviour of its own here: whatever LVGL does stands, and
  the docs say what that is. `trlv_fill_*` logs an `LV_LOG_WARN` when it
  sees the combination, so the cause is visible. Confirm the warning is
  useful and not noisy.
- **`trlv_text` hugging by default.** This is LVGL's native label behaviour;
  confirm it reads well in practice.

## v1 acceptance criteria

- The demo runs in a host project's simulator. Toggling light/dark recolours
  everything, including built-in widgets, with no per-screen code.
- A new project gets a themed screen by copying the token template and
  calling `trlv_init`, before it edits a single value.
- The same demo is correct in compact and regular layout modes.
- Text inside surfaces and cards gets the correct `on_` colour purely through
  inheritance.
- The public API is within the budget above, and every function passes the
  "What earns a place" test.
- Headless layout tests cover every primitive and modifier and pass in CI.
- Builds warning-free (including LVGL deprecations) with
  `-Wall -Wextra -Wpedantic -Werror` on LVGL 9.6; `trelvis.h` compiles from C++17.
- No direct `malloc` / `lv_malloc` in library code (CI grep). No allocation
  outside `trlv_init` and colour-mode switches other than LVGL's own object
  creation.
- Builds by adding `src/` + `include/` alone (CI step).
- Every public function is documented with its Figma / MD3 / CSS equivalent.

## v1.1

Each item is added only when a product needs it:

- `trlv_icon(parent, symbol, TRLV_SIZE_*)`: an icon-font glyph sized by a new
  `icon_size` token, with inherited colour.
- Subject-bound theme switching (`TRLV_USE_SUBJECT_THEME`,
  `trlv_theme_bind_subject`).
- Motion tokens (`duration_short` / `duration_medium`) and style transitions
  for state changes.
- Elevated button variant; selected / activated state styles.
- Hover state layer (`LV_STATE_HOVERED`) for pointer-driven products.
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
  builds weekly against LVGL master. If the move changes the TreLVis API, it
  becomes a new major version, with an LVGL 9 maintenance branch if needed.

## Related project: `trelvis-starter`

A separate template repository for new products is planned. It will pin a
TreLVis release and provide the project skeleton: LVGL and TreLVis
submodules, CMake presets, platform folders (simulator / Linux / STM32), a
`ui/` folder (globals, tokens, fonts, images, components, widgets, screens),
font tooling, and reference components. Scaffolding and templates belong
there, not in this library.

To keep that possible, TreLVis keeps primitives usable from `_create()`-style
functions, keeps every modifier working on any `lv_obj_t`, and keeps project
token values and subjects out of the library.

## Not planned

These are deliberately excluded:

- Wrappers for single clear LVGL calls: visibility, clipping, scrolling,
  wrapping, text truncation, min / max size, grid cells. The mapping table
  names the LVGL call for each.
- `trlv_page` (header / body / footer) and `trlv_grid`. Not every project
  uses them; a page is a project component, and grids use LVGL's grid API.
- Token values compiled into the library. Tokens come from a template the
  project copies and owns, so unused values cost no flash.
- A simulator. Host projects and `trelvis-starter` have their own; TreLVis
  ships a demo function they can call, and headless tests for CI.
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
