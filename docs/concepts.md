# TreLVis concepts

This document explains how TreLVis is designed: its vocabulary, design tokens,
fonts, how it uses LVGL's theme and style engine, and its functions, by
kind (§7).

> **Status:** this describes TreLVis v1, which is fully implemented. The
> public headers are the source of truth for exact signatures; this document
> explains the reasons behind them. Items marked *(planned, v1.x)* are not
> built yet (see [ROADMAP.md](../ROADMAP.md)).

- [1. What TreLVis is](#1-what-trelvis-is)
- [2. Vocabulary](#2-vocabulary)
- [3. Naming](#3-naming)
- [4. Design tokens](#4-design-tokens)
- [5. Fonts](#5-fonts)
- [6. Theme and style engine](#6-theme-and-style-engine)
- [7. The API by kind](#7-the-api-by-kind)
- [8. Memory and threading](#8-memory-and-threading)
- [9. LVGL behaviours TreLVis handles](#9-lvgl-behaviours-trelvis-handles)
- [10. Mapping: Figma / MD3 → CSS → TreLVis → LVGL](#10-mapping-figma--md3--css--trelvis--lvgl)

---

## 1. What TreLVis is

TreLVis is a **utility layer, not a framework**: a small toolset for starting
LVGL projects quickly and keeping them consistent.

- Everything returns a plain `lv_obj_t *`. Raw `lv_*` calls remain valid
  everywhere; raw LVGL is the escape hatch.
- It does **not** wrap LVGL widgets. It styles them through LVGL's theme
  engine. `lv_button_create()` gives you an MD3 filled button.
- It does not wrap LVGL calls that are already clear (`lv_obj_set_size`,
  `lv_obj_center`, `lv_obj_set_hidden`, `lv_obj_set_scrollable`). The
  [mapping table](#10-mapping-figma--md3--css--trelvis--lvgl) names the LVGL
  call for each Figma or CSS concept TreLVis leaves alone.
- It owns no project content: fonts, images, screens, components and the
  project's token **values** live in your project. Token values start from
  a template you copy at setup; none are compiled into the library.
- It adds **no new surprises for LVGL users**. Where TreLVis has no reason to
  differ, LVGL's natural behaviour stands, and every deliberate difference
  (such as `trlv_box_create` not scrolling) is documented on the function.

### What earns a place

TreLVis stays small on purpose. Its main job is to turn the blocks of LVGL
calls that every project repeats into one standard call. A public function
exists only if it does at least one of these:

- **Replaces a block of LVGL calls that projects repeat**, so it is written
  once and the same way everywhere (`trlv_row_create(parent)`: create the
  object, reset its theme look, padding and size, turn off scrolling and
  clicks, set the flow).
- **Takes a design token**, so screens hold no magic numbers
  (`trlv_set_pad(obj, TRLV_SPACE_LG)`).
- **Fixes an LVGL default that surprises** people coming from Figma or CSS
  (`trlv_box_create`: not scrollable, not clickable, no theme padding).
- **Makes a decision LVGL leaves to the caller** (`trlv_set_width_fill`: grow or
  100 %, depending on the parent's direction).

A function that only renames a single clear LVGL call does not qualify.

## 2. Vocabulary

| Term | Meaning | Examples |
|---|---|---|
| **Primitive** | A TreLVis building block (this library): an object made by a TreLVis constructor (§7.2). | box, row, column, spacer, divider, text |
| **Component** | A project-level composition with no state of its own, created by `<name>_create(parent, props…)`. The default choice. | `stat_card` |
| **Widget** | Something with its own C state or logic: LVGL's built-in widgets, plus a project's custom widgets (`wd_<name>`). | `lv_slider`, `wd_readout` |

How this maps to other frameworks:

| Elsewhere | TreLVis term |
|---|---|
| Figma component, React component, QML `Component`, Flutter `StatelessWidget` | **component** |
| Flutter `StatefulWidget`, custom `QWidget`, LVGL widget | **widget** |

TreLVis ships **primitives only**, plus theming for LVGL's built-in widgets.
Components and widgets belong in projects, typically written as
`lv_obj_t * <name>_create(lv_obj_t * parent, …)` functions that use TreLVis
primitives inside.

## 3. Naming

- **Vocabulary priority:** Figma / MD3 term → CSS term → LVGL term. TreLVis
  never invents a fourth vocabulary.
- **Avoided words**, because they already mean something else:

  | Word | Clashes with |
  |---|---|
  | `frame` | framebuffers |
  | `float` | CSS `float` |
  | `stack` | means different things in different frameworks |
  | `layer` | LVGL draw layers and `lv_layer_top()`. MD3 "state layer" appears only in docs. |
  | `canvas` | `lv_canvas` |
  | `screen` | LVGL screens |
  | `align` for flex distribution | `lv_obj_align()` means positioning |

- **Words from Figma / CSS / MD3, grammar from LVGL.** The verb at the call
  site says what a call does, as in LVGL:
  - **Constructors** end in `_create`, like `lv_button_create`, and return
    the new object: `trlv_row_create(parent)`. They take the parent, plus
    only arguments whose type makes them clear at the call site
    (`trlv_text_create(parent, "Start", TRLV_TEXT_LABEL_LARGE)`). A bare
    `TRLV_SPACE_*` doesn't pass that test, so gap and padding are never
    constructor arguments.
  - **Layout setters** are `trlv_set_<property>`: the properties of
    Figma's auto layout panel (`trlv_set_gap`, `trlv_set_justify`,
    `trlv_set_width_fill`). A setter never changes colour.
  - **Appearance functions** are `trlv_apply_<look>`: Figma's "apply a
    style", for colour, shape and type (`trlv_apply_surface`,
    `trlv_apply_card`, `trlv_apply_text_role`, `trlv_apply_text_emphasis`,
    `trlv_apply_button_type`).
    An apply never changes layout, so a card's padding is a separate
    `trlv_set_pad`.
  - **Getters** use `_get_`: `trlv_get_color_tokens()`.
  - **Setup** functions keep LVGL's module grammar: `trlv_init`, like
    `lv_init`, and `trlv_theme_set_color_mode`, like `lv_theme_*`.

  §7 describes each kind, its rules and how a screen uses them. Setters
  and applies (together, the *modifiers*) have no `obj` in their names,
  unlike LVGL's `lv_obj_set_*`: they work on any object, so there is no
  class to name.

  The same grammar is what §2 asks projects to use for their
  components (`readout_create(parent, …)`), so library and project calls
  read alike. Names that state their action also help code generators and
  LLMs porting from a Figma file, which otherwise mistake a constructor for
  a modifier.
- **One property per line.** Create, then set: each setter line is one row
  of Figma's auto layout panel, for every construct, with no per-construct
  exceptions to learn:

  ```c
  lv_obj_t * page = trlv_column_create(screen);
  trlv_set_gap(page, TRLV_SPACE_LG);
  trlv_set_pad(page, TRLV_SPACE_LG);
  trlv_set_width_fill(page);
  ```
- **No abbreviations** except LVGL's `pad`: `trlv_column_create`,
  `trlv_set_width_fill`, `trlv_set_align_items`.
- **Fully qualified enums:** `TRLV_JUSTIFY_SPACE_BETWEEN`, `TRLV_ALIGN_ITEMS_CENTER`,
  `TRLV_SPACE_MD`, `TRLV_TEXT_BODY_MEDIUM`, `TRLV_SURFACE_CONTAINER_HIGH`.
- **Prefixes:** functions `trlv_*`, types `trlv_*_t`, macros and enums
  `TRLV_*`, config `TRLV_USE_*` / `TRLV_*`, styles `trlv_style_*`.

**Token names are MD3 role names in snake_case:**

| MD3 / Figma | C |
|---|---|
| Surface Container High | `trlv_get_color_tokens()->surface_container_high` |
| On Primary Container | `trlv_get_color_tokens()->on_primary_container` |
| Spacing `space/md` | `TRLV_SPACE_MD` |
| Shape corner Medium | `TRLV_RADIUS_MD` |
| Type scale Body Medium | `TRLV_TEXT_BODY_MEDIUM` |

## 4. Design tokens

TreLVis follows **Material Design 3 (MD3)**. The library defines the token
**types**; each project supplies the **values**. Token collections mirror
Figma variable collections:

| Collection | Modes | Selected | Contents |
|---|---|---|---|
| Primitives | — | n/a | Tonal palettes (e.g. `PAL_PRIMARY_40`). Project-side `#define`s only. |
| Color | light, dark, brand… | **at runtime** | MD3 colour roles, the project's custom colours, state-layer opacities |
| Layout | compact, regular, large… | **at start-up** | Spacing, shape, sizes, elevation geometry, type scale |

### 4.1 Token types

The types are defined in [`trlv_tokens.h`](../include/trelvis/trlv_tokens.h),
which documents every field and maps each token to its Figma variable or MD3
token name. In outline:

| Type | What it holds |
|---|---|
| `trlv_space_t`, `trlv_radius_t`, `trlv_size_t`, `trlv_elevation_t`, `trlv_text_role_t` | The scales: enums that index the layout arrays (`TRLV_SPACE_MD`, `TRLV_RADIUS_FULL`, `TRLV_TEXT_BODY_MEDIUM`, …) |
| `trlv_text_style_t` | One type-scale role: font, Figma line height, letter spacing |
| `trlv_shadow_t` | One elevation level as an LVGL shadow (colour from the `shadow` role) |
| `trlv_state_tokens_t` | State-layer and disabled opacities |
| `trlv_custom_color_t` | One project-defined custom colour: colour, on colour, container, on container |
| `trlv_color_tokens_t` | Color collection: the MD3 colour roles, `focus`, the state opacities, and the project's custom colour table |
| `trlv_layout_tokens_t` | Layout collection: spacing, shape, stroke and focus widths, control heights, elevation, type scale |

The structs hold the standard MD3 roles and scales, plus what v1 reads.
Tokens for later features (icon sizes, hover, motion) are added together
with the feature that uses them.

**One shape.** No configuration option adds or removes token fields, so a
token file, a generator and the docs are the same for every project. What
differs between products is in the custom colour table.

**Custom colours.** MD3 has one status role, `error`. Anything more is
product-specific, so the colour tokens carry a table the project defines
(`custom`, `custom_count`), named and indexed by the project's own enum.
Each entry has the four values Material Theme Builder exports for a custom
colour. Typical tables:

| Product | Entries |
|---|---|
| Consumer / brand | success, warning, an extra brand accent, or none |
| Process HMI (ISA-101, ISA-18.2) | one per alarm priority in the site's alarm philosophy; optionally non-alarm status colours |
| Medical device (IEC 60601-1-8) | high, medium and low priority alarms |

`trlv_apply_surface_custom(obj, index, surface)` fills an object with an entry's
colour (`TRLV_CUSTOM_SURFACE_COLOR`, a strong fill such as an alarm banner)
or its container (`TRLV_CUSTOM_SURFACE_CONTAINER`), with the content colour
inherited and following colour-mode switches.
`TRLV_CUSTOM_COLOR_MAX` (default 4) sets how many entries it can apply; it
sizes the library's shared styles and nothing else. TreLVis gives the
entries no meaning and draws no alarm behaviour (flashing, acknowledgement):
that belongs to the product.

The tokens are hand-written C for now. The structs are designed so that a
future generator (DTCG JSON or Material Theme Builder JSON) can emit them
unchanged.

### 4.2 Token template and reference values

The library compiles in no token values, so unused values cost no flash.
Tokens come from a template, the same way `lv_conf_template.h` works in LVGL.
At setup, copy [`templates/trlv_tokens_template.c`](../templates/trlv_tokens_template.c)
and `.h` into your project, rename them (for example to `tokens.c` and
`tokens.h`), fix the `#include` in the `.c` file, and build them with your
sources:

```c
#include "tokens.h"                              /* your copy of the template */

trlv_init(disp, &tokens_light, &tokens_regular);
```

The template defines four `const` collections, `tokens_light`, `tokens_dark`,
`tokens_regular` and `tokens_compact`, and works unedited:

- **Colour:** the MD3 baseline light and dark schemes (seed `#6750A4`) as
  published by Material Design. The custom colour table holds four example
  status colours: `TOKEN_CUSTOM_FAULT`, `_WARNING`, `_ADVISORY` and
  `_BAD_DATA`. Their strong colours are the same in both modes and their
  containers follow the mode, and none of them is `error`, `primary` or
  `focus`. They are examples that claim compliance with no standard: a
  project renames, replaces or deletes them.
- **Layout:** the Regular and Compact values below. In Regular, each text
  role uses the Montserrat size LVGL ships nearest to its MD3 size, with the
  MD3 line height, when that size is enabled in `lv_conf.h`
  (`LV_FONT_MONTSERRAT_<size>`). Compact steps the display and headline
  roles down one size and tightens every line height. A size
  that isn't enabled falls back to the next smaller enabled size with its
  own line height, and with none enabled to `LV_FONT_DEFAULT`. Nothing
  needs a font file, and the type hierarchy shows as soon as a few sizes
  are enabled.

The template is compiled in CI with the strict warning flags, so it stays in
step with the token types.

The project then owns the file: regenerate the colours from the product's
seed colour, give each role a real font, and delete what it doesn't use,
such as the layout mode it doesn't ship. The demo's tests run in both
layout modes, and the focus ring fits the `space[SM]` padding in both.

**Line height.** Figma gives every text style a line height in px (Body
Medium is 14 px text on a 20 px line). Set `line_height` to that number and
TreLVis converts it to LVGL's line spacing. Set it to `0` when there is no
design value yet: the text then uses the font's own line height, exactly as a
plain LVGL label does. The template uses `0` for a role whose Montserrat size
isn't enabled, because a 20 px line only makes sense with the role's real
font.

LVGL adds line spacing *between* lines only, while Figma centres each line
in its line height. So a text box is shorter than in Figma by
`line_height − font line height` (a one-line Body Medium text is about 16 px
tall rather than 20 px), and its lines sit half that difference higher. The
spacing between lines matches.

**Spacing:** a 4 px grid that is MD3-compatible, with a 2 px half-step for
hairlines.

| Token | NONE | 2XS | XS | SM | MD | LG | XL | 2XL | 3XL |
|---|---|---|---|---|---|---|---|---|---|
| Regular | 0 | 2 | 4 | 8 | 12 | 16 | 24 | 32 | 48 |
| Compact | 0 | 2 | 2 | 4 | 8 | 12 | 16 | 24 | 32 |

Typical use: card padding `LG` (16), gaps `SM`–`MD`, section spacing `XL`–`2XL`.

**Shape (MD3 corner scale) and sizes:**

| Token | Regular | Compact |
|---|---|---|
| radius XS / SM / MD / LG / XL | 4 / 8 / 12 / 16 / 28 | 2 / 4 / 8 / 12 / 20 |
| control_height SM / MD / LG | 32 / 40 / 56 (touch target ≥ 44–48 incl. padding) | 28 / 36 / 44 |
| border / focus width | 1 / 3 | 1 / 2 |
| elevation | levels 0–5 approximating MD3 shadows | all zero (tone only) |

**Type scale:** a project's Regular tokens normally follow the MD3 type scale
(Display Large 57/64 … Label Small 11/16), and Compact scales it down. The 15
roles map onto a reduced set of about 5–6 real fonts (see [Fonts](#5-fonts)).
The token template uses LVGL's Montserrat sizes, which have one weight only.

**State layers:** focus 10%, pressed 10%, disabled content 38%, disabled
container 12%. These are the MD3 values; projects may tune them.

### 4.3 Rules

- Screens and components use **semantic roles only**, never raw hex values.
- Always pair a container role with its `on_` role. The theme,
  `trlv_apply_surface` and `trlv_apply_card` do this automatically through inheritance.
- Elevation is shown mainly by **tone** (surface container levels). Shadows
  are optional, and zero in compact / MCU modes.

## 5. Fonts

- **Roles are names, not fonts.** Each of the 15 MD3 type roles points at an
  `lv_font_t *`. It is normal for several roles to share one font. Only fonts
  referenced by the tokens a product compiles in get linked.
- **Font-source agnostic.** TreLVis works with both:
  - **compiled bitmap fonts**: the MCU default. Each family + weight + size
    combination is a separate C file in flash.
  - **runtime-rendered TTF** (LVGL's FreeType or Tiny TTF engines): suits
    Linux / Raspberry Pi. One TTF serves every size, at the cost of RAM for a
    glyph cache and some CPU.

  TreLVis uses only the public font API (e.g. `lv_font_get_line_height()`),
  never glyph data or font internals.
- **Runtime fonts and `const` tokens.** Runtime fonts only exist after
  start-up, so projects that use them fill the layout token struct in RAM
  before `trlv_init`. The API takes a pointer. `const` / flash storage is
  recommended for compiled fonts but not required.
- **Flash cost of bitmap fonts** grows with glyph count × pixel size² × bits
  per pixel, so large Display sizes dominate. Ways to reduce it:
  - share fonts across roles (about 5–6 real fonts per layout mode);
  - restrict character sets, e.g. digits-and-units-only fonts for large readouts;
  - compile only the layout modes a product ships;
  - use 2 bpp and/or compressed bitmaps for large sizes;
  - use external memory-mapped (QSPI) flash on STM32.
- **Fallback:** LVGL's font fallback can chain a text font to an icon font, so
  icons and text can mix in one label.
- **Readouts:** use a font family with tabular (fixed-width) digits for roles
  that show changing numbers, so values don't jitter.

## 6. Theme and style engine

### 6.1 Principle: use LVGL's engine wherever possible

- **Everything visual and layout-related is a style property**, applied
  through shared styles with part/state selectors. TreLVis does no styling in
  event callbacks and uses no per-object local styles in library code.
- **Constant styles** (box reset / transparent, row and column flow, justify
  and align values, grow, fill width/height, fit) use `LV_STYLE_CONST_INIT`.
  They live in flash and need no RAM and no allocation. **Token-dependent
  styles** (gap, pad, text roles, surfaces, cards, button types, the
  divider and the theme's own styles) use the same
  `LV_STYLE_CONST_INIT` mechanism, but their property array is a static,
  writable array in RAM: `trlv_init` and colour-mode switches write the token
  values into it instead of calling `lv_style_set_*`. They need no allocation,
  their memory is fixed at compile time, and they stay valid across
  `lv_deinit` / `lv_init`.
- **Layout is style properties:** `layout`, `flex_flow`, `flex_main_place`,
  `flex_cross_place`, `flex_track_place`, `flex_grow`, `pad_row`,
  `pad_column` and `width` / `height` (`LV_PCT`, `LV_SIZE_CONTENT`).
  Primitives are just an `lv_obj` plus shared styles plus the per-flag setters
  they need (scrollable and clickable are flags, not styles).
- **Inheritance works like CSS.** LVGL text properties inherit.
  `trlv_apply_surface` and `trlv_apply_card` set the container colour **and** its `on_`
  text colour, so labels and icons inside inherit the right MD3 pairing.
  A text-role style applied to a container sets the role for all descendant
  labels. The theme styles the screen with `surface` / `on_surface` and the
  Body font.
- **States use selectors.** Pressed, focus-key, checked and disabled
  styles use `LV_STATE_*` selectors. MD3 state layers are **pre-mixed
  colours** (`lv_color_mix(on_role, role, opa)`) in state styles, computed at
  init and on each colour-mode switch. LVGL colour filters are not used for
  state layers because they also tint the content. The disabled treatment
  is real opacity (`on_surface` at 12 % container, 38 % content), as MD3
  specifies. Each interactive widget has its own pressed and focus styles
  (its colour pair); the focus ring and the disabled style are shared.
  The ring is `focus_width` thick and `space[2XS]` outside the widget, and
  only shows for keypad or encoder focus (`LV_STATE_FOCUS_KEY`).
- **Transitions** use `lv_style_transition_dsc_t` with motion tokens *(planned, v1.x)*.
- **Modifier groups replace rather than stack.** Calling `trlv_set_gap` (or
  `trlv_set_pad`, `trlv_apply_card`, `trlv_apply_surface`, `trlv_set_justify`, …) again removes
  that group's previous style before adding the new one. The target is 2–4
  TreLVis styles per object. The new style takes the old one's place in the
  object's style list (`lv_obj_replace_style`), so calling a modifier again
  doesn't change its precedence: a project style added after it still wins.

### 6.2 Style precedence

From lowest to highest priority:

1. theme styles: the parent theme's first, then the TreLVis theme's, which
   override them
2. TreLVis modifier styles
3. project styles added later with `lv_obj_add_style`
4. local style properties (`lv_obj_set_style_*`)

### 6.3 Theme API

The theme API is in [`trlv_theme.h`](../include/trelvis/trlv_theme.h):

```c
void trlv_init(lv_display_t * disp,
               const trlv_color_tokens_t * color,
               const trlv_layout_tokens_t * layout);
void trlv_theme_set_color_mode(const trlv_color_tokens_t * color);  /* run-time switch */
const trlv_color_tokens_t  * trlv_get_color_tokens(void);
const trlv_layout_tokens_t * trlv_get_layout_tokens(void);
```

- `trlv_init` takes an explicit display. LVGL v10 no longer accepts `NULL`
  for the default display.
- **Call `trlv_init` before creating objects.** LVGL themes style an object
  when it is created, so objects that already exist keep their old theme
  styles.
- **Colour-mode switch:** existing shared styles are re-set in place, the
  parent default theme is re-initialised with the new colours, then
  `lv_obj_report_style_change(NULL)` is called. TreLVis never walks the object
  tree. LVGL's default theme needs a dark flag; TreLVis decides it from the
  luminance of `surface`, so the colour tokens need no extra field.
- **Layout mode** is chosen once, at `trlv_init`: layout tokens change sizes,
  so switching them would need every screen rebuilt.
- **One set of tokens per application.** The shared styles are global; the
  theme is installed on the display passed to `trlv_init`, and another
  display can use the same theme with `lv_display_set_theme`.
- `TRLV_USE_THEME 0` skips installing the theme, for projects that want only
  layout helpers and tokens or bring their own theme.
- **Parent theme:** LVGL's default theme, initialised with MD3 `primary` /
  `secondary`, the dark flag and the Body Medium font, so widgets TreLVis
  doesn't cover still match. Set with `TRLV_THEME_PARENT` (default / none).
- **Following a subject.** TreLVis has no subject binding of its own: an
  integer subject (0 = light, 1 = dark, …) drives the colour mode through
  one observer the project writes once. Add it after `trlv_init`: LVGL calls
  an observer as soon as it is added, which applies the subject's current
  mode.

  ```c
  static const trlv_color_tokens_t * const color_modes[] = { &tokens_light, &tokens_dark };

  static void color_mode_cb(lv_observer_t * observer, lv_subject_t * subject)
  {
      LV_UNUSED(observer);
      int32_t mode = lv_subject_get_int(subject);
      if(mode >= 0 && mode < (int32_t)(sizeof(color_modes) / sizeof(color_modes[0]))) {
          trlv_theme_set_color_mode(color_modes[mode]);
      }
  }

  /* after trlv_init */
  lv_subject_t * color_mode = lv_subject_create(LV_SUBJECT_TYPE_INT);
  lv_subject_add_observer(color_mode, color_mode_cb, NULL);

  /* a light/dark switch anywhere in the UI */
  lv_obj_t * sw = lv_switch_create(parent);
  lv_obj_bind_checked(sw, color_mode);
  ```

  [`tests/theme/test_color_mode.c`](../tests/theme/test_color_mode.c) runs
  this recipe. A built-in binding (`trlv_theme_bind_subject` behind a
  config option) was planned for v1.1 and dropped: it would save only
  these lines.

### 6.4 MD3 widget coverage (v1)

The theme's apply callback checks each object's class with
`lv_obj_check_type()` and styles its parts and states:

| LVGL widget | MD3 treatment |
|---|---|
| Screen | `surface` background, `on_surface` text and the Body Medium role, inherited by everything on it. `trlv_init` restyles the active screen while it is still empty, since it exists before `trlv_init` can run. |
| Plain `lv_obj` | LVGL's container shape (radius, border, padding, scrolling) in MD3 colours: `surface`, `outline_variant` border, `on_surface` content. Boxes, rows and columns remove all theme styles, so this never reaches them. |
| `lv_label` | No role or colour of its own: it inherits Body Medium and `on_surface` from the screen, or the role and `on_` colour of the surface, card or button it sits in. |
| `lv_button` | Filled button by default (`primary` / `on_primary`, radius FULL, height `control_height[MD]`, side padding `space[XL]`, no shadow, Label Large); variants via `trlv_apply_button_type()`. Content is a centred flex row with a `space[SM]` gap, so a label is centred and an icon sits beside it, as in an MD3 button. This differs from LVGL's default theme, which leaves placement to the caller: `lv_obj_align` and `lv_obj_set_pos` on a button's children are ignored, as for any flex child. To place one child by hand (a badge), make it floating with `lv_obj_set_floating(child, true)`; to place them all, `lv_obj_set_layout(btn, LV_LAYOUT_NONE)`. The tab buttons of an `lv_tabview` are left to the parent theme. |
| `lv_switch` | MD3 52 × 32 track (`control_height[SM]` tall, radius FULL): `surface_container_highest` with an `outline` border when off, `primary` when on. Round handle in `outline` (16 px) when off, `on_primary` (24 px) when on, 28 px while pressed, as fractions of the track height. |
| `lv_checkbox` | A `space[SM]` gap before the text. Box with a `border_width` border in `on_surface_variant`, radius XS, 3 dpx around the mark; checked: `primary` with an `on_primary` mark (`LV_SYMBOL_OK`). Text inherits. No recolour on press. |
| `lv_slider`, `lv_bar` | `surface_container_highest` track, `primary` indicator (and slider knob), all radius FULL. The slider knob reaches 6 dpx beyond the track; pressed, it doesn't grow. |
| `lv_arc`, `lv_spinner` | The same pair as arc colours: `surface_container_highest` track, `primary` indicator (and round arc knob, 6 dpx beyond the arc), 15 dpx wide with rounded ends. |
| `lv_dropdown`, `lv_textarea` | MD3 outlined field: `surface`, a `border_width` border in `outline`, radius XS, `on_surface` text, `space[LG]` side padding and `space[SM]` top and bottom, so a one-line field set to `control_height[MD]` fits a line of body text (MD3's 16 px is for its 56 px field). Focused: the border turns `primary` and `focus_width` thick, with no extra ring. LVGL draws borders inside, so the padding shrinks by the extra border width: the field's size, its content box and the text stay where they are. Textarea caret in `primary`, placeholder in `on_surface_variant`. |
| Dropdown list | MD3 menu: `surface_container`, radius XS, the level 2 elevation shadow, `on_surface` text. The selected option is `secondary_container` / `on_secondary_container`; a pressed option gets the state layer. |
| All interactive widgets | Disabled: `on_surface` at 12 % for containers and tracks, 38 % for content, indicators, knobs and check boxes, as real opacity. Keypad or encoder focus (and encoder editing): the focus ring (`focus`, `focus_width`, `space[2XS]` outside), except fields, which use their border. LVGL's default recolour on press, grow on press and focus outline are switched off. |

Buttons, in every variant, and the dropdown list's options also get
pressed state layers, and buttons a focus state layer, pre-mixed (§6.1).
The switch, checkbox, slider and arc don't have the MD3 circular state
layer around their handle: LVGL has no part for it.

The theme gives the widgets it covers their shape as well as their
colours, so they look the same with `TRLV_THEME_PARENT_NONE`. Sizes with
a token come from the layout tokens; the few with none (arc width, knob
and check-mark padding) are LVGL's default-theme values in dpx, scaled by
the display's DPI as LVGL scales them (`LV_DPX_CALC`).

Other widgets fall back to the parent default theme, seeded with MD3
colours. With `TRLV_THEME_PARENT_NONE`, they are unstyled.

**Coverage audit (LVGL 9.6).** Every LVGL 9.6 widget was drawn in light and
dark with the template tokens, with each parent theme setting. The table
records the result. The default-parent column means:

- **covered**: styled by TreLVis (table above).
- **fine via the parent theme**: LVGL's default theme, seeded with MD3
  colours, looks at home next to MD3 widgets.
- **looks wrong next to MD3**: usable, but in LVGL's neutral white and
  greys instead of the `surface` roles, or with white text on `primary`,
  which has poor contrast in dark mode (MD3 dark `primary` is light). Each
  has a v1.x issue, built when a product needs it.
- **content only**: the widget draws images, media or colours the caller
  passes, so a theme has nothing to add. These weren't drawn.

The last column says what `TRLV_THEME_PARENT_NONE` leaves.

| Widget | Default parent theme | `TRLV_THEME_PARENT_NONE` |
|---|---|---|
| Screen, plain `lv_obj`, `lv_label`, `lv_button` | covered | usable |
| `lv_switch`, `lv_checkbox`, `lv_slider`, `lv_bar` | covered | covered |
| `lv_arc`, `lv_spinner` | covered | covered |
| `lv_dropdown`, `lv_textarea` | covered | covered |
| `lv_arclabel`, `lv_spangroup` | fine via the parent theme: text inherits the role and colour | usable |
| `lv_led` | fine via the parent theme (`primary`) | invisible |
| `lv_line` | fine via the parent theme | invisible: no line width |
| `lv_scale` | fine via the parent theme: ticks and labels in the text colour | labels only, no ticks |
| `lv_tabview` | tab bar fine via the parent theme; content area looks wrong: neutral grey ([#139](https://github.com/tmitchell-dev/trelvis/issues/139)) | active tab not marked |
| `lv_tileview` | looks wrong: neutral grey ([#139](https://github.com/tmitchell-dev/trelvis/issues/139)) | usable |
| `lv_roller` | looks wrong: neutral card; selected row white on `primary` ([#132](https://github.com/tmitchell-dev/trelvis/issues/132)) | selected row not marked |
| `lv_buttonmatrix`, `lv_keyboard` | looks wrong: neutral grey keys with a shadow; checked key white on `primary` ([#133](https://github.com/tmitchell-dev/trelvis/issues/133)) | keys have no background |
| `lv_msgbox` | looks wrong: neutral header and card; its buttons aren't `lv_button`s, so they get LVGL's small button with a shadow ([#134](https://github.com/tmitchell-dev/trelvis/issues/134)) | no background or border |
| `lv_spinbox` | looks wrong: a neutral card next to outlined fields (it is a textarea subclass, which the theme's exact class check doesn't match); cursor white on `primary` ([#135](https://github.com/tmitchell-dev/trelvis/issues/135)) | cursor not shown |
| `lv_table` | looks wrong: neutral card, grey cell borders ([#136](https://github.com/tmitchell-dev/trelvis/issues/136)) | usable, no cell padding or borders |
| `lv_chart` | looks wrong: neutral card and grid; series colours come from the caller ([#137](https://github.com/tmitchell-dev/trelvis/issues/137)) | nothing drawn |
| `lv_calendar` | looks wrong: neutral card, a bordered cell per day; the header arrows are covered buttons ([#138](https://github.com/tmitchell-dev/trelvis/issues/138)) | usable, today not marked |
| `lv_ime_pinyin` | follows the button matrix and keyboard: its candidate panel is a button matrix | as the button matrix |
| `lv_list`, `lv_menu`, `lv_win` | not styled: deprecated in LVGL 9.6. Build them from rows and columns ([CONTRIBUTING](../CONTRIBUTING.md#lvgl-usage-v10-forward-compatibility)) | unstyled |
| `lv_image`, `lv_imagebutton`, `lv_animimg`, `lv_canvas` | content only | content only |
| `lv_barcode`, `lv_qrcode` | content only: set MD3 colours with `lv_barcode_set_dark_color` / `lv_qrcode_set_dark_color` and their `light` pair | content only |
| `lv_gif`, `lv_lottie`, `lv_ffmpeg_player`, `lv_gstreamer`, `lv_3dtexture` | content only | content only |

`TRLV_THEME_PARENT_NONE` therefore suits a product that uses only the
covered widgets or styles the others itself.

**Focus rings need clearance.** The ring is drawn outside the widget
(`focus_width + space[2XS]`, 5 px in the template), and LVGL clips children
to their parent. A container that holds focusable widgets therefore needs
at least that much padding, or the ring is cut off at its edges: for the
template, `trlv_set_pad(row, TRLV_SPACE_SM)`. Only the edges of the widget's
ancestors matter; the gap between siblings is free space for the ring.
Treat it as a layout rule and put the clearance in the design
([figma-handoff.md](figma-handoff.md)). `lv_obj_set_overflow_visible` on
the container does not help: LVGL extends the clip area only by the
container's own outline or shadow size, which is zero for a box. Products
with touch input only never show the ring.

**Focus moved in code may show no ring.** The ring follows LVGL's
`LV_STATE_FOCUS_KEY`, which LVGL sets only when the focus comes from a
keypad or encoder. Key presses always qualify. Focus moved in code
(`lv_group_focus_next`, `lv_group_focus_obj`) is credited to an input
device LVGL picks, and it prefers a pointer: in a product with a touch
screen and keys, focus set in code (on a new screen, say) gets
`LV_STATE_FOCUSED` without the ring until the next key press. Add
`LV_STATE_FOCUS_KEY` yourself if the ring should show at once.

### 6.5 Style catalogue

TreLVis keeps its shared styles internal, with one exception: a style is
exposed in [`trlv_styles.h`](../include/trelvis/trlv_styles.h) only when no
modifier can apply it to an object that already exists. That leaves three
constant styles:

- `trlv_style_box`: the box reset (transparent, no border or padding, fit content);
- `trlv_style_row` and `trlv_style_column`: the flex flow.

`trlv_box_create`, `trlv_row_create` and `trlv_column_create` create new objects, so these styles
are how a project turns an existing object into a row or column, for
example a plain `lv_obj` list item that keeps its theme look:
`lv_obj_add_style(item, &trlv_style_row, 0)`. (An `lv_button` is already a
centred row through the theme.)

Every other style is reached through its modifier, which works on any
object (also inside a project's own theme apply callback) and replaces the
group's previous style:

| Styles (internal) | Kind | Modifier |
|---|---|---|
| Gap, pad: one per space step | token | `trlv_set_gap`, `trlv_set_pad` |
| Justify, align items: one per value | constant | `trlv_set_justify`, `trlv_set_align_items` |
| Fit, fill (100 %), grow: per axis | constant | `trlv_set_*_fit`, `trlv_set_*_fill` |
| Text: one per role | token | `trlv_text_create`, `trlv_apply_text_role` |
| Text emphasis: lower emphasis | token | `trlv_apply_text_emphasis` |
| Surfaces: one per `TRLV_SURFACE_*`; cards: filled, elevated, outlined | token | `trlv_apply_surface`, `trlv_apply_card` |
| Buttons: tonal, outlined, text, each with state styles | token | `trlv_apply_button_type` |
| Divider: horizontal, vertical | token | `trlv_divider_create` |

The theme's own styles (screen, plain object, each covered widget and the
shared state styles) are internal too, and only the theme applies them.
They are token styles like the rest: `trlv_init` and every colour-mode
switch rewrite them in place.

## 7. The API by kind

TreLVis has five kinds of function. The verb in the name says which
kind a call is, as in LVGL, so a call site reads the same way whether it
was written by hand, ported from Figma or generated:

| Kind | Name | Takes | Returns | Rule |
|---|---|---|---|---|
| [Setup](#71-setup) | `trlv_init`, `trlv_theme_set_color_mode` | the display, token sets | nothing | Once at start-up; the colour mode at any time. |
| [Constructors](#72-constructors-_create) | `trlv_<thing>_create` | the parent, plus only arguments whose type is clear at the call site | the new `lv_obj_t *` | Containers start empty: no gap, no padding, fit content. |
| [Layout setters](#73-layout-setters-trlv_set_) | `trlv_set_<property>` | any object | nothing | One row of Figma's auto layout panel. Never changes colour. |
| [Appearance](#74-appearance-trlv_apply_) | `trlv_apply_<look>` | any object | nothing | Figma's "apply a style": colour, shape, type. Never changes layout. |
| [Getters](#75-getters-trlv_get_) | `trlv_get_<what>_tokens` | nothing | the current tokens | For raw LVGL calls that need a token value. |

Setters and applies together are the **modifiers**. They follow the same
three rules:

- **Any object.** They work on every `lv_obj_t`, not only objects TreLVis
  created: a row, an `lv_obj`, an `lv_button`. Raw LVGL calls stay valid
  on the same objects.
- **Replace, don't stack.** Each modifier group (gap, padding, width,
  surface, button type, …) replaces the style it applied before, which
  is what `set` means in LVGL (`lv_obj_add_style` stacks,
  `lv_obj_set_*` replaces). Calling `trlv_set_pad` twice leaves one
  padding.
- **Shared, not local.** In LVGL, `lv_obj_set_style_*` adds a *local*
  style to the one object, which costs RAM per object. A modifier uses a
  *shared* style (one per value) instead, so `trlv_set_pad` on a thousand
  objects costs no more RAM than on one. As shared styles, they sit below
  project styles and local styles in precedence (§6.2).

**How a screen reads.** One block per Figma frame, in the same order
every time: the constructor, then one setter per auto layout property
(gap, padding, width and height, justify, align items), then the looks it
applies. Children follow their parent's block:

```c
lv_obj_t * card = trlv_column_create(parent);
trlv_set_gap(card, TRLV_SPACE_SM);
trlv_set_pad(card, TRLV_SPACE_LG);
trlv_set_width_fill(card);
trlv_apply_card(card, TRLV_CARD_FILLED);

lv_obj_t * header = trlv_row_create(card);
trlv_set_gap(header, TRLV_SPACE_SM);
trlv_set_width_fill(header);                          /* space-between needs free space */
trlv_set_justify(header, TRLV_JUSTIFY_SPACE_BETWEEN);
trlv_set_align_items(header, TRLV_ALIGN_ITEMS_CENTER);
trlv_text_create(header, "Temperature", TRLV_TEXT_TITLE_MEDIUM);
trlv_text_create(header, "37.2 °C", TRLV_TEXT_HEADLINE_SMALL);

lv_obj_t * btn = lv_button_create(card);          /* MD3 filled button via the theme */
trlv_apply_button_type(btn, TRLV_BUTTON_TONAL);
trlv_text_create(btn, "Calibrate", TRLV_TEXT_LABEL_LARGE);
```

The order is a convention for reading, not a rule of the API: setters and
applies touch different style properties, so any order gives the same
result. The one ordering that matters is the parent's direction before
`trlv_set_*_fill` on its children (§7.3), and a constructor always sets
it.

The [demo](../examples/demo) uses every function on real screens, and
each of its sections is a worked example.

### 7.1 Setup

| Function | When | What it does |
|---|---|---|
| `trlv_init(disp, color, layout)` | Once, after creating the display and before building screens | Takes the project's token sets, builds the shared styles and installs the MD3 theme on the display. Objects created before it keep their old theme styles. |
| `trlv_theme_set_color_mode(color)` | Any time after `trlv_init` | Switches to another colour token set (light, dark, …): rewrites the shared styles in place and refreshes every object. Layout tokens stay as set at `trlv_init`. |

Setup functions act on the whole library, not on an object, so they are
not modifiers. The details, including driving the colour mode from a
subject, are in §6.3.

### 7.2 Constructors (`_create`)

| Function | Figma | CSS | Behaviour |
|---|---|---|---|
| `trlv_box_create(parent)` | Frame (no auto layout) | `div` | `lv_obj` with theme styles removed + `trlv_style_box`: transparent, no padding, **not scrollable, not clickable**, fit-content size. Place children freely with `lv_obj_align`; with `lv_obj_set_floating` this also covers overlays. |
| `trlv_row_create(parent)` | Auto layout → | `flex-direction: row` | box + row style. No gap until `trlv_set_gap`. |
| `trlv_column_create(parent)` | Auto layout ↓ | `flex-direction: column` | box + column style. No gap until `trlv_set_gap`. |
| `trlv_spacer_create(parent)` | Empty frame, fill container | `flex: 1` | box + `trlv_set_*_fill` along the parent's main axis, chosen at call time. Pushes the items after it to the far end. |
| `trlv_text_create(parent, text, role)` | Text layer + MD3 text style | `p`, `h1`… | Label + role style. Fits its content like any LVGL label (Figma "auto width"); `trlv_set_width_fill` gives it a width, so it wraps (Figma "auto height"). Colour is inherited from the enclosing surface. |
| `trlv_divider_create(parent)` | MD3 divider, 1 px line | `hr` | `border_width` line in `outline_variant`. Horizontal in a column (or no flex), vertical in a row, chosen at call time. Insets: LVGL margins. |

**Defaults.** Layout containers (box, row, column) start like a fresh
Figma frame or a CSS `div`: no gap, no padding, fit-content size. LVGL widgets
come with their MD3 spec padding from the theme, as Figma kit components
do: every `lv_button` is padded. A setter not called means the default, as
in LVGL and CSS; `trlv_set_pad(obj, TRLV_SPACE_NONE)` says "no padding" on
purpose.

**No card constructor.** A card is a look applied to a row or column, as
a surface is: `trlv_set_pad` (MD3 cards use `LG`), then `trlv_apply_card`.
Every part of an MD3 card (headline, subhead, media, supporting text,
actions) is optional, so a card's content is children, and a card with
fixed content is a project component (§2), such as
`stat_card_create(parent, label, value)`.

There is no grid or page constructor. Grids use LVGL's own grid API on a
`trlv_box_create`, and a page skeleton (header / body / footer) is a project
component.

### 7.3 Layout setters (`trlv_set_*`)

All setters work on **any** `lv_obj_t` and each group replaces its
previous style. They never change colour.

| Group | Functions | Notes |
|---|---|---|
| Spacing | `trlv_set_gap(obj, space)`, `trlv_set_pad(obj, space)` | Different horizontal / vertical values, or one side: use raw LVGL with a token value (below). |
| Distribution | `trlv_set_justify(obj, TRLV_JUSTIFY_START / CENTER / END / SPACE_BETWEEN / SPACE_AROUND / SPACE_EVENLY)`, `trlv_set_align_items(obj, TRLV_ALIGN_ITEMS_START / CENTER / END)` | Two named calls in place of `lv_obj_set_flex_align`'s three positional arguments, with shared styles instead of a local one. `ALIGN_ITEMS`, not `ALIGN`, so the values aren't mistaken for `lv_align_t`. The enum values equal LVGL's `LV_FLEX_ALIGN_*`. LVGL flex has no cross-axis stretch; use `trlv_set_*_fill` on each child, as in Figma (in CSS a column's children stretch by default). Expect `trlv_set_width_fill` to be one of the most frequent calls in a screen. |
| Sizing | `trlv_set_width_fit`, `trlv_set_height_fit`, `trlv_set_width_fill`, `trlv_set_height_fill` | Fixed size: `lv_obj_set_size`. Fit and fill are one group per axis, so `trlv_set_width_fit` also clears an earlier `trlv_set_width_fill`. |

**Axis-aware fill.** Figma's "fill container" means *grow* along the parent's
main axis and *stretch* along its cross axis. `trlv_set_width_fill` and
`trlv_set_height_fill` check the parent's flex flow and apply the grow style or the 100 % style
(100 % when the parent has no flex layout). **Set the parent's direction
first**: the decision is made when the modifier is called, and it stays as
set, like any LVGL style. If the parent's direction changes later, call the
fill modifier again.

**Fill inside a parent that fits its content.** Figma doesn't allow it; TreLVis adds no
behaviour of its own, so LVGL's result stands
(`tests/layout/test_fill_warning.c` pins it):

- **Across the axis** (100 %), it works: LVGL leaves the filled child out of
  the parent's content size, so the parent fits its other children and the
  filled child stretches to the widest of them, as with CSS
  `align-self: stretch`. This is a useful pattern, for example equal-width
  items in a menu that fits its content. With no sized sibling the child is
  0 wide (a text then wraps one character per line).
- **Along the main axis** (grow), it always fails: the parent sizes itself
  without the growing child, so there is no free space and the child
  collapses to 0. `trlv_set_*_fill` logs an `LV_LOG_WARN` naming the object when
  it sees this at the time of the call. The warning changes nothing and
  compiles out with LVGL logging. Size the parent before filling its
  children along its main axis.

**Left to LVGL.** These Figma and CSS concepts are already short and clear in
LVGL, so TreLVis adds no function for them. Setters named
`lv_obj_set_style_*` create a local style on the object:

| Need | LVGL |
|---|---|
| Horizontal / vertical gap | `lv_obj_set_style_pad_column(obj, trlv_get_layout_tokens()->space[TRLV_SPACE_MD], 0)`, `…pad_row` |
| Horizontal / vertical padding, one side | `lv_obj_set_style_pad_hor(obj, trlv_get_layout_tokens()->space[TRLV_SPACE_LG], 0)`, `…pad_ver`, `…pad_left`, … |
| Absolute position | `lv_obj_set_floating(obj, true)`, then `lv_obj_align(obj, LV_ALIGN_TOP_RIGHT, x, y)`. Floating, unlike the ignore-layout flag, also leaves the object out of a fit-content parent's size, as in Figma. |
| Wrap | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP)` |
| Min / max size | `lv_obj_set_style_min_width(obj, px, 0)`, `…max_width`, `…min_height`, `…max_height` |
| Scroll | `lv_obj_set_scrollable(obj, true)`, then `lv_obj_set_scroll_dir(obj, LV_DIR_VER)` |
| Clip content | LVGL clips children by default. With rounded corners, add `lv_obj_set_style_clip_corner(obj, true, 0)`. |
| Show / hide | `lv_obj_set_hidden(obj, true)`. Hidden objects leave the layout (CSS `display: none`). |
| Truncate text | `lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS)` |
| Grid | `lv_obj_set_grid_dsc_array(obj, cols, rows)` on a `trlv_box_create`, `lv_obj_set_grid_cell(child, …)`. The track arrays are referenced, not copied, so keep them `static`. |

### 7.4 Appearance (`trlv_apply_*`)

Each applies a shared look to any object, replaces the look its group had
and never changes layout. Surfaces, custom surfaces and cards are one
group and replace each other; a button type is a group of its own. The
looks are token styles, so they recolour on `trlv_theme_set_color_mode`.

| Group | Functions | Notes |
|---|---|---|
| Surfaces | `trlv_apply_surface(obj, TRLV_SURFACE_SURFACE / CONTAINER_LOWEST / CONTAINER_LOW / CONTAINER / CONTAINER_HIGH / CONTAINER_HIGHEST / PRIMARY_CONTAINER / SECONDARY_CONTAINER / TERTIARY_CONTAINER / ERROR_CONTAINER / INVERSE / DIM / BRIGHT)`, `trlv_apply_surface_custom(obj, index, TRLV_CUSTOM_SURFACE_COLOR / CONTAINER)` | Sets container colour + matching `on_` text colour (inherited). Tone only: no radius, border or padding. `trlv_apply_surface_custom` does the same with a project custom colour or its container. One group with cards: `trlv_apply_surface` and `trlv_apply_card` replace each other. |
| Cards | `trlv_apply_card(obj, TRLV_CARD_FILLED / ELEVATED / OUTLINED)` | MD3 cards: tone + radius MD + border/shadow, on a row or column. No padding: set it with `trlv_set_pad`, `LG` for MD3 cards. A header, media and actions are children. Not interactive in v1. |
| Buttons | `trlv_apply_button_type(btn, TRLV_BUTTON_FILLED / TONAL / OUTLINED / TEXT)` | Restyles an `lv_button`, or any clickable object (the theme default is FILLED): container colour, border and content colour, with pressed / focus / disabled states. Shape, height and text role stay the theme's. |
| Text | `trlv_apply_text_role(obj, role)` | On a label it changes the role; on a container it sets the role for every label inside that has none of its own (inheritance). |
| Text emphasis | `trlv_apply_text_emphasis(obj, TRLV_TEXT_EMPHASIS_LOW / NORMAL)` | LOW sets the inherited text colour to `on_surface_variant` (tags, units, captions, supporting text), on a label or on a container for every label inside; NORMAL removes it. Shared, so it follows colour-mode switches. Independent of the text role. |

**Odd targets warn.** Like LVGL styles, each look applies to any object.
Where the object makes no sense for it, the function logs an
`LV_LOG_WARN` naming the object, then applies the look anyway (the warning
compiles out with LVGL logging):

- `trlv_apply_card` on anything but a plain container (box, row, column,
  `lv_obj`): on a widget the card mixes with the widget's own parts and
  states;
- `trlv_apply_surface` or `trlv_apply_surface_custom` on an `lv_button`:
  only the colour at rest changes, while pressed and focus keep the
  button's colours. A custom-coloured button (a red Stop button) is not
  part of v1;
- `trlv_apply_button_type` on an object that isn't clickable: its pressed
  and focus states never show.

### 7.5 Getters (`trlv_get_*`)

| Function | Returns |
|---|---|
| `trlv_get_color_tokens()` | The colour tokens in use: the set passed to `trlv_init` or the latest `trlv_theme_set_color_mode`. |
| `trlv_get_layout_tokens()` | The layout tokens passed to `trlv_init`. |

Modifiers take token names (`TRLV_SPACE_LG`, `TRLV_SURFACE_CONTAINER`)
and look the values up themselves. The getters are for the places a
modifier doesn't reach: raw LVGL calls (§7.3 "Left to LVGL") and widget
APIs that take a value, such as a chart series colour.

**Token values in raw LVGL are snapshots.** `trlv_get_color_tokens()` and
`trlv_get_layout_tokens()` return the current values, and a local style
(`lv_obj_set_style_*`) or a widget call such as `lv_chart_add_series`
stores the value it was given. TreLVis's own shared styles follow
`trlv_theme_set_color_mode`; these don't, so after a switch to dark they
keep the light colour. Layout values are fixed at `trlv_init`, so only
colours are affected. If the product switches colour mode, either:

- re-apply the colours in the code that calls
  `trlv_theme_set_color_mode` (or in the subject observer in §6.3), or
- keep the colours in the project's own shared `lv_style_t`, re-set its
  properties from `trlv_get_color_tokens()` on each switch, and call
  `lv_obj_report_style_change(&style)`, as the theme does.

A product with a single colour mode can ignore this.

### 7.6 Debug aids

- `TRLV_USE_DEBUG_OUTLINE` draws a 1 px outline around every box, row,
  column and spacer. It is drawn outside the object, in magenta,
  and takes no space, so the layout it shows doesn't change.
- *(planned, v1.x)* `trlv_debug_dump(obj)` prints the tree with sizes,
  padding, gaps and styles for comparison with Figma (`TRLV_USE_DEBUG`).

## 8. Memory and threading

- TreLVis **never calls `malloc` / `lv_malloc` directly**.
- Shared styles allocate nothing, not even through LVGL. Constant styles use
  `LV_STYLE_CONST_INIT` (flash, no RAM). Token styles use it too, with their
  property array in static RAM (8 bytes per property on a 32-bit MCU, the
  `lv_style_t` itself in flash); `trlv_init` and colour-mode switches
  overwrite the values in place. Token structs are `const` (flash on MCUs).
- LVGL allocates internally for theme objects. TreLVis triggers this **only
  in `trlv_init` and colour-mode switches**, and it is bounded. Creating
  objects allocates as usual in LVGL.
- Every modifier applies shared styles; none creates a local style on the
  object. A value with no shared style (an uncommon padding, gap or size)
  is raw LVGL in the project (§7.3), with LVGL's per-object local style.
- **Threading:** call every TreLVis function from the LVGL thread only, the
  same rule as for LVGL itself.

## 9. LVGL behaviours TreLVis handles

LVGL has defaults that surprise people coming from Figma or CSS. TreLVis
handles them, and its docs note where:

| LVGL behaviour | What TreLVis does |
|---|---|
| Plain `lv_obj`s are scrollable and clickable by default, and themed (background, border, padding). | `trlv_box_create` and the layout primitives remove theme styles and turn scroll and click off. Scrolling is opt-in via `lv_obj_set_scrollable`. |
| Labels fit their text and don't wrap unless they have a width. | `trlv_text_create` keeps that default (Figma "auto width"): names, values, headings and button labels all want to fit their content. `trlv_set_width_fill` gives the label a width, so it wraps (Figma "auto height"); a paragraph needs it. |
| A child that fills (grows) along the main axis of a parent that fits that axis to its content collapses to 0. | `trlv_set_*_fill` logs a warning naming the object. Filling *across* a fit-content parent stretches to the widest sibling and is left alone (§7.3). |
| `lv_obj_set_style_*` allocates a local style per object. | Modifiers use shared styles; any exceptions are documented. |
| A border sits inside the object's size and insets the content by its width, on top of the padding. | Matches Figma "inside stroke, include in layout" and CSS `box-sizing: border-box`. See the hand-off guide. |
| LVGL uses line *spacing*, Figma uses line *height*. | Tokens use line height; TreLVis converts (`line_height − font line height`). |
| Object flags use per-flag setters in 9.6+ (`lv_obj_set_scrollable`). | TreLVis uses them only; the generic flag API is deprecated. |
| Style precedence: later-added styles win, and local styles beat all. | Documented precedence (§6.2). Modifier groups replace their own previous style. |
| Only some properties inherit (text colour, font, letter/line spacing, opa…); background and padding don't. | Surfaces/cards set the `on_` text colour so it inherits; documented per function. |
| Flex has no cross-axis stretch. | `trlv_set_*_fill` emulates it with 100 % on the cross axis. |
| `flex_cross_place` places items within their line, which is only as deep as the deepest item and sits at the start of the container, so a centred short item lines up with its tallest sibling, not the container. | `trlv_set_align_items` sets `flex_track_place` to the same value, so items are placed in the container as in Figma and CSS. |

## 10. Mapping: Figma / MD3 → CSS → TreLVis → LVGL

Rows marked *raw LVGL* have no TreLVis function on purpose: the LVGL call is
already clear (see [What earns a place](#what-earns-a-place)).

For the table applied to a whole card, see
[A taste of the API](../README.md#a-taste-of-the-api) in the README: one
card written with TreLVis and with plain LVGL side by side, checked by
[`tests/layout/test_readme_card.c`](../tests/layout/test_readme_card.c) to
lay out the same.

| Figma / MD3 | CSS | TreLVis | LVGL mechanism |
|---|---|---|---|
| Frame, no auto layout | `div` | `trlv_box_create` | `lv_obj` + reset style, scroll/click off |
| Auto layout → / ↓ | `display: flex; flex-direction: row / column` | `trlv_row_create` / `trlv_column_create` | `layout = LV_LAYOUT_FLEX`, `flex_flow` |
| Empty frame, fill container | `flex: 1` | `trlv_spacer_create` | box + `flex_grow` |
| Divider (MD3) | `hr` | `trlv_divider_create` | box + `bg_color = outline_variant` |
| Grid auto layout | `display: grid` | *raw LVGL* | `lv_obj_set_grid_dsc_array`, `lv_obj_set_grid_cell` |
| Gap between items | `gap` | `trlv_set_gap` | `pad_column`, `pad_row` |
| Horizontal / vertical gap | `column-gap`, `row-gap` | *raw LVGL* | `lv_obj_set_style_pad_column`, `…pad_row` |
| Padding | `padding` | `trlv_set_pad` | `pad_left/right/top/bottom` |
| Horizontal / vertical / one-side padding | `padding: y x`, `padding-left` | *raw LVGL* | `lv_obj_set_style_pad_hor`, `…pad_ver`, `…pad_left`, … |
| Alignment (main axis) / space between | `justify-content` | `trlv_set_justify` | `flex_main_place` |
| Alignment (cross axis) | `align-items` | `trlv_set_align_items` | `flex_cross_place` + `flex_track_place` |
| Wrap | `flex-wrap` | *raw LVGL* | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP)` |
| Hug contents | `width: fit-content` | `trlv_set_width_fit/height_fit` | `LV_SIZE_CONTENT` |
| Fill container | `flex: 1` / `align-self: stretch` | `trlv_set_width_fill/height_fill` | `flex_grow` or `LV_PCT(100)` |
| Fixed size | `width`, `height` | *raw LVGL* | `lv_obj_set_size` |
| Min / max size | `min-width`, `max-width`, … | *raw LVGL* | `lv_obj_set_style_min_width`, `…max_width`, … |
| Absolute position, constraints | `position: absolute` | *raw LVGL* | `lv_obj_set_floating` + `lv_obj_align` |
| Clip content | `overflow: hidden` | *raw LVGL* | LVGL default; `lv_obj_set_style_clip_corner` for rounded corners |
| Scroll | `overflow: auto` | *raw LVGL* | `lv_obj_set_scrollable` + `lv_obj_set_scroll_dir` |
| Visible | `display: none` | *raw LVGL* | `lv_obj_set_hidden` |
| Colour variable (MD3 role) | custom property `--md-sys-color-*` | `trlv_get_color_tokens()->role` | `lv_color_t` in styles |
| Surface container | `background` + `color` | `trlv_apply_surface` | `bg_color` + inherited `text_color` |
| Custom colour (Material Theme Builder) | `--md-extended-color-*` | `trlv_apply_surface_custom` | `bg_color` + inherited `text_color` |
| MD3 card | — | `trlv_apply_card` | shared style: `bg_color`, `radius`, `border_*` / `shadow_*` |
| MD3 button type (filled, tonal, outlined, text) | button classes | `trlv_apply_button_type` | shared styles with `LV_STATE_PRESSED` / `FOCUS_KEY` / `DISABLED` selectors |
| Text style (type scale) | `font`, `line-height`, `letter-spacing` | `trlv_text_create`, `trlv_apply_text_role` | `text_font`, `text_line_space`, `text_letter_space` |
| Lower-emphasis text (*On Surface Variant*) | lower-emphasis `color` | `trlv_apply_text_emphasis` | inherited `text_color` |
| List (MD3 list items) | `ul` / `li` | `trlv_column_create` of `trlv_row_create` items with `trlv_divider_create` | flex; `lv_list` is deprecated in LVGL 9.6 |
| Truncate text | `text-overflow: ellipsis` | *raw LVGL* | `lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS)` |
| Corner radius (shape scale) | `border-radius` | `TRLV_RADIUS_*` | `radius` |
| Stroke, inside | `border` (`box-sizing: border-box`) | cards, fields | `border_width`, `border_color` |
| Focus ring (outside stroke) | `outline` | theme | `outline_width`, `outline_color` |
| Drop shadow (elevation) | `box-shadow` | `TRLV_ELEVATION_*` | `shadow_*` |
| Interactive states / variants | `:active`, `:focus-visible`, `:checked`, `:disabled` | theme state styles | `LV_STATE_*` selectors |
| Component | component | `<name>_create(parent, …)` (project) | function returning `lv_obj_t *` |
