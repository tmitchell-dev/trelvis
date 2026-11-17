# Contributing to TreLVis

Thanks for your interest in TreLVis. This guide explains how the project is
run, what fits its scope, and the rules code must follow to be merged.

TreLVis is at an early stage (0.x) and the public API is still being designed.
For anything bigger than a small fix, please open an issue first so we can
agree on the approach before you write code.

## Scope

TreLVis is a **utility layer, not a framework**. Before proposing a feature,
check it fits:

**In scope:** layout primitives and modifiers (Figma auto layout / CSS
flexbox), Material Design 3 token *types* and a token template projects copy,
an MD3 theme for LVGL's built-in widgets using LVGL's theme and style engine,
surfaces, cards, text roles, and documentation for the Figma hand-off.

**What earns a place.** TreLVis stays small on purpose. Its main job is to
turn the blocks of LVGL calls that every project repeats into one standard
call. A new public function must do at least one of these:

- replace a block of LVGL calls that projects repeat, so it is written once
  and the same way everywhere;
- take a design token, so screens hold no magic numbers;
- fix an LVGL default that surprises people coming from Figma or CSS;
- make a decision LVGL leaves to the caller (for example axis-aware fill).

A function that only renames a single clear LVGL call does not qualify.
Nothing may surprise someone who knows LVGL: where TreLVis has no reason to
differ, LVGL's natural behaviour stands, and deliberate differences are
documented on the function.
Add a row to the mapping table in [docs/concepts.md](docs/concepts.md)
instead, naming the LVGL call to use.

**Out of scope** (pull requests adding these will be declined):

- Wrapping LVGL widgets, or wrapping LVGL calls that are already clear
  (`lv_obj_set_size`, `lv_obj_center`, `lv_obj_set_hidden`,
  `lv_obj_set_scrollable`, …).
- Macro DSLs, varargs builders or other "clever" APIs that hide LVGL.
- `_px` variants of token-based setters. Raw LVGL is the escape hatch.
- Project-specific content: token *values* compiled into the library (they
  start from the copied template and live in the project), fonts, images, screens, components.
- A pixel-perfect Material implementation: ripples, MD3 motion specs, MD3
  Expressive, full component specs.
- Anything that depends on LVGL Pro or its artefacts (XML, `globals.xml`, …).

[ROADMAP.md](ROADMAP.md) lists what is planned and what is deliberately left
out.

## Development setup

Prerequisites: Git, CMake ≥ 3.16 and a C99 / C++17 compiler (GCC or Clang).

```sh
git clone https://github.com/tmitchell-dev/trelvis.git
cd trelvis
git config core.hooksPath .githooks   # shared hooks: blocks direct pushes to main

cmake -B build                        # fetches LVGL v9.6 on first run
cmake --build build
ctest --test-dir build --output-on-failure
```

Built on its own, TreLVis fetches LVGL and compiles the library, the token
template, the C++17 include test and the headless layout tests with
`-Wall -Wextra -Wpedantic -Werror`, which is the same build CI runs (see
[`tests/CMakeLists.txt`](tests/CMakeLists.txt)). The layout tests build an
object tree on a test display that renders nothing, let LVGL lay it out and
check positions and sizes; see [`tests/harness/trlv_test.h`](tests/harness/trlv_test.h).
Useful options:

- `-DFETCHCONTENT_SOURCE_DIR_LVGL=path/to/lvgl` uses a local LVGL tree.
- `-DTRLV_LVGL_GIT_TAG=master` builds against another LVGL version. For
  LVGL 10, also pass `-DCMAKE_C_FLAGS=-DTRLV_IGNORE_LVGL_VERSION=1`.
- `-DTRLV_TEST_CONFIG=minimal` builds against the small product's
  configuration in [`tests/config/minimal`](tests/config/minimal) (RGB565,
  no log, one font, no default theme, no widgets but the label) and runs
  the tests that need no widgets.

CI also builds that minimal configuration and compiles the theme with each
widget it styles turned on alone; builds [`tests/consumer`](tests/consumer),
a stand-in product, with CMake and with a Makefile; cross-compiles for
Cortex-M4 and M7 and reports the footprint
([`tests/footprint.sh`](tests/footprint.sh)); runs Doxygen to check every
public symbol is documented; and checks that library code doesn't allocate
directly, that every source file has an SPDX header, and that the sources
compile without CMake.

## Workflow

TreLVis uses **GitHub flow**:

- `main` is the only long-lived branch and always builds. **Never commit or
  push directly to `main`.**
- Make one short-lived branch per change: `feat/<topic>`, `fix/<topic>`,
  `docs/<topic>`, `ci/<topic>`, or `m<N>-<topic>` for roadmap milestones.
- Open a pull request. CI must pass. PRs are **squash-merged**, so the PR
  title becomes the commit message on `main`.
- Keep PRs small and focused.

### Commit messages and PR titles

Write for readers who don't know the context. Use the imperative mood with a
short summary line (≤ 72 characters), for example *Add axis-aware fill
modifiers*. The body should explain *why*, not just *what*.

### Changelog

Every PR that changes behaviour or the public API adds an entry under
**Unreleased** in [CHANGELOG.md](CHANGELOG.md), using the
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/) categories (Added,
Changed, Deprecated, Removed, Fixed, Security). Changes to docs, CI or tests
only don't need an entry.

The entry must let a project judge the upgrade against the
[compatibility promise](README.md#what-a-version-number-promises):

- A change to the theme's look goes under *Changed*, naming what looks
  different.
- A change to a token struct ends with a **Migration:** note saying what to
  add to or change in a copied token template. A new token field must be
  safe at zero (fall back to a documented value and log a warning), and the
  templates keep using designated initialisers.
- A deprecation goes under *Deprecated* with the replacement, and the doc
  comment gets a matching `@deprecated` note.

## Coding rules

These rules keep TreLVis portable, small and ready for LVGL v10. CI enforces
the ones it can.

### Language and build

- **C99** for library code (LVGL's requirement). No compiler extensions in
  library code.
- Every public header has `extern "C"` guards and must compile from **C++17**.
  No compound literals or other C-only constructs in headers.
- Clean under `-Wall -Wextra -Wpedantic -Werror` with GCC and Clang,
  **including LVGL deprecation warnings**. Never suppress a warning; fix it.
- No build-time code generation. A project must be able to build TreLVis by
  adding `src/` and `include/` to its build.
- No OS, HAL or platform calls.
- If you add a source file, list it in `CMakeLists.txt`. `trelvis.mk` picks up
  `src/*.c` automatically.

### LVGL usage (v10 forward-compatibility)

LVGL 9.6 deprecates what v10 removes. Follow these rules:

- Include LVGL only through `TRLV_LVGL_INCLUDE` (default `<lvgl/lvgl.h>`).
  Never include headers under `lvgl/src/`, `lvgl_private.h`, or rely on
  `LV_USE_PRIVATE_API`. Use public API only, for example public theme
  functions rather than theme struct internals.
- Object flags: use the per-flag setters and getters (`lv_obj_set_scrollable()`,
  `lv_obj_is_scrollable()`, …), not `lv_obj_add_flag` / `remove_flag` /
  `has_flag`.
- Never pass `NULL` as a display.
- Subjects: use the pointer-based API (`lv_subject_create(LV_SUBJECT_TYPE_*)`,
  `lv_subject_set_*`, `lv_obj_bind_*`, `lv_subject_add_observer_obj`), not
  `lv_subject_init_*`.
- Object lookup: `lv_obj_find_by_name()`, not `lv_obj_find_by_id()`.
- Don't read removed or derived config options such as `LV_COLOR_DEPTH`. Use
  `LV_COLOR_FORMAT_DEFAULT` if the colour format is needed.
- Don't use the deprecated composites `lv_list`, `lv_win` or `lv_menu`. Build
  them from flex `lv_obj`s instead.
- Guard code for a widget with its `LV_USE_*` option, styles included, so a
  product that turns the widget off pays nothing for it. CI builds the
  theme with each styled widget on alone.
- Keep areas likely to change upstream in one place: all theme integration in
  `trlv_theme.c`, and all label and text specifics in `trlv_text.c`.

### Memory

- Library code **never calls `malloc` / `lv_malloc` (or friends) directly**.
  CI checks this with a grep.
- Constant styles use `LV_STYLE_CONST_INIT`, so they live in flash and use no RAM.
- Token styles use `LV_STYLE_CONST_INIT` too, with a static, writable property
  array in RAM. Build functions write the values into it; they never call
  `lv_style_init` or `lv_style_set_*`, so styles allocate nothing (see
  `src/trlv_internal.h`).
- LVGL's other internal allocations (theme objects) happen **only in
  `trlv_init` and colour-mode switches**, and are bounded.
- Modifiers apply shared styles, not per-object local styles. Where a local
  style can't be avoided (unusual padding, gap or size values), document the
  cost in the function's doc comment.
- Token structs are taken as `const` pointers.

### Styling

- Everything visual or layout-related is a **style property** on a shared
  style, with part/state selectors. Don't style inside event callbacks.
- A modifier group **replaces** its previous style rather than stacking a new
  one. Aim for 2–4 TreLVis styles per object.
- Colour-mode switches re-set existing shared styles and call
  `lv_obj_report_style_change(NULL)`. Never walk the object tree.

See [docs/concepts.md](docs/concepts.md) for the full design.

### Naming

- Prefixes: functions `trlv_*`, types `trlv_*_t`, macros and enums `TRLV_*`,
  config options `TRLV_USE_*` / `TRLV_*`, header guards `TRLV_*_H`, library
  styles `trlv_style_*`.
- Vocabulary priority: **Figma / MD3 term → CSS term → LVGL term**. Never
  invent a fourth vocabulary.
- Avoid words that clash with LVGL or CSS: `frame`, `float`, `stack`, `layer`,
  `canvas`, `screen`. Don't use `align` for flex distribution
  (`lv_obj_align` means positioning).
- Constructors are **nouns** that create objects (`trlv_row(parent, …)`).
  Modifiers are **verbs or properties** that work on any object
  (`trlv_justify(obj, …)`).
- No abbreviations in the public API except LVGL's `pad`: `trlv_column`,
  `trlv_fill_width`, `trlv_align_items`.
- Enum values are fully qualified: `TRLV_JUSTIFY_SPACE_BETWEEN`,
  `TRLV_TEXT_BODY_MEDIUM`.
- Token names are MD3 role names in snake_case: *Surface Container High* →
  `surface_container_high`.

### Style and documentation

- Follow LVGL's formatting: 4-space indents, `type * name` pointer spacing,
  `/* */` comments in C. See [.editorconfig](.editorconfig).
- Every source and header file starts with:

  ```c
  /* SPDX-License-Identifier: MIT */
  /* Copyright (c) 2026 Timothy Mitchell */
  ```

- Every public function has a Doxygen-style comment that names its
  **Figma / MD3 / CSS equivalent** and any surprising LVGL behaviour it
  handles. Each public header includes a mapping table (Figma/MD3 → CSS →
  `trlv` → LVGL mechanism).
- Every public symbol (function, type, field, enum value, macro) is
  documented, with every parameter and return value. CI runs `doxygen`
  with the [Doxyfile](Doxyfile) and fails on a warning; run it locally to
  see the HTML reference in `build-doxygen/html`.
- Prefer clarity to cleverness. Code should be readable by someone new to LVGL.
- All public functions are called from the LVGL thread only. Say so wherever
  it's not obvious.

## Third-party material

Only add third-party material (fonts, icons, images, code) under a licence
compatible with MIT, such as SIL OFL for fonts or Apache-2.0 for Material
Symbols. Keep the licence file next to the asset and list the item in
[NOTICE](NOTICE).

Don't add anything specific to a company or product: names, internal URLs,
colour palettes, assets or data. The token template uses the generic MD3 baseline.
Never commit secrets.

## Versioning and releases

TreLVis follows [Semantic Versioning](https://semver.org). In 0.x, minor
releases may break the API. 1.0.0 follows once the API has been proven in a
real product. What counts as breaking (public headers, configuration
options, token struct shapes, public style names, supported LVGL versions)
and how the theme's look, deprecations and LVGL versions are handled is in
[Compatibility](README.md#what-a-version-number-promises).

Maintainers release like this:

1. In a PR, choose the version from the *Unreleased* entries using the
   compatibility rules: any breaking change makes a major release, any
   *Added*, *Changed* (including the theme's look) or *Deprecated* entry a
   minor one, and only *Fixed* or *Security* entries a patch. Check that
   every token schema change has its **Migration** note. Bump the version
   in `include/trelvis/trlv_version.h` (the only place it is defined; CMake
   reads it from there) and move the *Unreleased* changelog entries under a
   new `## [X.Y.Z] - YYYY-MM-DD` heading.
2. Squash-merge the PR.
3. Tag `main` and push the tag:
   `git tag -a vX.Y.Z -m "TreLVis X.Y.Z" && git push origin vX.Y.Z`.
4. The `release.yml` workflow runs CI, checks that the tag matches the header,
   and creates the GitHub Release from the changelog section.

Maintenance branches (`release/vX.x`) are created only when an older line
needs fixes while `main` moves on, for example an LVGL 9 line after the move
to LVGL v10.

## Product-trial findings

Before 1.0.0, anything found while using TreLVis in a real product (a bug,
a missing piece, a surprise, raw LVGL written more than once) is filed with
the *Product-trial finding* issue form, which asks whether a fix would be
breaking or additive. Breaking fixes go in before the 1.0 API freeze;
additive ones can wait for v1.x.

## Reporting bugs

Open an issue with your TreLVis and LVGL versions, target platform and
compiler, a minimal code sample, and what you expected to happen versus what
happened. For security issues, follow [SECURITY.md](SECURITY.md) instead.
