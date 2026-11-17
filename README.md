# TreLVis

*A light and versatile layout layer for [LVGL](https://lvgl.io).*

[![CI](https://github.com/tmitchell-dev/trelvis/actions/workflows/ci.yml/badge.svg)](https://github.com/tmitchell-dev/trelvis/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/license-MIT-blue.svg)](LICENSE)

TreLVis is a small C utility layer on top of LVGL that lets people who know
Figma, HTML/CSS or Qt build LVGL user interfaces in a familiar way, and makes
the hand-off from a Figma design to LVGL code close to mechanical. Its design
system is [Material Design 3](https://m3.material.io) (MD3): tokens use MD3
names, and LVGL's widgets get an MD3 look out of the box. A design made with
the Material 3 Figma kit and Material Theme Builder therefore maps straight
onto code.

- **Layout primitives**: box, row, column, spacer, text and divider, with
  Figma auto-layout sizing (hug / fill / fixed) and CSS-style alignment.
- **Material Design 3 tokens**: colour roles, surface containers, type scale,
  shape scale, state layers and elevation, plus a 4 px spacing grid. Defined
  once per project and named exactly like MD3 and the Figma variables. A
  token template with the generic MD3 baseline is copied into the project at
  setup, so it works before any value is edited and costs nothing unused.
- **An MD3 theme built on LVGL's own theme and style engine**: LVGL's built-in
  widgets get an MD3 look automatically. Colour modes (light / dark / brand)
  switch at runtime; layout modes (compact / regular / …) are chosen at start-up.
- **Surfaces and cards**: MD3 tonal surface containers and filled, elevated and
  outlined cards. Text inside them picks up the right colour through inheritance.
- **Figma hand-off conventions** aligned with the Material 3 Figma kit and
  Material Theme Builder.

It is a **utility layer, not a framework**: a small toolset for starting LVGL
projects quickly and keeping them consistent. Every function returns or takes
a plain `lv_obj_t *`, every modifier works on any LVGL object, and raw `lv_*`
calls stay valid everywhere. TreLVis does not wrap LVGL widgets; it styles them
through LVGL's theme engine. It also doesn't rename LVGL calls that are
already clear: for those, the docs name the LVGL call to use.

> **Status: early development (0.x).** Every function of the v1 API is
> built and covered by headless tests, and a [demo](examples/demo) shows
> the whole library in light and dark, Regular and Compact. The API may
> still change in any 0.x release; 1.0 follows once it has been proven in
> a real product. See [ROADMAP.md](ROADMAP.md) and what 1.0 will
> [promise](#what-a-version-number-promises).

## A taste of the API

The same card built with TreLVis and with plain LVGL 9.6 under LVGL's
default theme. Each side runs after a one-off set-up: for TreLVis,
`trlv_init(disp, &tokens_light, &tokens_regular)` with tokens from the
template you copy; for plain LVGL, a palette for light and dark and shared
styles for the card, the text roles and each button state (75 lines). A
[headless test](tests/layout/test_readme_card.c) checks that both versions
lay out the same and get the same fonts and colours, in light and dark.

<!-- The code below is copied from tests/layout/test_readme_card.c, with
     blank lines in the TreLVis column so each call lines up with the LVGL
     calls it replaces. Change them together, and the line counts. -->

<table>
<tr><th>TreLVis</th><th>Plain LVGL 9.6</th></tr>
<tr>
<td valign="top">

```c
lv_obj_t * card = trlv_column(parent, TRLV_SPACE_SM);




trlv_card(card, TRLV_CARD_FILLED);
trlv_pad(card, TRLV_SPACE_LG);
trlv_fill_width(card);

lv_obj_t * header = trlv_row(card, TRLV_SPACE_SM);




trlv_fill_width(header);
trlv_justify(header, TRLV_JUSTIFY_SPACE_BETWEEN);
trlv_align_items(header, TRLV_ALIGN_ITEMS_CENTER);

trlv_text(header, "Temperature", TRLV_TEXT_TITLE_MEDIUM);


trlv_text(header, "37.2 °C", TRLV_TEXT_HEADLINE_SMALL);



lv_obj_t * btn = lv_button_create(card);
trlv_button_variant(btn, TRLV_BUTTON_TONAL);




trlv_text(btn, "Calibrate", TRLV_TEXT_LABEL_LARGE);



```

</td>
<td valign="top">

```c
lv_obj_t * card = lv_obj_create(parent);
lv_obj_remove_style_all(card);   /* theme pad, border */
lv_obj_set_scrollable(card, false);
lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_row(card, SPACE_SM, 0);   /* gap */
lv_obj_add_style(card, &style_card, 0);
lv_obj_set_style_pad_all(card, SPACE_LG, 0);
lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);

lv_obj_t * header = lv_obj_create(card);
lv_obj_remove_style_all(header);
lv_obj_set_scrollable(header, false);
lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
lv_obj_set_style_pad_column(header, SPACE_SM, 0);   /* gap */
lv_obj_set_size(header, LV_PCT(100), LV_SIZE_CONTENT);
lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

lv_obj_t * title = lv_label_create(header);
lv_label_set_text(title, "Temperature");
lv_obj_add_style(title, &style_title_medium, 0);
lv_obj_t * value = lv_label_create(header);
lv_label_set_text(value, "37.2 °C");
lv_obj_add_style(value, &style_headline_small, 0);

lv_obj_t * btn = lv_button_create(card);
lv_obj_add_style(btn, &style_button, 0);
lv_obj_add_style(btn, &style_tonal, 0);
lv_obj_add_style(btn, &style_tonal_pressed, LV_STATE_PRESSED);
lv_obj_add_style(btn, &style_tonal_focus, LV_STATE_FOCUS_KEY);
lv_obj_add_style(btn, &style_tonal_disabled, LV_STATE_DISABLED);
lv_obj_t * label = lv_label_create(btn);
lv_label_set_text(label, "Calibrate");
lv_obj_add_style(label, &style_label_large, 0);
lv_obj_center(label);
```

</td>
</tr>
</table>

**13 lines instead of 32.** TreLVis also handles what the plain version
repeats on every screen: choosing a percentage or flex grow for fill and
`pad_row` or `pad_column` for the gap (both depend on the parent's
direction), and a style with its states for every other card, surface,
text role and button variant. `trlv_theme_set_color_mode(&tokens_dark)`
recolours everything, and Compact density is just another token set for
`trlv_init`.

## Design principles

- **Small on purpose.** The main job is to turn the blocks of LVGL calls that
  every project repeats into one standard call. A function exists only if it
  does that, takes a design token, fixes an LVGL default that surprises, or
  makes a decision LVGL leaves to the caller. Anything that would only rename
  a single clear LVGL call is left to LVGL.
- **No new surprises for LVGL users.** Where TreLVis has no reason to differ,
  LVGL's natural behaviour stands. Deliberate differences are documented.
- **Familiar vocabulary.** Names come from Figma / MD3 first, then CSS, then
  LVGL. No fourth vocabulary.
- **LVGL's engine, not a parallel one.** Layout, colour, type and states are
  all LVGL style properties on shared styles, with LVGL's part/state selectors
  and inheritance.
- **Small and predictable on microcontrollers.** C99, no direct heap
  allocation, constant styles in flash, token styles in fixed static memory, and
  bounded LVGL allocations (theme objects) only at initialisation and on
  colour-mode switches.
- **Forward-compatible.** Written against LVGL 9.6 following the rules that
  make the move to LVGL v10 straightforward.

## Compatibility

| TreLVis | LVGL | C | Notes |
|---|---|---|---|
| 0.x (in development) | 9.6 (any 9.x ≥ 9.6) | C99; headers usable from C++17 | Also built weekly against LVGL master (v10 development) as an early warning. |

A compile-time check rejects unsupported LVGL versions.

TreLVis has no OS, HAL or platform dependencies. It targets both Linux
(including Raspberry Pi) and microcontrollers such as STM32, from C or C++
applications.

### What a version number promises

TreLVis follows [Semantic Versioning](https://semver.org). Until 1.0.0, a
minor release may break anything below, and the [changelog](CHANGELOG.md)
says what. From 1.0.0 on, these rules hold.

TreLVis is built from source with your project, so the promise is about
source compatibility: code that builds and works with 1.x builds and works
with any later 1.y. Binary compatibility is not promised; rebuild TreLVis
when you update it.

**Covered.** A change that breaks any of these needs a new major version:

- The public headers in `include/trelvis/`: function names and parameters,
  types, struct fields, enum names and macros. Numeric enum values are
  covered only where a header documents them (such as `trlv_justify_t`
  matching `LV_FLEX_ALIGN_*`).
- What each function documents it does, including the deliberate
  differences from LVGL. Making a function match its documentation is a
  bug fix.
- The configuration options in
  [`trlv_conf_default.h`](include/trelvis/trlv_conf_default.h): their names
  and defaults.
- The shapes of the token structs (`trlv_color_tokens_t`,
  `trlv_layout_tokens_t` and the structs inside them), which your copy of
  the token template fills in. See the token rules below.
- The names of the public styles `trlv_style_box`, `trlv_style_row` and
  `trlv_style_column`, and what each is documented to do. The properties
  inside them may change.
- The supported LVGL versions, C99 for the sources and C++17 for the
  headers.

**Not covered:** anything under `src/` (including `trlv_internal.h`), the
demo in `examples/`, the tests, and the values in `templates/`. Your copy
of a template is yours; a later template is only a suggestion.

**Token schema.** Your project owns a copy of the token template, so the
token structs are filled in by code TreLVis doesn't control.

- Adding a token field (or a step to a token scale) is a minor change,
  provided an existing copy still builds and still works. That holds
  because the template uses designated initialisers (`.primary = …`,
  `[TRLV_SPACE_SM] = 8`), so a field your copy doesn't set is zero, and
  the library treats zero in a new field safely: it falls back to a
  documented value and logs a warning. Keep designated initialisers when
  you edit your copy.
- Renaming or removing a token field, or changing what an existing field
  means, is a major change.
- Every token schema change has a **Migration** note in its changelog
  entry, saying what to add to or change in a copied template.

**Visual output.** The theme's look (which colour role a widget uses, a
padding, a state-layer opacity, a shape) can change in a minor release,
listed under *Changed* in the changelog so you can check your screens. A
patch release changes the look only to fix a bug: output that doesn't
match the documentation or the MD3 role it names.

**Deprecation.** Anything covered is deprecated in a minor release first: a
`@deprecated` note in its doc comment naming the replacement, and an entry
under *Deprecated* in the changelog. It keeps working until the next major
release removes it.

**LVGL versions.** TreLVis 1.x supports LVGL 9 from 9.6 on. Support for a
newer LVGL 9 minor arrives in a minor or patch release once CI builds and
tests it; the minimum LVGL version rises only in a major release. TreLVis
already follows the rules that ease the move to LVGL v10 and builds weekly
against LVGL master ([#88](https://github.com/tmitchell-dev/trelvis/issues/88)).
When LVGL v10 is released, support for it comes in a new TreLVis major
version, and a `release/v1.x` branch takes fixes for products that stay on
LVGL 9.

## Quick start

From an LVGL 9.6 project with a working display to a themed screen in
five steps. Your project provides LVGL and its `lv_conf.h`; TreLVis
doesn't vendor it. CI builds [`tests/consumer`](tests/consumer), a small
project that follows these steps with CMake and with a Makefile.

### 1. Add TreLVis

```sh
git submodule add https://github.com/tmitchell-dev/trelvis.git external/trelvis
git -C external/trelvis checkout v0.1.0   # pin a release
```

### 2. Build it with your project

**CMake:**

```cmake
add_subdirectory(external/lvgl)
add_subdirectory(external/trelvis)
target_link_libraries(my_app PRIVATE trelvis)
```

TreLVis links against your project's `lvgl` target. If your LVGL target has
another name, set it first: `set(TRLV_LVGL_TARGET my_lvgl)` before
`add_subdirectory(external/trelvis)`.

TreLVis finds a `trlv_conf.h` (see [Configuration](#configuration)) on its
include path, which includes your LVGL target's public include
directories. LVGL's own CMake keeps the directory of `lv_conf.h` to itself,
so if you configure TreLVis, add that directory to TreLVis too:

```cmake
target_include_directories(trelvis PUBLIC config)   # holds lv_conf.h and trlv_conf.h
```

**Makefile:** `include external/trelvis/trelvis.mk` adds the sources to
`CSRCS` and the include path to `CFLAGS`, next to LVGL's own `lvgl.mk`.
`lvgl.mk` puts only LVGL's own directory on the include path, so also add
`-Iexternal/lvgl/include` (or the directory that holds `lvgl/`) for the
`<lvgl/lvgl.h>` that TreLVis includes, and the directory of your
`lv_conf.h` and `trlv_conf.h`.

**Anything else** (STM32CubeIDE and other IDE-managed builds): add
`external/trelvis/src/*.c` to your sources and `external/trelvis/include`
to your include path. TreLVis needs no generated files.

TreLVis includes LVGL as `<lvgl/lvgl.h>`. LVGL 9.6's `include/` directory
provides that path; if your build lays LVGL out differently, set
`TRLV_LVGL_INCLUDE` (see [Configuration](#configuration)).

### 3. Copy the token template

TreLVis compiles in no colours, spacing or fonts: your project owns them,
starting from a template, the way LVGL's `lv_conf_template.h` works.

```sh
cp external/trelvis/templates/trlv_tokens_template.c src/tokens.c
cp external/trelvis/templates/trlv_tokens_template.h src/tokens.h
```

In `tokens.c`, change `#include "trlv_tokens_template.h"` to
`#include "tokens.h"`, and add `tokens.c` to your sources. The template
works unedited: the MD3 baseline colours in light and dark, and the Regular
and Compact layouts. Later you replace the colours with your product's
(from Material Theme Builder), give the text roles real fonts, and delete
what you don't use; see [Design tokens](docs/concepts.md#4-design-tokens).

### 4. Call `trlv_init`

Once, after creating the display and before building screens:

```c
#include <trelvis/trelvis.h>
#include "tokens.h"

trlv_init(disp, &tokens_light, &tokens_regular);
```

This installs the MD3 theme. The screen is now `surface` with `on_surface`
text, and every LVGL widget you create gets its MD3 look.

### 5. Build a screen

```c
lv_obj_t * page = trlv_column(lv_display_get_screen_active(disp), TRLV_SPACE_LG);
trlv_pad(page, TRLV_SPACE_XL);
trlv_fill_width(page);
trlv_fill_height(page);

trlv_text(page, "Hello, TreLVis", TRLV_TEXT_HEADLINE_SMALL);

lv_obj_t * card = trlv_column(page, TRLV_SPACE_SM);
trlv_card(card, TRLV_CARD_OUTLINED);
trlv_pad(card, TRLV_SPACE_LG);
trlv_fill_width(card);
trlv_text(card, "Status", TRLV_TEXT_TITLE_MEDIUM);
lv_obj_t * body = trlv_text(card, "All systems normal.", TRLV_TEXT_BODY_MEDIUM);
trlv_fill_width(body);                                  /* a width makes text wrap */

lv_obj_t * btn = lv_button_create(page);
trlv_text(btn, "Continue", TRLV_TEXT_LABEL_LARGE);
```

Switch to dark at any time with `trlv_theme_set_color_mode(&tokens_dark)`.
Rows, columns and the other primitives are described in
[Concepts](docs/concepts.md); the [demo](examples/demo) shows every one of
them and builds in your project the same way.

## Configuration

All options have defaults in
[`trlv_conf_default.h`](include/trelvis/trlv_conf_default.h). To change one,
create a `trlv_conf.h` next to your `lv_conf.h` (anywhere on the include path
works, as long as TreLVis's sources see it too; see
[step 2](#2-build-it-with-your-project)), or pass a compiler definition. A
different file name or location can be given with `TRLV_CONF_PATH`, defined
for TreLVis and your code alike:

```cmake
target_compile_definitions(trelvis PUBLIC
    "TRLV_CONF_PATH=\"${CMAKE_SOURCE_DIR}/config/my_trlv_conf.h\"")
```

```c
/* trlv_conf.h */
#define TRLV_USE_THEME          1
#define TRLV_USE_DEBUG_OUTLINE  0
```

| Option | Default | Meaning |
|---|---|---|
| `TRLV_LVGL_INCLUDE` | `<lvgl/lvgl.h>` | How TreLVis includes LVGL. |
| `TRLV_USE_THEME` | `1` | Install the MD3 theme. `0` = tokens and layout helpers only (bring your own theme). |
| `TRLV_THEME_PARENT` | `TRLV_THEME_PARENT_DEFAULT` | Theme for widgets TreLVis doesn't cover: LVGL's default theme seeded with MD3 colours, or `TRLV_THEME_PARENT_NONE`. |
| `TRLV_CUSTOM_COLOR_MAX` | `4` | How many of the project's custom colours `trlv_surface_custom` can apply (two small shared styles each). |
| `TRLV_USE_DEBUG_OUTLINE` | `0` | Draw a 1 px outline on every TreLVis container. |
| `TRLV_IGNORE_LVGL_VERSION` | `0` | Skip the LVGL version check (unsupported; for trying LVGL master). |

## Threading

Like LVGL itself, every TreLVis function must be called from the LVGL thread
(the one that runs `lv_timer_handler()`).

## Documentation

- [Concepts](docs/concepts.md): vocabulary, design tokens, fonts, the theme and
  style engine, primitives and modifiers, and the Figma / MD3 / CSS / LVGL
  mapping.
- [Figma hand-off](docs/figma-handoff.md): conventions that make a Figma design
  translate directly into TreLVis code.
- [Roadmap](ROADMAP.md): milestones, scope and what is deliberately left out.
- [Changelog](CHANGELOG.md)

## Contributing

Contributions are welcome. Please read [CONTRIBUTING.md](CONTRIBUTING.md) first.
It covers the workflow, coding rules and project scope. Report security
issues as described in [SECURITY.md](SECURITY.md).

## Licence

TreLVis is released under the [MIT licence](LICENSE). Third-party material is
listed in [NOTICE](NOTICE).

TreLVis is an independent project. It is not affiliated with or endorsed by
LVGL, Google (Material Design) or Figma. Material Design is a design system
by Google; TreLVis implements its publicly documented token structure.
