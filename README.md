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
> a real product. See [ROADMAP.md](ROADMAP.md).

## A taste of the API

```c
#include <trelvis/trelvis.h>

trlv_init(disp, &tokens_light, &tokens_regular);  /* once; tokens come from your copy of the template */

lv_obj_t * card = trlv_column(parent, TRLV_SPACE_SM);   /* Figma: auto layout, vertical */
trlv_card(card, TRLV_CARD_FILLED);                      /* MD3 filled card               */
trlv_pad(card, TRLV_SPACE_LG);
trlv_fill_width(card);                                  /* Figma: fill container         */

lv_obj_t * header = trlv_row(card, TRLV_SPACE_SM);
trlv_fill_width(header);
trlv_justify(header, TRLV_JUSTIFY_SPACE_BETWEEN);       /* CSS: justify-content          */
trlv_align_items(header, TRLV_ALIGN_ITEMS_CENTER);      /* CSS: align-items              */
trlv_text(header, "Temperature", TRLV_TEXT_TITLE_MEDIUM);
trlv_text(header, "37.2 °C",     TRLV_TEXT_HEADLINE_SMALL);

lv_obj_t * btn = lv_button_create(card);                /* plain LVGL; MD3 filled button via the theme */
trlv_button_variant(btn, TRLV_BUTTON_TONAL);
trlv_text(btn, "Calibrate", TRLV_TEXT_LABEL_LARGE);
```

The text in the card takes the card's `on_` colour by inheritance, and
`trlv_theme_set_color_mode(&tokens_dark)` recolours all of it, the button
included, with no per-screen code.

### The same card in plain LVGL

Here is that card written with LVGL 9.6 calls only, under LVGL's default
theme, the way a careful LVGL user would: shared styles for colours and
fonts (so light/dark can re-set them), local styles for layout, project
constants instead of magic numbers. A
[headless test](tests/layout/test_readme_card.c) builds both versions and
checks that every object lands at the same position and size and gets
the same fonts and colours, in light and in dark.

<!-- The per-card block below is copied from tests/layout/test_readme_card.c,
     and the set-up summary and line counts describe the rest of that file's
     plain-LVGL part. Change them together. -->

**Once per project**, the plain version needs what the token template and
`trlv_init` give TreLVis (75 lines in the test):

- `#define`s for the spacing, radius, button height and focus width it uses;
- a palette struct with the card's colours, filled in for light and for dark;
- shared styles: the card (background, radius), three text roles (font and
  line height converted to LVGL's line spacing), the button's MD3 shape
  over the default theme's (height, pill radius, padding, no shadow), and
  the tonal variant in four states (default, pressed and focus state layers
  that also cancel the theme's recolour and grow, disabled opacities);
- `styles_set_colors(palette)`, which writes every colour into those styles,
  called at start-up and on each light/dark switch before
  `lv_obj_report_style_change(NULL)`.

**Per card**, using those styles:

```c
lv_obj_t * card = lv_obj_create(parent);
lv_obj_remove_style_all(card);                      /* no theme padding, border, scrollbar */
lv_obj_add_style(card, &style_card, 0);
lv_obj_set_scrollable(card, false);
lv_obj_set_size(card, LV_PCT(100), LV_SIZE_CONTENT);  /* in a row: lv_obj_set_flex_grow */
lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
lv_obj_set_style_pad_all(card, SPACE_LG, 0);
lv_obj_set_style_pad_row(card, SPACE_SM, 0);        /* a column's gap is pad_row */

lv_obj_t * header = lv_obj_create(card);
lv_obj_remove_style_all(header);
lv_obj_set_scrollable(header, false);
lv_obj_set_size(header, LV_PCT(100), LV_SIZE_CONTENT);
lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
lv_obj_set_flex_align(header, LV_FLEX_ALIGN_SPACE_BETWEEN,
                      LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);   /* track too */
lv_obj_set_style_pad_column(header, SPACE_SM, 0);   /* a row's gap is pad_column */

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

**32 lines per card instead of 13** (non-blank lines).

The plain version also has to repeat, on every screen:

- **Which axis**: `LV_PCT(100)` or `lv_obj_set_flex_grow` depending on
  the parent's direction, and `pad_row` or `pad_column` for the gap.
  `trlv_fill_width` and `trlv_gap` read the parent and decide.
- **Every other card, button variant, surface and text role** as more
  styles, each with its pressed, focus and disabled states.
- **Dark mode**: every colour style must be listed in `styles_set_colors`
  (and LVGL's default theme re-initialised for its own widgets).
  `trlv_theme_set_color_mode` covers the whole library and theme.
- **Compact density**: the `#define`s are fixed at compile time. Choosing
  a density at start-up means turning them into variables and
  rebuilding every style from them. TreLVis takes a layout token set
  in `trlv_init`.

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

A compile-time check rejects unsupported LVGL versions. A move to LVGL v10 that
changes the TreLVis API will be a new TreLVis major version.

TreLVis has no OS, HAL or platform dependencies. It targets both Linux
(including Raspberry Pi) and microcontrollers such as STM32, from C or C++
applications.

## Quick start

From an LVGL 9.6 project with a working display to a themed screen in
five steps. Your project provides LVGL and its `lv_conf.h`; TreLVis
doesn't vendor it.

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
another name, set `TRLV_LVGL_TARGET` before `add_subdirectory`.

**Makefile:** `include external/trelvis/trelvis.mk` adds the sources to
`CSRCS` and the include path to `CFLAGS`, next to LVGL's own `lvgl.mk`.
`lvgl.mk` puts only LVGL's own directory on the include path, so also add
`-Iexternal/lvgl/include` (or the directory that holds `lvgl/`) for the
`<lvgl/lvgl.h>` that TreLVis includes.

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
works), or pass a compiler definition. A different file name or location can
be given with `TRLV_CONF_PATH`.

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
