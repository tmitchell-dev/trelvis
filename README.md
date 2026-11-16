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

> **Status: early development (0.x).** The repository scaffold is in place and
> the public API is being designed. Nothing is usable yet, and the API may
> change in any 0.x release. See [ROADMAP.md](ROADMAP.md).

## A taste of the API

This shows the intended API, which is still under review:

```c
#include <trelvis/trelvis.h>

trlv_init(disp, &tokens_light, &tokens_regular);  /* once; tokens come from your copy of the template */

lv_obj_t * card = trlv_column(parent, TRLV_SPACE_SM);   /* Figma: auto layout, vertical */
trlv_card(card, TRLV_CARD_FILLED);                      /* MD3 filled card               */
trlv_pad(card, TRLV_SPACE_LG);
trlv_fill_width(card);                                  /* Figma: fill container         */

lv_obj_t * header = trlv_row(card, TRLV_SPACE_SM);
trlv_justify(header, TRLV_JUSTIFY_SPACE_BETWEEN);       /* CSS: justify-content          */
trlv_align_items(header, TRLV_ALIGN_CENTER);            /* CSS: align-items              */
trlv_text(header, "Temperature", TRLV_TEXT_TITLE_MEDIUM);
trlv_text(header, "37.2 °C",     TRLV_TEXT_DISPLAY_SMALL);

lv_obj_t * btn = lv_button_create(card);                /* plain LVGL; MD3 filled button via the theme */
trlv_button_variant(btn, TRLV_BUTTON_TONAL);
```

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

## Getting started

### CMake (git submodule)

```sh
git submodule add https://github.com/tmitchell-dev/trelvis.git external/trelvis
```

```cmake
add_subdirectory(external/lvgl)      # your project provides LVGL; TreLVis doesn't vendor it
add_subdirectory(external/trelvis)
target_link_libraries(my_app PRIVATE trelvis)
```

TreLVis links against your project's `lvgl` target. If your LVGL target has
another name, set `TRLV_LVGL_TARGET`.

### Without CMake

TreLVis needs no generated files. Add `trelvis/src/*.c` to your sources and
`trelvis/include` to your include path, next to LVGL. This also works for
IDE-managed builds such as STM32CubeIDE. Plain Makefiles can use
`include trelvis/trelvis.mk`, which adds both to `CSRCS` and `CFLAGS`.

TreLVis includes LVGL as `<lvgl/lvgl.h>`. LVGL 9.6's `include/` directory
provides that path; if your build lays LVGL out differently, set
`TRLV_LVGL_INCLUDE` (see below).

### Configuration

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
| `TRLV_USE_DEBUG_OUTLINE` | `0` | Draw a 1 px outline on every TreLVis container. |
| `TRLV_IGNORE_LVGL_VERSION` | `0` | Skip the LVGL version check (unsupported; for trying LVGL master). |

### Threading

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
