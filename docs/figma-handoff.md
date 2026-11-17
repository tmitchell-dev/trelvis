# Figma hand-off conventions

These conventions let a Figma design translate almost mechanically into
TreLVis code. They follow the **Material 3 Figma kit** and **Material Theme
Builder**, so designers work with familiar tools and names.

Each rule exists because LVGL behaves a certain way. Breaking one usually
means the design can't be built exactly, or needs hand-written exceptions.
The [demo](../examples/demo) shows most of them on real screens.

## Setup

- **Start from the Material 3 Figma kit.** Generate the colour scheme (light,
  dark, plus any custom colours the product needs) with **Material Theme
  Builder**, from the product's seed colour.
- **Custom colours are the product's own list**: success and warning, extra
  brand accents, or alarm priorities. Agree the list and its order with the
  developers; it becomes an enum in code. Each one has the four Material
  Theme Builder values (colour, on colour, container, on container) in every
  colour mode. Up to four are supported by default; more need a
  configuration change, so agree the count early.
- **Alarm colours come from the standard, not the brand.** For a process HMI
  (ISA-101, ISA-18.2) or a medical device (IEC 60601-1-8), take the alarm
  colours from the site's alarm philosophy or the standard, turn off
  *harmonize* for them in Material Theme Builder, keep them the same in
  every colour mode, and use them for alarms only. Check that MD3 `error`
  (invalid fields) is not the same colour as an alarm priority. Never show a
  state by colour alone: pair it with text, an icon or a shape.
  `templates/trlv_tokens_hmi.c` is a worked example of such a scheme.
- **Draw frames at 1×, in real display pixels**, with one frame size per layout
  mode (e.g. 480×320 for compact, 800×480 for regular). There is no scaling
  step between Figma and the display.

## Variables

- MD3 colour roles go in a **`Color`** collection, with modes Light / Dark / …
  These become `trlv_color_tokens_t` values and can be switched at runtime.
  Every mode needs every role: the token structs have one fixed shape.
- Spacing, shape, sizes, elevation and the type scale go in a **`Layout`**
  collection, with modes Compact / Regular / … These become
  `trlv_layout_tokens_t` values, chosen at start-up. A product usually ships
  one layout mode.
- Names convert directly to token names:

  | Figma variable | TreLVis |
  |---|---|
  | `Surface Container High` | `trlv_color()->surface_container_high` |
  | `On Primary Container` | `trlv_color()->on_primary_container` |
  | `space/md` | `TRLV_SPACE_MD` |
  | `radius/md` (shape corner Medium) | `TRLV_RADIUS_MD` |
  | `size/md` (control height) | `TRLV_SIZE_MD` |
  | Elevation level 1 | `TRLV_ELEVATION_1` |
  | Body Medium text style | `TRLV_TEXT_BODY_MEDIUM` |

- The spacing scale is `none`, `2xs`, `xs`, `sm`, `md`, `lg`, `xl`, `2xl`,
  `3xl`. The token template's values for both layout modes are in
  [concepts.md §4.2](concepts.md#42-token-template-and-reference-values);
  start from them so code and design agree before anything is tuned.

## Layout

- **Use auto layout everywhere.** Horizontal and vertical auto layout map to
  `trlv_row` / `trlv_column`, and "hug" / "fill" / "fixed" map to
  `trlv_hug_*` / `trlv_fill_*` / `lv_obj_set_size`. Grid auto layout maps to
  LVGL's own grid API, so prefer rows and columns where they do the job.
- **Spacing sits on the 4 px grid** and is bound to `space/*` variables only.
  Gap and padding are one value on all sides in TreLVis; different
  horizontal and vertical values are possible but are raw LVGL in the code.
- **An empty frame set to fill** pushes what follows to the far end (a
  spacer, `trlv_spacer`).
- **Fill needs a parent with a size.** A child set to fill along the
  direction of a parent that hugs has nowhere to grow, as in Figma, which
  doesn't allow it.
- **"Space between" needs free space**: the frame must be fixed or fill,
  not hug.
- **Wrap**: auto layout wrap is supported, but children set to fill don't
  wrap in LVGL. Wrapping items hug or have a fixed width.
- **Text layers:** "auto width" text hugs and never wraps; "auto height"
  (fixed or fill width) wraps. Use auto height for anything that can be
  longer than its space, such as paragraphs and translated strings.
- Use absolute positioning only where it's genuinely needed (overlays,
  badges). It maps to LVGL's `lv_obj_set_floating` plus `lv_obj_align`.
- **Leave room for the focus ring.** On products with a keypad or encoder,
  the focused widget gets a ring outside its bounds (3 px thick and 2 px
  away in Regular, 2 px and 2 px in Compact). LVGL clips everything to its
  parent's bounds, as if every Figma frame had "Clip content" on, so any
  frame whose edge a focusable widget touches needs at least that much
  padding: use `space/sm`, which the template sizes for it in both modes.
  Siblings need no extra gap.

## Type

- Use **MD3 type-scale styles only**: Display / Headline / Title / Body /
  Label × Large / Medium / Small.
- Give line height in **px**, not percent.
- The type scale belongs to the layout mode: a Compact design may use
  smaller sizes and tighter lines for the same roles. Keep one set of text
  styles per layout mode, or bind font size and line height to `Layout`
  variables.
- Text takes its colour from the surface it sits on (the `on_` role); don't
  colour text by hand.
- Use only the agreed fonts, as listed in the project's role → font map.
  Several roles usually share one compiled font.
- Roles mapped to **restricted character sets** (typically Display: digits and
  units only) may only show the agreed characters. The role → font map lists
  them.
- **Numeric readouts** use fonts with tabular (fixed-width) digits.

## Containers and surfaces

- Show hierarchy with **surface container roles** (tone rather than shadow):
  Surface, Container Lowest … Container Highest. Compact layouts usually have
  no shadows at all.
- Cards are **Filled, Elevated or Outlined** only (MD3 card types), with
  corner radius Medium. Their padding is explicit: usually `space/lg`.
  Cards are not clickable in v1; put an action in them as a button.
- Always pair a container colour with its `on_` content colour.
- Custom colours fill a surface in two strengths: the colour itself (an
  alarm banner, a status badge) or its container (a calmer background).
- **Dividers** are MD3 dividers: a stroke-width line in Outline Variant,
  the full width of their parent, or inset with a margin.
- **Lists** are a vertical stack of rows separated by dividers: leading
  icon, headline and supporting text, trailing control or value. List rows
  themselves are not clickable in v1; the trailing control (a switch, a
  chevron button) carries the action.

## Buttons and controls

- Buttons are **Filled, Tonal, Outlined or Text** (MD3 button types). The
  Elevated button type is not in v1. Shape (full), height (`size/md`) and the
  Label Large text style come from the theme.
- Built-in controls (switch, checkbox, slider, progress bar, arc, spinner,
  dropdown, text field) use the Material 3 kit's versions as drawn; the
  theme gives them the MD3 look. Other components need a project design.

## Strokes, corners and effects

- **Strokes: inside only**, with "include strokes in layout" turned on. LVGL
  does the same: the border sits inside the object's size and insets the
  content by its width, on top of the padding (CSS `box-sizing: border-box`).
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

## What has no TreLVis equivalent

These are possible in Figma but not in LVGL, or not in v1. Avoid them, or
agree a workaround with the developers:

| Figma | In LVGL / TreLVis |
|---|---|
| Per-corner radii, corner smoothing | One radius per object |
| Centre or outside strokes (other than the focus ring) | Inside strokes only |
| Background blur, blend modes | Not on MCU targets |
| Fill inside a hugging frame | Collapses; give the parent a size |
| Filling items in a wrapping row | Don't wrap; hug or fix their width |
| Hover states, Elevated button, clickable cards and list rows | Not in v1 |
| Motion specs and transitions | Not in v1 (planned) |
| Line height in percent | Use px |

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
| Empty frame, fill | `flex: 1` | `trlv_spacer` | box + `flex_grow` |
| Wrap | `flex-wrap` | *raw LVGL* | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP)` |
| Absolute position | `position: absolute` | *raw LVGL* | `lv_obj_set_floating` + `lv_obj_align` |
| Colour role | `--md-sys-color-*` | `trlv_color()->…` | `lv_color_t` |
| Surface container | `background` + `color` | `trlv_surface` | `bg_color` + inherited `text_color` |
| Custom colour | `--md-extended-color-*` | `trlv_surface_custom` | `bg_color` + inherited `text_color` |
| Card | — | `trlv_card` | shared card style |
| Divider | `hr` | `trlv_divider` | box + `bg_color` |
| Button type | button class | `trlv_button_variant` | shared styles with state selectors |
| Type style | `font` / `line-height` | `trlv_text` / `trlv_text_role` | `text_font` / `text_line_space` |
| Corner radius | `border-radius` | `TRLV_RADIUS_*` | `radius` |
| Inside stroke | `border` | cards, fields | `border_*` |
| Focus ring | `outline` | theme | `outline_*` |
| Elevation | `box-shadow` | `TRLV_ELEVATION_*` | `shadow_*` |
| Variant: Pressed / Checked / Disabled / Focused | `:active` / `:checked` / `:disabled` / `:focus-visible` | theme | `LV_STATE_*` |

See [concepts.md](concepts.md#10-mapping-figma--md3--css--trelvis--lvgl) for
the full table.
