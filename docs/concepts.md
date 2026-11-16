# TreLVis concepts

This document explains how TreLVis is designed: its vocabulary, design tokens,
fonts, how it uses LVGL's theme and style engine, and the primitives and
modifiers it provides.

> **Status:** this is the v1 design. A provisional slice of it is implemented
> (milestone 2): the token types and template, `trlv_init` without a theme,
> `trlv_box`, `trlv_row`, `trlv_column`, `trlv_gap`, `trlv_pad`,
> `trlv_hug_*`, `trlv_fill_*` and `trlv_text`. The rest is designed but not
> built yet. The public API is reviewed next (milestone 3).
> Once the headers land, they are the source of truth for exact signatures;
> this document explains the reasons behind them. Sections marked *(planned)*
> describe features that are not implemented yet.

- [1. What TreLVis is](#1-what-trelvis-is)
- [2. Vocabulary](#2-vocabulary)
- [3. Naming](#3-naming)
- [4. Design tokens](#4-design-tokens)
- [5. Fonts](#5-fonts)
- [6. Theme and style engine](#6-theme-and-style-engine)
- [7. Primitives and modifiers](#7-primitives-and-modifiers)
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
  (such as `trlv_box` not scrolling) is documented on the function.

### What earns a place

TreLVis stays small on purpose. Its main job is to turn the blocks of LVGL
calls that every project repeats into one standard call. A public function
exists only if it does at least one of these:

- **Replaces a block of LVGL calls that projects repeat**, so it is written
  once and the same way everywhere (`trlv_row(parent, gap)`: create the
  object, reset it, set the flow, set the gap).
- **Takes a design token**, so screens hold no magic numbers
  (`trlv_pad(obj, TRLV_SPACE_LG)`).
- **Fixes an LVGL default that surprises** people coming from Figma or CSS
  (`trlv_box`: not scrollable, not clickable, no theme padding).
- **Makes a decision LVGL leaves to the caller** (`trlv_fill_width`: grow or
  100 %, depending on the parent's direction).

A function that only renames a single clear LVGL call does not qualify.

## 2. Vocabulary

| Term | Meaning | Examples |
|---|---|---|
| **Primitive** | A TreLVis building block (this library). | `trlv_box`, `trlv_row`, `trlv_text` |
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

- **Grammar:** constructors are **nouns** that create an object
  (`trlv_row(parent, …)`). Modifiers are **verbs or properties** that work on
  any object (`trlv_justify(obj, …)`).
- **No abbreviations** except LVGL's `pad`: `trlv_column`, `trlv_fill_width`,
  `trlv_align_items`.
- **Fully qualified enums:** `TRLV_JUSTIFY_SPACE_BETWEEN`, `TRLV_ALIGN_ITEMS_CENTER`,
  `TRLV_SPACE_MD`, `TRLV_TEXT_BODY_MEDIUM`, `TRLV_SURFACE_CONTAINER_HIGH`.
- **Prefixes:** functions `trlv_*`, types `trlv_*_t`, macros and enums
  `TRLV_*`, config `TRLV_USE_*` / `TRLV_*`, styles `trlv_style_*`.

**Token names are MD3 role names in snake_case:**

| MD3 / Figma | C |
|---|---|
| Surface Container High | `trlv_color()->surface_container_high` |
| On Primary Container | `trlv_color()->on_primary_container` |
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
| Color | light, dark, brand… | **at runtime** | MD3 colour roles, extensions, state-layer opacities |
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
| `trlv_color_tokens_t` | Color collection: the MD3 colour roles, success / warning extensions, `focus`, and the state opacities |
| `trlv_layout_tokens_t` | Layout collection: spacing, shape, stroke and focus widths, control heights, elevation, type scale |

The structs hold only what v1 reads. Tokens for later features (icon sizes,
hover, motion) are added together with the feature that uses them.

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

The template defines three `const` collections, `tokens_light`, `tokens_dark`
and `tokens_regular`, and works unedited:

- **Colour:** the MD3 baseline light and dark schemes (seed `#6750A4`) as
  published by Material Design, plus example success and warning custom
  colours generated by Material Theme Builder.
- **Layout:** the Regular values below. Each text role uses the Montserrat
  size LVGL ships nearest to its MD3 size, with the MD3 line height, when
  that size is enabled in `lv_conf.h` (`LV_FONT_MONTSERRAT_<size>`). A size
  that isn't enabled falls back to the next smaller enabled size with its
  own line height, and with none enabled to `LV_FONT_DEFAULT`. Nothing
  needs a font file, and the type hierarchy shows as soon as a few sizes
  are enabled.

The template is compiled in CI with the strict warning flags, so it stays in
step with the token types.

The project then owns the file: regenerate the colours from the product's
seed colour, give each role a real font, delete what it doesn't use, and add
a Compact layout where needed.

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
  `trlv_surface` and `trlv_card` do this automatically through inheritance.
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
  and align values, grow, fill width/height, hug) use `LV_STYLE_CONST_INIT`.
  They live in flash and need no RAM and no allocation. **Token-dependent
  styles** (gap, pad, text roles, surfaces, cards) use the same
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
  `trlv_surface` and `trlv_card` set the container colour **and** its `on_`
  text colour, so labels and icons inside inherit the right MD3 pairing.
  A text-role style applied to a container sets the role for all descendant
  labels. The theme styles the screen with `surface` / `on_surface` and the
  Body font.
- **States use selectors.** Pressed, focus-key, checked and disabled
  styles use `LV_STATE_*` selectors. MD3 state layers are **pre-mixed
  colours** (`lv_color_mix(on_role, role, opa)`) in state styles, computed at
  init and on each colour-mode switch. LVGL colour filters are not used for
  state layers because they also tint the content.
- **Transitions** use `lv_style_transition_dsc_t` with motion tokens *(planned, v1.1)*.
- **Modifier groups replace rather than stack.** Calling `trlv_gap` (or
  `trlv_pad`, `trlv_card`, `trlv_surface`, `trlv_justify`, …) again removes
  that group's previous style before adding the new one. The target is 2–4
  TreLVis styles per object. The new style takes the old one's place in the
  object's style list (`lv_obj_replace_style`), so calling a modifier again
  doesn't change its precedence: a project style added after it still wins.

### 6.2 Style precedence

From lowest to highest priority:

1. theme styles
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
const trlv_color_tokens_t  * trlv_color(void);
const trlv_layout_tokens_t * trlv_layout(void);
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
- *(planned, v1.1)* `TRLV_USE_SUBJECT_THEME`:
  `trlv_theme_bind_subject(lv_subject_t * subject, const trlv_color_tokens_t * const modes[], uint32_t count)`.
  The app sets an integer subject (0 = light, 1 = dark, …) and the theme follows.

### 6.4 MD3 widget coverage (v1)

The theme's apply callback checks each object's class with
`lv_obj_check_type()` and styles its parts and states:

| LVGL widget | MD3 treatment |
|---|---|
| Screen / plain `lv_obj` | `surface` background, `on_surface` text, body font (inherited) |
| `lv_label` | inherits; default role Body Medium |
| `lv_button` | Filled button by default (`primary` / `on_primary`, radius FULL, height `control_height[MD]`, Label Large); variants via `trlv_button_variant()` |
| `lv_switch`, `lv_checkbox` | MD3 track / handle / box colours, checked states |
| `lv_slider`, `lv_bar`, `lv_arc` | `primary` indicator, `surface_container_highest` track |
| `lv_dropdown`, `lv_textarea` | outlined field (`outline`, focus → `primary`), menu on `surface_container` |
| `lv_spinner` | `primary` |
| All interactive widgets | pressed and focus state layers, disabled 38% / 12%, focus-key ring (`focus`, `focus_width`) |

Other widgets fall back to the parent default theme, seeded with MD3 colours.
Coverage is built screen, label and button first, then one widget at a time.

### 6.5 Style catalogue

The shared styles are exposed in `trlv_styles.h` so projects can use them with
`lv_obj_add_style` directly. The modifiers use the same styles:

- **Layout (const):** `trlv_style_box` (reset), `trlv_style_row`,
  `trlv_style_column`, justify/align variants, grow, fill/hug.
- **Spacing (token):** one gap style and one pad style per space step, plus
  no x/y variants (different horizontal and vertical values use raw LVGL).
- **Surfaces (token):** one per `TRLV_SURFACE_*` (container colour + `on_`
  text colour).
- **Cards (token):** `trlv_style_card_filled` (`surface_container_highest`),
  `trlv_style_card_elevated` (`surface_container_low` + elevation 1),
  `trlv_style_card_outlined` (`surface` + 1 px `outline_variant`). All use
  radius MD.
- **Text (token):** `trlv_style_text_<role>` (font, line space, letter space).
- **Buttons (token):** filled, tonal (`secondary_container`), outlined
  (`outline`) and text, each with its state styles.
- **Divider (token):** `outline_variant`.

## 7. Primitives and modifiers

```c
lv_obj_t * card = trlv_column(parent, TRLV_SPACE_SM);
trlv_card(card, TRLV_CARD_FILLED);
trlv_pad(card, TRLV_SPACE_LG);
trlv_fill_width(card);

lv_obj_t * header = trlv_row(card, TRLV_SPACE_SM);
trlv_justify(header, TRLV_JUSTIFY_SPACE_BETWEEN);
trlv_align_items(header, TRLV_ALIGN_ITEMS_CENTER);
trlv_text(header, "Temperature", TRLV_TEXT_TITLE_MEDIUM);
trlv_text(header, "37.2 °C", TRLV_TEXT_DISPLAY_SMALL);

lv_obj_t * btn = lv_button_create(card);          /* MD3 filled button via the theme */
trlv_button_variant(btn, TRLV_BUTTON_TONAL);
```

### 7.1 Constructors (nouns)

| Function | Figma | CSS | Behaviour |
|---|---|---|---|
| `trlv_box(parent)` | Frame (no auto layout) | `div` | `lv_obj` with theme styles removed + `trlv_style_box`: transparent, no padding, **not scrollable, not clickable**, hug size. Place children freely with `lv_obj_align`; with `lv_obj_set_floating` this also covers overlays. |
| `trlv_row(parent, gap)` | Auto layout → | `flex-direction: row` | box + row style + gap style. |
| `trlv_column(parent, gap)` | Auto layout ↓ | `flex-direction: column` | box + column style + gap style. |
| `trlv_spacer(parent)` | Empty frame, fill container | `flex: 1` | box + `trlv_fill_*` along the parent's main axis, chosen at call time. Pushes the items after it to the far end. |
| `trlv_text(parent, text, role)` | Text layer + MD3 text style | `p`, `h1`… | Label + role style. Hugs its content like any LVGL label (Figma "auto width"); `trlv_fill_width` gives it a width, so it wraps (Figma "auto height"). Colour is inherited from the enclosing surface. |
| `trlv_divider(parent)` | MD3 divider, 1 px line | `hr` | `border_width` line in `outline_variant`. Horizontal in a column (or no flex), vertical in a row, chosen at call time. Insets: LVGL margins. |

There is no grid or page constructor. Grids use LVGL's own grid API on a
`trlv_box`, and a page skeleton (header / body / footer) is a project
component.

### 7.2 Modifiers (verbs / properties)

All modifiers work on **any** `lv_obj_t`, not only objects TreLVis created,
and each group replaces its previous style.

| Group | Functions | Notes |
|---|---|---|
| Spacing | `trlv_gap(obj, space)`, `trlv_pad(obj, space)` | Different horizontal / vertical values, or one side: use raw LVGL with a token value (below). |
| Distribution | `trlv_justify(obj, TRLV_JUSTIFY_START / CENTER / END / SPACE_BETWEEN / SPACE_AROUND / SPACE_EVENLY)`, `trlv_align_items(obj, TRLV_ALIGN_ITEMS_START / CENTER / END)` | Two named calls in place of `lv_obj_set_flex_align`'s three positional arguments, with shared styles instead of a local one. `ALIGN_ITEMS`, not `ALIGN`, so the values aren't mistaken for `lv_align_t`. The enum values equal LVGL's `LV_FLEX_ALIGN_*`. LVGL flex has no cross-axis stretch; use `trlv_fill_*` on the children. |
| Sizing | `trlv_hug_width/height`, `trlv_fill_width/height` | Fixed size: `lv_obj_set_size`. Hug and fill are one group, so `trlv_hug_*` also clears an earlier fill. |
| Surfaces | `trlv_surface(obj, TRLV_SURFACE_SURFACE / CONTAINER_LOWEST / CONTAINER_LOW / CONTAINER / CONTAINER_HIGH / CONTAINER_HIGHEST / PRIMARY_CONTAINER / SECONDARY_CONTAINER / TERTIARY_CONTAINER / ERROR_CONTAINER / SUCCESS_CONTAINER / WARNING_CONTAINER / INVERSE)` | Sets container colour + matching `on_` text colour (inherited). Tone only: no radius or border. |
| Cards | `trlv_card(obj, TRLV_CARD_FILLED / ELEVATED / OUTLINED)` | MD3 cards: tone + radius MD + border/shadow. Padding stays explicit (`trlv_pad`, typically `LG`). |
| Buttons | `trlv_button_variant(btn, TRLV_BUTTON_FILLED / TONAL / OUTLINED / TEXT)` | Restyles an `lv_button` (the theme default is FILLED). |
| Text | `trlv_text_role(obj, role)` | On a container it sets the role for its descendants (inheritance). |

**Axis-aware fill.** Figma's "fill container" means *grow* along the parent's
main axis and *stretch* along its cross axis. `trlv_fill_width/height` checks
the parent's flex flow and applies the grow style or the 100 % style
(100 % when the parent has no flex layout). **Set the parent's direction
first**: the decision is made when the modifier is called, and it stays as
set, like any LVGL style. If the parent's direction changes later, call the
fill modifier again.

**Fill inside a parent that hugs.** Figma doesn't allow it; TreLVis adds no
behaviour of its own, so LVGL's result stands. The slice showed what that is
(the tests in `tests/layout/test_fill_warning.c` pin it):

- **Across the axis** (100 %), it works: LVGL leaves the filled child out of
  the parent's content size, so the parent hugs its other children and the
  filled child stretches to the widest of them, as with CSS
  `align-self: stretch`. This is a useful pattern, for example equal-width
  items in a menu that hugs its content. With no sized sibling the child is
  0 wide (a text then wraps one character per line).
- **Along the main axis** (grow), it always fails: the parent sizes itself
  without the growing child, so there is no free space and the child
  collapses to 0. `trlv_fill_*` logs an `LV_LOG_WARN` naming the object when
  it sees this at the time of the call. The warning changes nothing and
  compiles out with LVGL logging. Size the parent before filling its
  children along its main axis.

**Left to LVGL.** These Figma and CSS concepts are already short and clear in
LVGL, so TreLVis adds no function for them. Setters named
`lv_obj_set_style_*` create a local style on the object:

| Need | LVGL |
|---|---|
| Horizontal / vertical gap | `lv_obj_set_style_pad_column(obj, trlv_layout()->space[TRLV_SPACE_MD], 0)`, `…pad_row` |
| Horizontal / vertical padding, one side | `lv_obj_set_style_pad_hor(obj, trlv_layout()->space[TRLV_SPACE_LG], 0)`, `…pad_ver`, `…pad_left`, … |
| Absolute position | `lv_obj_set_floating(obj, true)`, then `lv_obj_align(obj, LV_ALIGN_TOP_RIGHT, x, y)`. Floating, unlike the ignore-layout flag, also leaves the object out of a hugging parent's size, as in Figma. |
| Wrap | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP)` |
| Min / max size | `lv_obj_set_style_min_width(obj, px, 0)`, `…max_width`, `…min_height`, `…max_height` |
| Scroll | `lv_obj_set_scrollable(obj, true)`, then `lv_obj_set_scroll_dir(obj, LV_DIR_VER)` |
| Clip content | LVGL clips children by default. With rounded corners, add `lv_obj_set_style_clip_corner(obj, true, 0)`. |
| Show / hide | `lv_obj_set_hidden(obj, true)`. Hidden objects leave the layout (CSS `display: none`). |
| Truncate text | `lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS)` |
| Grid | `lv_obj_set_grid_dsc_array(obj, cols, rows)` on a `trlv_box`, `lv_obj_set_grid_cell(child, …)`. The track arrays are referenced, not copied, so keep them `static`. |

### 7.3 Debug aids

- `TRLV_USE_DEBUG_OUTLINE` draws a 1 px outline around every box, row,
  column and spacer *(planned, milestone 5)*. It is drawn outside the object
  and takes no space, so the layout it shows doesn't change.
- *(planned, v1.1)* `trlv_debug_dump(obj)` prints the tree with sizes,
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
- Modifiers use shared styles. Where a value has no shared style (an uncommon
  padding, gap or size), the function's documentation states that it uses a
  local style and what that costs.
- **Threading:** call every TreLVis function from the LVGL thread only, the
  same rule as for LVGL itself.

## 9. LVGL behaviours TreLVis handles

LVGL has defaults that surprise people coming from Figma or CSS. TreLVis
handles them, and its docs note where:

| LVGL behaviour | What TreLVis does |
|---|---|
| Plain `lv_obj`s are scrollable and clickable by default, and themed (background, border, padding). | `trlv_box` and the layout primitives remove theme styles and turn scroll and click off. Scrolling is opt-in via `lv_obj_set_scrollable`. |
| Labels hug their text and don't wrap unless they have a width. | `trlv_text` keeps that default (Figma "auto width"), confirmed on the slice's demo screen: names, values, headings and button labels all want to hug. `trlv_fill_width` gives the label a width, so it wraps (Figma "auto height"); a paragraph needs it. |
| A child that fills (grows) along the main axis of a parent that hugs that axis collapses to 0. | `trlv_fill_*` logs a warning naming the object. Filling *across* a hugging parent stretches to the widest sibling and is left alone (§7.2). |
| `lv_obj_set_style_*` allocates a local style per object. | Modifiers use shared styles; any exceptions are documented. |
| Border is drawn inside the padding box. | Matches Figma "inside stroke, include in layout". See the hand-off guide. |
| LVGL uses line *spacing*, Figma uses line *height*. | Tokens use line height; TreLVis converts (`line_height − font line height`). |
| Object flags use per-flag setters in 9.6+ (`lv_obj_set_scrollable`). | TreLVis uses them only; the generic flag API is deprecated. |
| Style precedence: later-added styles win, and local styles beat all. | Documented precedence (§6.2). Modifier groups replace their own previous style. |
| Only some properties inherit (text colour, font, letter/line spacing, opa…); background and padding don't. | Surfaces/cards set the `on_` text colour so it inherits; documented per function. |
| Flex has no cross-axis stretch. | `trlv_fill_*` emulates it with 100 % on the cross axis. |

## 10. Mapping: Figma / MD3 → CSS → TreLVis → LVGL

Rows marked *raw LVGL* have no TreLVis function on purpose: the LVGL call is
already clear (see [What earns a place](#what-earns-a-place)).

| Figma / MD3 | CSS | TreLVis | LVGL mechanism |
|---|---|---|---|
| Frame, no auto layout | `div` | `trlv_box` | `lv_obj` + reset style, scroll/click off |
| Auto layout → / ↓ | `display: flex; flex-direction: row / column` | `trlv_row` / `trlv_column` | `layout = LV_LAYOUT_FLEX`, `flex_flow` |
| Empty frame, fill container | `flex: 1` | `trlv_spacer` | box + `flex_grow` |
| Divider (MD3) | `hr` | `trlv_divider` | box + `bg_color = outline_variant` |
| Grid auto layout | `display: grid` | *raw LVGL* | `lv_obj_set_grid_dsc_array`, `lv_obj_set_grid_cell` |
| Gap between items | `gap` | `trlv_gap` | `pad_column`, `pad_row` |
| Horizontal / vertical gap | `column-gap`, `row-gap` | *raw LVGL* | `lv_obj_set_style_pad_column`, `…pad_row` |
| Padding | `padding` | `trlv_pad` | `pad_left/right/top/bottom` |
| Horizontal / vertical / one-side padding | `padding: y x`, `padding-left` | *raw LVGL* | `lv_obj_set_style_pad_hor`, `…pad_ver`, `…pad_left`, … |
| Alignment (main axis) / space between | `justify-content` | `trlv_justify` | `flex_main_place` |
| Alignment (cross axis) | `align-items` | `trlv_align_items` | `flex_cross_place` |
| Wrap | `flex-wrap` | *raw LVGL* | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP)` |
| Hug contents | `width: fit-content` | `trlv_hug_width/height` | `LV_SIZE_CONTENT` |
| Fill container | `flex: 1` / `align-self: stretch` | `trlv_fill_width/height` | `flex_grow` or `LV_PCT(100)` |
| Fixed size | `width`, `height` | *raw LVGL* | `lv_obj_set_size` |
| Min / max size | `min-width`, `max-width`, … | *raw LVGL* | `lv_obj_set_style_min_width`, `…max_width`, … |
| Absolute position, constraints | `position: absolute` | *raw LVGL* | `lv_obj_set_floating` + `lv_obj_align` |
| Clip content | `overflow: hidden` | *raw LVGL* | LVGL default; `lv_obj_set_style_clip_corner` for rounded corners |
| Scroll | `overflow: auto` | *raw LVGL* | `lv_obj_set_scrollable` + `lv_obj_set_scroll_dir` |
| Visible | `display: none` | *raw LVGL* | `lv_obj_set_hidden` |
| Colour variable (MD3 role) | custom property `--md-sys-color-*` | `trlv_color()->role` | `lv_color_t` in styles |
| Surface container | `background` + `color` | `trlv_surface` | `bg_color` + inherited `text_color` |
| MD3 card | — | `trlv_card` | shared style: `bg_color`, `radius`, `border_*` / `shadow_*` |
| Text style (type scale) | `font`, `line-height`, `letter-spacing` | `trlv_text`, `trlv_text_role` | `text_font`, `text_line_space`, `text_letter_space` |
| Truncate text | `text-overflow: ellipsis` | *raw LVGL* | `lv_label_set_long_mode(label, LV_LABEL_LONG_MODE_DOTS)` |
| Corner radius (shape scale) | `border-radius` | `TRLV_RADIUS_*` | `radius` |
| Stroke, inside | `border` (`box-sizing: border-box`) | cards, fields | `border_width`, `border_color` |
| Focus ring (outside stroke) | `outline` | theme | `outline_width`, `outline_color` |
| Drop shadow (elevation) | `box-shadow` | `TRLV_ELEVATION_*` | `shadow_*` |
| Interactive states / variants | `:active`, `:focus-visible`, `:checked`, `:disabled` | theme state styles | `LV_STATE_*` selectors |
| Component | component | `<name>_create(parent, …)` (project) | function returning `lv_obj_t *` |
