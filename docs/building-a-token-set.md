# Building a token set

A TreLVis project owns its token values, starting from the token template
([quick start](../README.md#3-copy-the-token-template)). This page is a
reference for changing them: what each token is, what in TreLVis reads it,
and what TreLVis assumes about it. It is written so that an engineer, or
an AI agent working for them, can build a token set for any product, from
a brand-led consumer device to an industrial display.

It doesn't say what makes a good palette. Choosing the values is the
project's design decision, or the job of a tool such as Material Theme
Builder. The project is also responsible for whether its values meet any
standard it follows. For how to use the roles on a screen, see
[Designing a screen](designing-a-screen.md). The types themselves are in
`include/trelvis/trlv_tokens.h`, and the template's reference values in
[concepts §4.2](concepts.md#42-token-template-and-reference-values).

- [How the tokens are organised](#how-the-tokens-are-organised)
- [Colour roles](#colour-roles)
- [Custom colours](#custom-colours)
- [Layout tokens](#layout-tokens)
- [Examples](#examples)
- [Checking a token set](#checking-a-token-set)

## How the tokens are organised

| Collection | Type | Modes | Chosen |
|---|---|---|---|
| Color | `trlv_color_tokens_t` | light, dark, a brand scheme, … | at run time: `trlv_init`, then `trlv_theme_set_color_mode` |
| Layout | `trlv_layout_tokens_t` | regular, compact, … | once, at `trlv_init` |

- **Every field in every mode.** The structs have one fixed shape, and a
  colour mode replaces every colour at once. A field left at zero is
  black (a colour) or 0 px (a size). Only fields whose doc comment says so
  are safe at zero.
- **Role names are MD3's** in snake_case (*Surface Container High* is
  `surface_container_high`), so Material Theme Builder and the Material 3
  Figma kit map onto them one-to-one. A scheme doesn't have to look like
  Material, though: any values fit.
- **The template uses designated initialisers.** Keep them, so a field
  added to TreLVis later can't shift your values.

## Colour roles

### What reads each role

Changing a role changes everything listed against it.

| Role | Read by |
|---|---|
| `primary`, `on_primary` | Filled buttons (and their pressed and focus layers); the indicators of bars, sliders, switches and arcs; checked checkboxes; the border of a focused text field or dropdown; the content of outlined and text buttons. LVGL's default theme, for widgets TreLVis doesn't style, is seeded with `primary`. |
| `secondary` | Seeds LVGL's default theme, for widgets TreLVis doesn't style. |
| `secondary_container`, `on_secondary_container` | Tonal buttons; the selected option of a dropdown list; `TRLV_SURFACE_SECONDARY_CONTAINER`. |
| `primary_container`, `tertiary_container`, `error_container`, with their `on_` roles | Their `TRLV_SURFACE_*` values only. |
| `surface`, `on_surface` | Screens; outlined cards; text fields and dropdowns (background and text); the disabled look of every widget; the content colour of every surface container. |
| `on_surface_variant` | `trlv_apply_text_emphasis(…LOW)`; the checkbox box; placeholder text. |
| `surface_dim`, `surface_bright` | `TRLV_SURFACE_DIM`, `TRLV_SURFACE_BRIGHT`. |
| `surface_container_lowest` … `_highest` | Their `TRLV_SURFACE_*` values. Also `_low`: elevated cards; `surface_container`: dropdown lists; `_highest`: filled cards and the tracks of switches, sliders and bars. |
| `inverse_surface`, `inverse_on_surface` | `TRLV_SURFACE_INVERSE`. |
| `outline` | Field borders; outlined buttons; the switch's track border and its knob when off. |
| `outline_variant` | Dividers; outlined cards. |
| `shadow` | Elevation shadows (elevated cards, dropdown lists). |
| `focus` | The focus ring (keypad and encoder). Not an MD3 role: the template sets it to `secondary`. |
| `state` | Pressed and focus state layers (pre-mixed with the container) and the disabled opacities. Keep MD3's values unless there is a reason to change them. |

**Read by nothing in TreLVis:** `on_secondary`, `tertiary`,
`on_tertiary`, `error`, `on_error`, `inverse_primary` and `scrim`. They
are there for a project's own components ([Designing a
screen](designing-a-screen.md#recipes) uses `scrim`); changing them
changes no TreLVis look.

### What TreLVis assumes

- **`on_` pairs are used together.** `trlv_apply_surface` sets a container
  and its `on_` colour, and labels inside inherit the content colour. So
  every `on_` colour must be readable on its container, and `on_surface`
  and `on_surface_variant` on every surface and surface container.
- **The surface containers are distinct steps** within a mode, from
  `_lowest` to `_highest`, so nesting one container in another shows. They
  needn't be monotonic in lightness. A low-colour scheme may make
  `_lowest` lighter than `surface` and the rest darker.
- **`surface_dim` and `surface_bright` follow MD3's meaning:** in a dark
  scheme `surface_dim` equals `surface`, and in a light scheme
  `surface_bright` does. So neither is a darker or lighter step in both
  modes. For a recessed fill in both modes, use
  `surface_container_highest`, or set your own dark `surface_dim` below
  `surface`.
- **`focus` stays visible** on every container and custom colour it can
  sit on, at least 3:1 against `surface`.
- **`error` is for input and action errors** (an invalid field, a failed
  save). Keep it visibly different from any alarm or fault colour.

## Custom colours

The project's own colour families, in `custom[]`, indexed by its own enum.

| Field | Read by |
|---|---|
| `color`, `on_color` | `trlv_apply_surface_custom(obj, i, TRLV_CUSTOM_SURFACE_COLOR)`: the strong fill |
| `color_container`, `on_color_container` | `trlv_apply_surface_custom(obj, i, TRLV_CUSTOM_SURFACE_CONTAINER)`: the calm fill |

- **The same entries, in the same order, in every colour mode,** with the
  same `custom_count`. The enum is the index; TreLVis can't tell if two
  modes disagree on order.
- **Up to `TRLV_CUSTOM_COLOR_MAX` work with `trlv_apply_surface_custom`**
  (default 4, at most 8; raise it in `trlv_conf.h`). Entries beyond it can
  still be read with `trlv_get_color_tokens()`.
- **Fills only.** TreLVis applies custom colours as surfaces. To colour
  text or a line with one, use a project style and re-set it on
  colour-mode switches.
- **The template's four entries are examples**: `TOKEN_CUSTOM_FAULT`,
  `_WARNING`, `_ADVISORY` and `_BAD_DATA`. Their strong colours are the
  same in both modes and their containers follow the mode, and none is
  `error`, `primary` or `focus`. Rename, replace or delete them. For
  alarm colours, take the values from your standard or alarm philosophy.

## Layout tokens

| Field | Read by | Assumption |
|---|---|---|
| `space[]` | `trlv_set_gap`, `trlv_set_pad`, `trlv_get_space`; the theme's button padding (XL at the sides, SM between icon and text), field padding (SM, LG), checkbox gap (SM) and focus ring gap (2XS) | `space[TRLV_SPACE_SM]` is at least the focus ring's reach (`focus_width` plus its 2XS gap), so `TRLV_SPACE_SM` padding clears the ring. |
| `radius[]` | Cards (MD); buttons, switches and sliders (FULL); fields, checkboxes and dropdown lists (XS); `trlv_get_radius` | `TRLV_RADIUS_FULL` is `LV_RADIUS_CIRCLE`. |
| `border_width` | Outlined cards and buttons, fields, dividers, switch tracks, checkboxes | |
| `focus_width` | The focus ring; a focused field's border | At least `border_width`. |
| `control_height[]` | Buttons (MD), the switch track (SM); `trlv_get_control_height` | MD is the touch target height: 48 px or more for gloved touch. |
| `elevation[]` | Elevated cards (level 1), dropdown lists (level 2) | A layout mode may set every level to no shadow (`opa` 0), as Compact does. |
| `text[]` | `trlv_text_create`, `trlv_apply_text_role`; the screen (Body Medium) and buttons (Label Large) | Each role has a font that is compiled in. The template picks the nearest enabled Montserrat size and falls back to a smaller one; roles a project doesn't use cost nothing. `line_height` is in px; 0 keeps the font's own. |

## Examples

Each shows only the fields that change.

**Rename the status set** to a site's three alarm priorities, plus bad
data:

```c
/* tokens.h */
typedef enum {
    TOKEN_CUSTOM_ALARM_1,       /* was TOKEN_CUSTOM_FAULT */
    TOKEN_CUSTOM_ALARM_2,       /* was TOKEN_CUSTOM_WARNING */
    TOKEN_CUSTOM_ALARM_3,       /* was TOKEN_CUSTOM_ADVISORY */
    TOKEN_CUSTOM_BAD_DATA,
    TOKEN_CUSTOM_COUNT
} token_custom_color_t;
```

Then give `custom_light` and `custom_dark` the same four entries, in this
order, with the values from the alarm philosophy.

**A layout mode for gloved touch:** taller controls, and flat elevation.

```c
const trlv_layout_tokens_t tokens_gloved = {
    /* … the Regular values, then: */
    .control_height = {
        [TRLV_SIZE_SM] = 32,
        [TRLV_SIZE_MD] = 48,
        [TRLV_SIZE_LG] = 64,
    },
    .elevation = { { 0 } },     /* every level: no shadow */
};
```

**Colours from Material Theme Builder or a Figma variables export.**
- **Mapping:** each colour role's name converts to the field name in
  snake_case (`Surface Container High` → `.surface_container_high`).
- **Skip** `Background`, `On Background`, `Surface Tint`,
  `Surface Variant`, the `Fixed` roles and the state layers.
- **`focus`** has no source: keep `secondary`, or choose a role.
- **Custom colours** come four per colour (`Custom Color 1`,
  `On Custom Color 1`, `Custom Color 1 Container`,
  `On Custom Color 1 Container`), and map onto your enum by hand.

The [Figma hand-off](figma-handoff.md#from-figma-variables-to-tokensc) has
the full steps.

## Checking a token set

Before using a new set, check it against the assumptions above:

- [ ] Every field is set in every mode, with designated initialisers.
- [ ] Each `on_` colour is readable on its container, and `on_surface` and
      `on_surface_variant` on every surface (4.5:1 for text).
- [ ] The surface containers are distinct steps in each mode.
- [ ] `outline` and `focus` stand out from `surface` (3:1).
- [ ] Custom colours: the same entries in the same order in every mode,
      both pairs readable, none equal to `error`, `primary` or `focus`.
- [ ] Alarm and status values come from the applicable standard or alarm
      philosophy.
- [ ] `space[SM]` clears the focus ring; `control_height[MD]` suits the
      product's touch targets.
- [ ] Every text role has an enabled font.
- [ ] Both colour modes look right on the target display.

**The contrast check.** TreLVis's test suite has a small check,
`tests/tokens/token_check.c`, that runs the contrast, custom colour and
mode-count items above on the template. It reports problems and
enforces nothing. To run it on your own tokens, add
`external/trelvis/tests/tokens/token_check.c` and that directory's include
path to a host (simulator) build, link the maths library, and call it:

```c
#include "token_check.h"
#include "tokens.h"

int main(void)
{
    uint32_t problems = token_check_color_mode("tokens_light", &tokens_light)
                      + token_check_color_mode("tokens_dark", &tokens_dark)
                      + token_check_color_modes_match("tokens_light", &tokens_light,
                                                      "tokens_dark", &tokens_dark);
    return problems == 0 ? 0 : 1;
}
```

It can't check order, standards or how a design reads. External theme
tools, such as Material Theme Builder, check more, and a project may use
them instead.
