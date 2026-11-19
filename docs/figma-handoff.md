# Figma hand-off conventions

These conventions let a Figma design translate almost mechanically into
TreLVis code. They follow the **Material 3 Figma kit** and **Material Theme
Builder**, so designers work with familiar tools and names.

This page is about Figma: how the design rules show up in Figma's panels,
the kit and the variables. The rules themselves are tool-neutral and live
in [Designing a screen](designing-a-screen.md): the rules for writing a
screen, which role to use for what, layout, type, and status and alarm
colours. Read that first. The token values are covered in
[Building a token set](building-a-token-set.md).

Each convention exists because LVGL behaves a certain way. Breaking one
usually means the design can't be built exactly, or needs hand-written
exceptions. The [demo](../examples/demo) shows most of them on real
screens.

## Setup

- **Start from the Material 3 Figma kit.** Generate the colour scheme (light,
  dark, plus any custom colours the product needs) with **Material Theme
  Builder**, from the product's seed colour. Both are free in Figma
  Community (search for them by name):
  - *Material 3 Design Kit* is a Community **file**. Open it and choose
    *Open in Figma* (or *Duplicate*) to get your own editable copy. Its
    text styles, components and variables exist only inside that copy.
  - *Material Theme Builder* is a Community **plugin**. Run it inside your
    copy of the kit (*Plugins* in the main menu or the toolbar). It creates
    the colour scheme as variables, and can swap the kit's components onto
    it.
- **Contrast modes:** Theme Builder also generates medium and high contrast
  versions of each scheme. TreLVis doesn't use them. Ignore them, or ship
  one as an extra colour token set (another `trlv_color_tokens_t` for
  `trlv_theme_set_color_mode`).
- **Custom colours are the product's own list**: status colours, alarm
  priorities, or extra brand accents. Agree the list and its order with the
  developers; it becomes an enum in code. Each one has the four Material
  Theme Builder values (colour, on colour, container, on container) in every
  colour mode. Up to four are supported by default; more need a
  configuration change, so agree the count early.
- **Alarm and status colours come from the standard, not the brand**
  ([Designing a screen](designing-a-screen.md#status-and-alarm-colours)).
  In Material Theme Builder, turn off *harmonize* for them.
- **Draw frames at 1×, in real display pixels**, with one frame size per layout
  mode (e.g. 480×320 for compact, 800×480 for regular). There is no scaling
  step between Figma and the display.
- **Design in the product's real font.** The kit's text styles use Roboto;
  the token template uses the nearest Montserrat size LVGL ships
  ([concepts §4.2](concepts.md#42-token-template-and-reference-values)). If the
  product will keep Montserrat, switch the kit's text styles to it.
  Otherwise give the template the product's font. A side-by-side comparison
  with mismatched fonts looks wrong for reasons unrelated to layout.

### A first screen: the minimal path

The rest of this document describes the full set-up. A first screen or a
prototype needs only:

1. A copy of the kit, with a **Light** scheme from Material Theme Builder.
2. A `Layout` collection holding at least the `space/*` variables (see
   [Variables](#variables); the kit doesn't include it).
3. One frame at the display's size, built with auto layout.

It can skip the dark scheme, custom colours, contrast modes, a Compact
layout mode and the alarm-colour rules, and add them when the product
needs them.

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
  | `Surface Container High` | `trlv_get_color_tokens()->surface_container_high` |
  | `On Primary Container` | `trlv_get_color_tokens()->on_primary_container` |
  | `space/md` | `TRLV_SPACE_MD` |
  | `radius/md` (shape corner Medium) | `TRLV_RADIUS_MD` |
  | `size/md` (control height) | `TRLV_SIZE_MD` |
  | Elevation level 1 | `TRLV_ELEVATION_1` |
  | Body Medium text style | `TRLV_TEXT_BODY_MEDIUM` |

- The spacing scale is `none`, `2xs`, `xs`, `sm`, `md`, `lg`, `xl`, `2xl`,
  `3xl`. The token template's values for both layout modes are in
  [concepts.md §4.2](concepts.md#42-token-template-and-reference-values);
  start from them so code and design agree before anything is tuned.
- **The kit has no `Layout` variables.** It comes with colour variables
  only, so create the `Layout` collection (`space/*`, `radius/*`, `size/*`)
  by hand from the tables in concepts §4.2.
- **Names in the kit differ from the table above.** The kit's colour
  variables sit in a group called `Schemes` (`Schemes/Surface Container
  High`), and Material Theme Builder names its collection `material-theme`.
  Only the role name at the end matters: it converts to the token name in
  snake_case.

### From Figma variables to `tokens.c`

Until the token generator exists ([ROADMAP](../ROADMAP.md) "Later"), the
colour values are copied by hand into the project's copy of the token
template:

1. In *Local variables*, export the colour collection. Figma writes one
   `<Mode>.tokens.json` per mode (`Light.tokens.json`, `Dark.tokens.json`).
2. In each file, use the `Schemes` group. Each colour's `$value` has a
   `hex` field. Convert the Title Case name to snake_case to get the field
   (`Surface Container High` → `.surface_container_high`), and paste the
   value as `TOKEN_HEX(0x…)` in `tokens_light` / `tokens_dark`.
3. Skip roles TreLVis has no field for: `Background` and `On Background`
   (the same as Surface / On Surface), `Surface Tint`, `Surface Variant`,
   and every `Fixed`, `Fixed Dim` and `Fixed Variant` role.
4. Skip the `State Layers` group: TreLVis computes pressed and focus
   colours from the `.state` opacities.
5. **`focus` has no Figma source.** It is TreLVis's focus-ring colour;
   the template sets it to `secondary`. Keep that, or choose a role.
6. Custom colours are in the `Extended Colors` group, four per colour
   (`Custom Color 1`, `On Custom Color 1`, `Custom Color 1 Container`,
   `On Custom Color 1 Container`). Their names are the designer's, so map
   each one to the project's custom colour enum by hand.
7. Do every mode. The template's baseline differs from the kit's current
   scheme (for example `primary` `#6750A4` in the template, `#65558F` in
   the kit), so a mode that isn't imported is visibly out of step with
   the design.

## Layout

- **Use auto layout everywhere.** Horizontal and vertical auto layout map to
  `trlv_row_create` / `trlv_column_create`, and "hug" / "fill" / "fixed" map to
  `trlv_set_width_fit` / `trlv_set_width_fill` / `lv_obj_set_size` (and
  the `_height_` setters). Grid auto layout maps to
  LVGL's own grid API, so prefer rows and columns where they do the job.
- **Spacing sits on the 4 px grid** and is bound to `space/*` variables only.
  Gap is one value both ways in TreLVis. Padding can differ horizontally
  and vertically (`trlv_set_pad_horizontal`, `trlv_set_pad_vertical`);
  padding that differs on one side only, or a different row and column
  gap, is raw LVGL in the code.
  Figma starts new auto layout with a 10 px gap and padding, which is off
  the grid: bind both to a `space/*` variable (or 0) every time.
- **An empty frame set to fill** pushes what follows to the far end (a
  spacer, `trlv_spacer_create`).
- **Fill, "space between" and wrap** follow the
  [layout rules](designing-a-screen.md#layout): fill and "space between"
  need a frame that is fixed or fill, not hug, and children set to fill
  don't wrap.
- **Text layers:** "auto width" text hugs and never wraps; "auto height"
  (fixed or fill width) wraps. Use auto height for anything that can be
  longer than its space, such as paragraphs and translated strings.
- Use absolute positioning only where it's genuinely needed (overlays,
  badges). It maps to LVGL's `lv_obj_set_floating` plus `lv_obj_align`.
- **Leave room for the focus ring.** LVGL clips everything to its parent's
  bounds, as if every Figma frame had "Clip content" on. On products with
  a keypad or encoder, a frame whose edge a focusable widget touches needs
  `space/sm` padding, which the template sizes for the ring in both
  modes.

## Type

- Use **MD3 type-scale styles only**: Display / Headline / Title / Body /
  Label × Large / Medium / Small. In the kit, select a text layer and apply
  one with the *Apply styles* button in the *Typography*
  section of the right panel. The styles live in the kit, so this works in
  your copy of it. To set a style by hand, use MD3's values (Regular
  layout mode):

  | Role | Large | Medium | Small | Weight |
  |---|---|---|---|---|
  | Display | 57 / 64 | 45 / 52 | 36 / 44 | Regular |
  | Headline | 32 / 40 | 28 / 36 | 24 / 32 | Regular |
  | Title | 22 / 28 | 16 / 24 | 14 / 20 | Regular (Large), Medium |
  | Body | 16 / 24 | 14 / 20 | 12 / 16 | Regular |
  | Label | 14 / 20 | 12 / 16 | 11 / 16 | Medium |

  Size / line height in px. The token template maps each role to the
  nearest Montserrat size ([concepts §4.2](concepts.md#42-token-template-and-reference-values)).
- Give line height in **px**, not percent.
- **Text boxes are shorter in LVGL.** LVGL adds line spacing only between
  lines, while Figma centres each line in its line height. So a text box
  is shorter than in Figma by `line_height − font line height` (a one-line
  Body Medium text is about 16 px tall rather than 20 px), and its text sits
  slightly higher. The spacing between lines matches. Expect this when
  comparing screenshots; see [concepts §4.2](concepts.md#42-token-template-and-reference-values).
- **Lower-emphasis text** (On Surface Variant, for labels and supporting
  text) is `trlv_apply_text_emphasis(obj, TRLV_TEXT_EMPHASIS_LOW)`, on the
  text layer or on the frame around several.
- The type scale belongs to the layout mode: a Compact design may use
  smaller sizes and tighter lines for the same roles. Keep one set of text
  styles per layout mode, or bind font size and line height to `Layout`
  variables.
- Use only the agreed fonts, as listed in the project's role → font map.
  Several roles usually share one compiled font.
- The rest of the type rules (text colour from its surface, restricted
  character sets, fixed-width digits for live values) are in
  [Designing a screen](designing-a-screen.md#type).

## Containers and surfaces

- Choose surface roles by purpose
  ([Which role for what](designing-a-screen.md#which-role-for-what)), and
  show hierarchy by tone rather than shadow. Compact layouts usually have no
  shadows at all.
- Cards are **Filled, Elevated or Outlined** only (MD3 card types), with
  corner radius Medium, and padded `space/lg` as in the kit. In the code a
  card is a look applied to an auto layout frame (`trlv_apply_card`), and
  its padding is set like any frame's. Headers, media and actions are
  children; a card used with the same content in many places is a project
  component ([Components](#components)).
  Cards are not clickable in v1; put an action in them as a button.
- **A rounded tonal area is a card.** A surface container role on its own
  has no corner radius in TreLVis. For a rounded panel, use one of the
  card types (Elevated is Surface Container Low, and has no shadow in a
  layout mode without shadows). Any other tone with a radius is a surface
  plus `lv_obj_set_style_radius(obj, trlv_get_radius(TRLV_RADIUS_MD), 0)`.
- **Dividers** are MD3 dividers: a stroke-width line in Outline Variant,
  the full width of their parent, or inset with a margin.
- **Lists** are a vertical stack of rows separated by dividers: leading
  icon, headline and supporting text, trailing control or value. List rows
  themselves are not clickable in v1; the trailing control (a switch, a
  chevron button) carries the action.

## Buttons and controls

- Buttons are **Filled, Tonal, Outlined or Text** (MD3 button types). A
  button that stays on or off (the kit's selected / unselected toggle) is a
  **Filled, Tonal or Outlined toggle**. The Elevated button type is not in
  v1. Shape (full), height (`size/md`) and the
  Label Large text style come from the theme.
- Built-in controls (switch, checkbox, slider, progress bar, arc, spinner,
  dropdown, text field) use the Material 3 kit's versions as drawn; the
  theme gives them the MD3 look. Other components need a project design.
- Kit components and what they become:

  | Material 3 kit | TreLVis / LVGL |
  |---|---|
  | Buttons: Filled, Tonal, Outlined, Text | `lv_button` + `trlv_apply_button_type` |
  | Toggle buttons: Filled, Tonal, Outlined (selected / unselected) | a checkable `lv_button` (`lv_obj_set_checkable`) + `trlv_apply_button_type(btn, TRLV_BUTTON_*_TOGGLE)`; selected is `LV_STATE_CHECKED` |
  | Buttons: Elevated, FAB, icon buttons, segmented | Not in v1 (an icon button is an `lv_button` holding an icon) |
  | Cards: Filled, Elevated, Outlined | a row or column + `trlv_apply_card` + `trlv_set_pad` |
  | Text field: Outlined | `lv_textarea` (one line or more). The floating label in the border has no LVGL equivalent: put the label above the field as text. |
  | Text field: Filled | Not in v1; use Outlined |
  | Menu (with a field) | `lv_dropdown` |
  | Switch, Checkbox, Slider | `lv_switch`, `lv_checkbox`, `lv_slider` |
  | Progress indicators: linear, circular | `lv_bar`, `lv_arc` / `lv_spinner` |
  | Divider | `trlv_divider_create` |
  | Lists | Rows and dividers (see above) |
  | Chips, badges, navigation, dialogs, sheets, snackbars, tabs, date and time pickers | No MD3 style in v1: a project design, or an LVGL widget the theme doesn't cover ([concepts §6.4](concepts.md#64-widget-coverage-v1)) |

- **Field heights.** The kit draws fields at a fixed 56 px; LVGL sizes a
  textarea or dropdown from its font and padding (`space/sm` top and
  bottom, `space/lg` at the sides). If the design fixes the height, bind
  it to `size/*` and set it in code (`lv_obj_set_height` with
  `trlv_get_control_height(…)`). A one-line field at
  `size/md` fits a line of body text; the text sits at the top of the
  field, under the padding, not centred.

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

## Reading a Figma frame and writing the screen

The sections above are for the designer. This one is for the developer
turning a frame into a screen function.

1. **Read the frame in Figma's right panel.** Select each layer. *Auto
   layout* shows direction, gap, padding and alignment; *W* / *H* show
   Fixed, Hug or Fill. Values bound to variables appear as named pills:
   the pill's name is the token (`space/lg` → `TRLV_SPACE_LG`). A plain
   number where you expected a pill is a question for the designer. Dev
   Mode shows the same information but is optional (and a paid feature).
2. **Rename layers to code names** (`header`, `readouts`, `settings`), so
   the design and the code use the same words.
3. **Sketch the frame tree as a comment** at the top of the screen
   function, one line per frame with its auto layout settings:

   ```c
   /* page        column, gap lg, pad lg, fill × fill
    *   header    row, gap sm, fill width, space between
    *   readouts  row, gap md, fill width
    *     card ×3 column, Filled card, pad lg, gap xs, fill width
    *   ...
    */
   ```

   Then write the code top-down, one block per frame: the constructor,
   then one setter per auto layout property, then the styles the frame
   applies (a card, a surface, a text style). A reviewer, or a tool, can
   then check the code against the design frame by frame:

   ```c
   lv_obj_t * page = trlv_column_create(screen);
   trlv_set_gap(page, TRLV_SPACE_LG);
   trlv_set_pad(page, TRLV_SPACE_LG);
   trlv_set_width_fill(page);
   trlv_set_height_fill(page);

   lv_obj_t * card = trlv_column_create(readouts);
   trlv_set_gap(card, TRLV_SPACE_XS);
   trlv_set_pad(card, TRLV_SPACE_LG);
   trlv_set_width_fill(card);
   trlv_apply_card(card, TRLV_CARD_FILLED);
   ```

   A new row or column has no gap or padding, like a fresh frame set to 0.
   Layout is set (`trlv_set_*`, the auto layout panel); appearance is
   applied (`trlv_apply_*`), as you apply a colour or text style in
   Figma. [Concepts §7](concepts.md#7-the-api-by-kind) lists every
   function by kind.
4. **Translate each property with the [mapping table](#mapping-table).**
   Each auto layout property is one call. Set a container's direction (its
   constructor) before filling its children, as the fill modifiers depend
   on it. A component the designer made becomes a project `_create()`
   function ([Components](#components)).
5. **Where raw LVGL is needed:** fixed sizes (`lv_obj_set_size`, with
   `trlv_get_control_height(TRLV_SIZE_MD)` for a field bound to
   `size/md`), a corner radius on a non-card, a different row and column
   gap or one-side padding, wrap, absolute
   positions, and widget behaviour (options, one-line mode, accepted
   characters). Raw LVGL colours taken from `trlv_get_color_tokens()` don't follow a
   colour-mode switch; see [concepts §7.5](concepts.md#75-getters-trlv_get_).
6. **Compare.** Build with `TRLV_USE_DEBUG_OUTLINE 1` in `trlv_conf.h` to
   outline every container, and compare a screenshot with the frame
   exported at 1×. Expect text boxes to be a little shorter than Figma's
   ([Type](#type)) and fonts to differ unless the design uses the
   product's real font.

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
| Frame (no auto layout) | `div` | `trlv_box_create` | `lv_obj`, reset style |
| Auto layout → / ↓ | `flex-direction: row / column` | `trlv_row_create` / `trlv_column_create` | flex layout |
| Gap | `gap` | `trlv_set_gap` | `pad_row` / `pad_column` |
| Padding | `padding` | `trlv_set_pad` | `pad_*` |
| Horizontal / vertical padding | `padding-inline` / `padding-block` | `trlv_set_pad_horizontal` / `trlv_set_pad_vertical` | `pad_left/right`, `pad_top/bottom` |
| Space between / packed | `justify-content` | `trlv_set_justify` | `flex_main_place` |
| Cross-axis alignment | `align-items` | `trlv_set_align_items` | `flex_cross_place` + `flex_track_place` |
| Hug / Fill / Fixed | `fit-content` / `flex: 1` / `width` | `trlv_set_width_fit` / `trlv_set_width_fill` / `lv_obj_set_size` (and `_height_`) | `LV_SIZE_CONTENT` / `flex_grow` or `LV_PCT(100)` / `width` |
| Empty frame, fill | `flex: 1` | `trlv_spacer_create` | box + `flex_grow` |
| Wrap | `flex-wrap` | *raw LVGL* | `lv_obj_set_flex_flow(obj, LV_FLEX_FLOW_ROW_WRAP)` |
| Absolute position | `position: absolute` | *raw LVGL* | `lv_obj_set_floating` + `lv_obj_align` |
| Colour role | `--md-sys-color-*` | `trlv_get_color_tokens()->…` | `lv_color_t` |
| Surface container | `background` + `color` | `trlv_apply_surface` | `bg_color` + inherited `text_color` |
| Custom colour | `--md-extended-color-*` | `trlv_apply_surface_custom` | `bg_color` + inherited `text_color` |
| Card | — | `trlv_apply_card` | shared card style |
| Divider | `hr` | `trlv_divider_create` | box + `bg_color` |
| Button type | button class | `trlv_apply_button_type` | shared styles with state selectors |
| Type style | `font` / `line-height` | `trlv_text_create` / `trlv_apply_text_role` | `text_font` / `text_line_space` |
| Text fill *On Surface Variant* | lower-emphasis `color` | `trlv_apply_text_emphasis(obj, TRLV_TEXT_EMPHASIS_LOW)` | inherited `text_color` |
| Corner radius | `border-radius` | `TRLV_RADIUS_*` | `radius` |
| Inside stroke | `border` | cards, fields | `border_*` |
| Focus ring | `outline` | theme | `outline_*` |
| Elevation | `box-shadow` | `TRLV_ELEVATION_*` | `shadow_*` |
| Variant: Pressed / Checked / Disabled / Focused | `:active` / `:checked` / `:disabled` / `:focus-visible` | theme | `LV_STATE_*` |

See [concepts.md](concepts.md#10-mapping-figma--md3--css--trelvis--lvgl) for
the full table.
