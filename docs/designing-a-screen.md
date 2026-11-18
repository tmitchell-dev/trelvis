# Designing a screen

This page is for whoever chooses how a screen looks: an engineer with no
designer, an AI agent writing a screen from these docs, or a designer
working without Figma. It is tool-neutral. With a Figma design, the
[Figma hand-off](figma-handoff.md) says how the same rules show up in
Figma. [Building a token set](building-a-token-set.md) covers the token
values themselves.

TreLVis's tokens and theme follow Material Design 3 (MD3), so this page
uses MD3's role names. You don't need to know MD3: the tables below say
which role to use for what.

- [Rules for writing a screen](#rules-for-writing-a-screen)
- [Which role for what](#which-role-for-what)
- [Layout](#layout)
- [Type](#type)
- [Status and alarm colours](#status-and-alarm-colours)
- [Composing what has no primitive](#composing-what-has-no-primitive)
- [Recipes](#recipes)
- [Reviewing a screen](#reviewing-a-screen)

## Rules for writing a screen

Each rule can be checked by reading the code.

1. **Tokens only.** Every spacing, radius and height comes from a token:
   a setter (`trlv_set_pad(obj, TRLV_SPACE_LG)`) or a getter in a raw
   call (`trlv_get_space(TRLV_SPACE_LG)`). No pixel numbers in screens,
   except fixed sizes the design fixes.
2. **Roles only.** Every colour comes from a role or a custom colour,
   through `trlv_apply_*` or the theme. No hex values in screens.
3. **Text takes its colour from its surface.** A surface sets the `on_`
   colour, and labels inside inherit it. Don't colour text by hand; for
   lower-emphasis text use `trlv_apply_text_emphasis`.
4. **Never show a state by colour alone.** Pair a colour with text, a
   shape, an icon or a position. About one in twelve men can't tell red
   from green.
5. **Fill needs a sized parent** along its axis: a parent that fits its
   content has no space to grow into ([Layout](#layout)).
6. **Leave room for the focus ring.** On a product with a keypad or
   encoder, a container whose edge a focusable widget touches needs at
   least `TRLV_SPACE_SM` padding.
7. **One radius per object, strokes inside.** LVGL has no per-corner
   radii, and borders sit inside the object's size.
8. **Live values need fixed-width digits** ([Type](#type)).
9. **Don't hide what blinks.** Hiding an object reflows a flex layout, so
   an indicator that comes and goes keeps its slot and fades instead.
10. **A colour that must follow light and dark goes in a shared style.**
    A local style (`lv_obj_set_style_*`) keeps the colour it was given
    ([concepts §7.5](concepts.md#75-getters-trlv_get_)).

## Which role for what

### Backgrounds and containers

Each step of nesting goes one step up the container ladder, so the tone
shows the structure without shadows.

| Purpose | Role | Notes |
|---|---|---|
| Page / screen background | `TRLV_SURFACE_SURFACE` | The theme sets it on screens. |
| Panel on the page | `TRLV_SURFACE_CONTAINER_LOW`, radius MD | Or `_LOWEST` for a panel lighter than the page. A Filled, Elevated or Outlined card if one fits. |
| Nested panel | one step up from its parent | |
| Header or footer bar | `TRLV_SURFACE_CONTAINER_HIGH` | Full width, no radius. |
| Menu, popup | `TRLV_SURFACE_CONTAINER` | What the theme uses for dropdown lists. |
| Modal dialog | `TRLV_SURFACE_CONTAINER_HIGH`, radius XL, over a scrim | [Recipe](#a-modal-over-a-scrim). |
| Selected item, chip | `TRLV_SURFACE_SECONDARY_CONTAINER` | Noticeable, not loud. |
| Emphasised area | `TRLV_SURFACE_PRIMARY_CONTAINER` | Rare in a low-colour product. |
| Input or action error | `TRLV_SURFACE_ERROR_CONTAINER` | Not for process alarms: those are custom colours. |
| Toast, tooltip | `TRLV_SURFACE_INVERSE` | |
| Recessed track or well | `TRLV_SURFACE_CONTAINER_HIGHEST` | Contrasts with panels in both modes. `TRLV_SURFACE_DIM` isn't darker in dark mode. |
| Status or alarm fill | custom colour | [Status and alarm colours](#status-and-alarm-colours). |

### Text

| Purpose | Text role | Emphasis |
|---|---|---|
| Large readout (a process value, a big number) | Headline Small up to Display Small, by screen size | normal |
| Screen title | Title Large | normal |
| Section title | Title Medium | normal |
| Item name, row label | Body Large | normal |
| Body text, description | Body Medium | normal |
| Tag, field label, caption | Label Medium | low |
| Unit after a value | one or two steps smaller than the value | low |
| Timestamp, small status | Body Small | low |
| Button text | Label Large (the theme sets it) | — |

Display and Headline are just the larger sizes; use them for readouts as
well as headings.

### Lines and strokes

| Purpose | Use |
|---|---|
| Divider between items | `trlv_divider_create` (`outline_variant`) |
| Border of an outlined control | the theme, or `TRLV_BUTTON_OUTLINED` (`outline`) |
| Ticks, hairline graphics | a project style with `outline` or `outline_variant`, re-set on colour-mode switches |

### In a low-colour product

Some products keep colour for abnormal states, such as a process HMI in
the high-performance style. There, `primary` is for the few actions that
need emphasis (a confirm button). Use tonal and outlined buttons for the
rest, and show equipment states with text and shape (a filled or hollow
marker), never red and green.

## Layout

- **Rows and columns first.** A box with no layout is for overlapping
  children only.
- **Spacing sits on the token scale** and is one value per side through
  `trlv_set_pad`. For different horizontal and vertical padding, use
  `lv_obj_set_style_pad_hor(obj, trlv_get_space(TRLV_SPACE_LG), 0)`.
- **Fill needs free space.** A child set to fill along the direction of a
  parent that fits its content collapses to 0, and TreLVis warns. Across
  the axis it stretches to the widest sibling (CSS `align-self: stretch`).
- **"Space between" needs free space** too: the parent must be fixed or
  fill, not fit its content.
- **Wrapping rows:** children set to fill don't wrap. Wrapping items fit
  their content or have a fixed width.
- **Text** fits its content and never wraps until it has a width
  (`trlv_set_width_fill` or a fixed width). Give anything that can be
  longer than its space a width, including translated strings.
- **Absolute position** only where it's needed (overlays, badges):
  `lv_obj_set_floating` plus `lv_obj_align`.
- **Focus ring:** see rule 6. Siblings need no extra gap.

## Type

- **Roles, not sizes.** Use the 15 text roles; [Which role for
  what](#text) maps purposes to them.
- **Fixed-width digits for live values.** A value that changes jitters
  sideways if its digits are proportional. Montserrat, the font the token
  template uses, has proportional digits: at 24 px a "1" is 9 px wide and
  a "0" 16 px. LVGL ships no font with tabular digits. Until the product
  has one, give the value label a fixed width that fits its widest value,
  and align its text right:

  ```c
  lv_obj_t * value = trlv_text_create(row, "37.9", TRLV_TEXT_HEADLINE_SMALL);
  lv_obj_set_width(value, 72);                                   /* widest value */
  lv_obj_set_style_text_align(value, LV_TEXT_ALIGN_RIGHT, 0);
  ```

- **Restricted character sets.** A role compiled with a few glyphs (such
  as digits and units for a large readout) may show only those.
- **Text boxes are shorter than in design tools.** LVGL adds line spacing
  only between lines, so a one-line Body Medium text is about 16 px tall,
  not 20 px.

## Status and alarm colours

Status colours are the project's custom colours. The token template has
four examples (fault, warning, advisory, bad data), to rename or replace.

- **Take the values from your standard or alarm philosophy**, not from the
  brand. TreLVis's examples claim compliance with no standard.
- **Two strengths.** `TRLV_CUSTOM_SURFACE_COLOR` is the strong fill (an
  active alarm, an unacknowledged banner). `TRLV_CUSTOM_SURFACE_CONTAINER`
  is the calm one (acknowledged, or a background). One call switches
  between them, and the text inside follows.
- **Keep them apart from `error`, `primary` and `focus`**, so a fault
  never reads as an input error or a selection.
- **Same in every colour mode** for the strong colour, so an alarm looks
  the same everywhere.
- **Never colour alone** (rule 4): pair each priority with a number,
  shape or word.
- **Blink with opacity, in a fixed slot** (rule 9): animate `opa` with
  `lv_anim`, rather than hiding and showing.
- Custom colours are fills only. To colour text or a line with one, use a
  project style and re-set it on colour-mode switches.

## Composing what has no primitive

TreLVis ships primitives, not components. Chips, badges, indicators,
gauges, dialogs and alarm banners are compositions of rows, columns,
roles and tokens, written once as a project `_create()` function. This is
the expected path, not a workaround.

A status chip: a row with a tone, a radius, padding wider than it is tall,
and a label.

```c
lv_obj_t * chip_create(lv_obj_t * parent, const char * text, uint32_t status)
{
    lv_obj_t * chip = trlv_row_create(parent);
    trlv_set_align_items(chip, TRLV_ALIGN_ITEMS_CENTER);
    trlv_set_pad(chip, TRLV_SPACE_XS);
    lv_obj_set_style_pad_hor(chip, trlv_get_space(TRLV_SPACE_MD), 0);
    lv_obj_set_style_radius(chip, trlv_get_radius(TRLV_RADIUS_FULL), 0);
    trlv_apply_surface_custom(chip, status, TRLV_CUSTOM_SURFACE_CONTAINER);
    trlv_text_create(chip, text, TRLV_TEXT_LABEL_MEDIUM);
    return chip;
}
```

The same pattern builds a range bar (a track in
`TRLV_SURFACE_CONTAINER_HIGHEST`, a band and a pointer placed with
`lv_obj_set_floating` and `lv_obj_align`), or a state marker (a small
circle, filled with `on_surface` when running and hollow with a border
when stopped).

## Recipes

### A rounded panel in any tone

```c
trlv_apply_surface(panel, TRLV_SURFACE_CONTAINER_LOWEST);
lv_obj_set_style_radius(panel, trlv_get_radius(TRLV_RADIUS_MD), 0);
```

If the tone is a card's, use the card instead:
`trlv_apply_card(panel, TRLV_CARD_FILLED)`.

### A modal over a scrim

The scrim is a full-screen box on the top layer, in `scrim` at MD3's
32 %, clickable so it blocks input behind it:

```c
lv_obj_t * scrim = trlv_box_create(lv_layer_top());
lv_obj_set_size(scrim, LV_PCT(100), LV_PCT(100));
lv_obj_set_style_bg_color(scrim, trlv_get_color_tokens()->scrim, 0);
lv_obj_set_style_bg_opa(scrim, 82, 0);                 /* 32 % */
lv_obj_set_clickable(scrim, true);

lv_obj_t * dialog = trlv_column_create(scrim);
lv_obj_center(dialog);
trlv_set_gap(dialog, TRLV_SPACE_LG);
trlv_set_pad(dialog, TRLV_SPACE_XL);
trlv_apply_surface(dialog, TRLV_SURFACE_CONTAINER_HIGH);
lv_obj_set_style_radius(dialog, trlv_get_radius(TRLV_RADIUS_XL), 0);
```

The scrim's colour is a local style, so a dialog that stays open across a
colour-mode switch keeps the old colour; `scrim` is black in most schemes,
so this rarely shows.

## Reviewing a screen

- **Outline every container:** build with `TRLV_USE_DEBUG_OUTLINE 1` in
  `trlv_conf.h`.
- **Look at both colour modes**, and at the smallest display the product
  supports.
- **Screenshots without a person at the window.** With `LV_USE_SNAPSHOT 1`
  in `lv_conf.h`, a host build can save any object as a BMP. Step through
  the screen's states from a timer, saving one image per state, and review
  the images. This is how an agent checks its own visual decisions.

  ```c
  #include <stdio.h>

  static void put_u32(FILE * f, uint32_t v)
  {
      for(int i = 0; i < 4; i++) fputc((int)((v >> (8 * i)) & 0xFF), f);
  }

  /* Save obj as a 24-bit BMP. Host builds only. */
  bool save_bmp(lv_obj_t * obj, const char * path)
  {
      lv_obj_update_layout(obj);
      lv_draw_buf_t * buf = lv_snapshot_take(obj, LV_COLOR_FORMAT_RGB888);
      if(buf == NULL) return false;

      uint32_t w = buf->header.w;
      uint32_t h = buf->header.h;
      uint32_t row = (w * 3 + 3) & ~3u;            /* BMP rows are 4-byte aligned */
      FILE * f = fopen(path, "wb");
      if(f == NULL) {
          lv_draw_buf_destroy(buf);
          return false;
      }

      fputc('B', f);
      fputc('M', f);
      put_u32(f, 54 + row * h);                    /* file size */
      put_u32(f, 0);
      put_u32(f, 54);                              /* offset of the pixels */
      put_u32(f, 40);                              /* info header size */
      put_u32(f, w);
      put_u32(f, h);
      put_u32(f, 1 | (24 << 16));                  /* 1 plane, 24 bits per pixel */
      put_u32(f, 0);                               /* no compression */
      put_u32(f, row * h);
      put_u32(f, 2835);                            /* 72 dpi */
      put_u32(f, 2835);
      put_u32(f, 0);
      put_u32(f, 0);

      static const uint8_t pad[3] = { 0 };
      for(uint32_t y = h; y-- > 0;) {              /* BMP rows go bottom-up */
          fwrite(buf->data + y * buf->header.stride, 1, w * 3, f);
          fwrite(pad, 1, row - w * 3, f);
      }
      fclose(f);
      lv_draw_buf_destroy(buf);
      return true;
  }
  ```

  LVGL's RGB888 is stored blue, green, red, the order BMP expects.
