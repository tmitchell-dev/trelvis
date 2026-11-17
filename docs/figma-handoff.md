# Figma hand-off conventions

These conventions let a Figma design translate almost mechanically into
TreLVis code. They follow the **Material 3 Figma kit** and **Material Theme
Builder**, so designers work with familiar tools and names.

Each rule exists because LVGL behaves a certain way. Breaking one usually
means the design can't be built exactly, or needs hand-written exceptions.

## Setup

- **Start from the Material 3 Figma kit.** Generate the colour scheme (light,
  dark, plus any custom colours the product needs) with **Material Theme
  Builder**, from the product's seed colour.
- **Custom colours are the product's own list**: success and warning, extra
  brand accents, or alarm priorities. Agree the list and its order with the
  developers; it becomes an enum in code. Each one has the four Material
  Theme Builder values (colour, on colour, container, on container) in every
  colour mode.
- **Leave room for the focus ring.** On products with a keypad or encoder,
  the focused widget gets a ring outside its bounds (3 px thick, 2 px away
  in the template). Any frame that directly holds focusable widgets needs
  at least that much padding (use `space/sm`), or the ring is cut off: LVGL
  clips children to their parent, unlike Figma without "Clip content".
- **Alarm colours come from the standard, not the brand.** For a process HMI
  (ISA-101, ISA-18.2) or a medical device (IEC 60601-1-8), take the alarm
  colours from the site's alarm philosophy or the standard, turn off
  *harmonize* for them in Material Theme Builder, and use them for alarms
  only. Check that MD3 `error` (invalid fields) is not the same colour as an
  alarm priority. Never show a state by colour alone: pair it with text, an
  icon or a shape.
- **Draw frames at 1×, in real display pixels**, with one frame size per layout
  mode (e.g. 480×320 for compact, 800×480 for regular). There is no scaling
  step between Figma and the display.

## Variables

- MD3 colour roles go in a **`Color`** collection, with modes Light / Dark / …
  These become `trlv_color_tokens_t` values and can be switched at runtime.
- Spacing, shape and sizes go in a **`Layout`** collection, with modes
  Compact / Regular / … These become `trlv_layout_tokens_t` values, chosen at
  start-up.
- Names convert directly to token names:

  | Figma variable | TreLVis |
  |---|---|
  | `Surface Container High` | `trlv_color()->surface_container_high` |
  | `On Primary Container` | `trlv_color()->on_primary_container` |
  | `space/md` | `TRLV_SPACE_MD` |
  | `radius/md` (shape corner Medium) | `TRLV_RADIUS_MD` |

## Layout

- **Use auto layout everywhere.** Horizontal and vertical auto layout map to
  `trlv_row` / `trlv_column`, and "hug" / "fill" / "fixed" map to
  `trlv_hug_*` / `trlv_fill_*` / `lv_obj_set_size`. Grid auto layout maps to
  LVGL's own grid API, so prefer rows and columns where they do the job.
- **Spacing sits on the 4 px grid** and is bound to `space/*` variables only.
- Use absolute positioning only where it's genuinely needed (overlays,
  badges). It maps to LVGL's `lv_obj_set_floating` plus `lv_obj_align`.

## Type

- Use **MD3 type-scale styles only**: Display / Headline / Title / Body /
  Label × Large / Medium / Small.
- Give line height in **px**, not percent.
- Use only the agreed fonts, as listed in the project's role → font map.
  Several roles usually share one compiled font.
- Roles mapped to **restricted character sets** (typically Display: digits and
  units only) may only show the agreed characters. The role → font map lists
  them.
- **Numeric readouts** use fonts with tabular (fixed-width) digits.

## Containers and surfaces

- Show hierarchy with **surface container roles** (tone rather than shadow):
  Surface, Container Lowest … Container Highest.
- Cards are **Filled, Elevated or Outlined** only (MD3 card types).
- Always pair a container colour with its `on_` content colour.

## Strokes, corners and effects

- **Strokes: inside only**, with "include strokes in layout" turned on. LVGL
  draws borders inside the padding box.
- **Outside strokes are reserved for the focus ring** (LVGL `outline`).
- **No centre strokes.**
- **Corner radius** comes from the MD3 shape scale and is uniform per object.
  LVGL has one radius per object: no per-corner radii, no corner smoothing.
- **Effects:** elevation levels only. No background blur on MCU targets;
  avoid blend modes and heavy gradients.

## States

- Use **MD3 state layers** (pressed and focus; hover is not built in v1) and the MD3
  disabled treatment. Designers don't hand-pick pressed colours; TreLVis
  computes them from the tokens.
- Component state variants are named **Default / Pressed / Checked / Disabled /
  Focused**, which match LVGL states.

## Components

- Component **property names = `_create()` argument names**. A Figma
  component `Stat card` with properties `label` and `value` becomes
  `stat_card_create(parent, label, value)`.
- Mark **stateful widgets** (own logic or state, e.g. a live readout)
  distinctly from pure components. See [Vocabulary](concepts.md#2-vocabulary).

## Assets

- Use **one icon set**. Material Symbols is recommended.
- Export images at **1×**, named `icon_*` / `img_*`.

## Mapping table

| Figma / MD3 | CSS | TreLVis | LVGL |
|---|---|---|---|
| Frame (no auto layout) | `div` | `trlv_box` | `lv_obj`, reset style |
| Auto layout → / ↓ | `flex-direction: row / column` | `trlv_row` / `trlv_column` | flex layout |
| Gap | `gap` | `trlv_gap` | `pad_row` / `pad_column` |
| Padding | `padding` | `trlv_pad` | `pad_*` |
| Space between / packed | `justify-content` | `trlv_justify` | `flex_main_place` |
| Cross-axis alignment | `align-items` | `trlv_align_items` | `flex_cross_place` + `flex_track_place` |
| Hug / Fill / Fixed | `fit-content` / `flex: 1` / `width` | `trlv_hug_*` / `trlv_fill_*` / `lv_obj_set_size` | `LV_SIZE_CONTENT` / `flex_grow` or `LV_PCT(100)` / `width` |
| Absolute position | `position: absolute` | *raw LVGL* | `lv_obj_set_floating` + `lv_obj_align` |
| Colour role | `--md-sys-color-*` | `trlv_color()->…` | `lv_color_t` |
| Surface container | `background` + `color` | `trlv_surface` | `bg_color` + inherited `text_color` |
| Card | — | `trlv_card` | shared card style |
| Type style | `font` / `line-height` | `trlv_text` / `trlv_text_role` | `text_font` / `text_line_space` |
| Corner radius | `border-radius` | `TRLV_RADIUS_*` | `radius` |
| Inside stroke | `border` | cards, fields | `border_*` |
| Focus ring | `outline` | theme | `outline_*` |
| Elevation | `box-shadow` | `TRLV_ELEVATION_*` | `shadow_*` |
| Variant: Pressed / Checked / Disabled / Focused | `:active` / `:checked` / `:disabled` / `:focus-visible` | theme | `LV_STATE_*` |

See [concepts.md](concepts.md#10-mapping-figma--md3--css--trelvis--lvgl) for
the full table.
