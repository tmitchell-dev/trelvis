# Getting started

This page is the short path from a project that builds TreLVis to a first
screen that is themed, works in light and dark, and can be used with a
keypad. It is written for an engineer or an AI agent new to TreLVis. It
assumes you can already build an LVGL 9.6 project; for LVGL itself, use
LVGL's documentation (or, for an agent, LVGL's MCP server).

- [What to read, in order](#what-to-read-in-order)
- [Five kinds of call](#five-kinds-of-call)
- [Sizing: fit, fill and the parent's direction](#sizing-fit-fill-and-the-parents-direction)
- [A first screen](#a-first-screen)
- [What stays LVGL, and yours](#what-stays-lvgl-and-yours)

## What to read, in order

| Read | For | When |
|---|---|---|
| [README Quick start](../README.md#quick-start), steps 1–4 | Adding TreLVis, copying the token template, `trlv_init` | First |
| This page | The grammar, sizing, a whole screen, what LVGL still does | First |
| The headers you call: [`trlv_layout.h`](../include/trelvis/trlv_layout.h), [`trlv_surface.h`](../include/trelvis/trlv_surface.h), [`trlv_text.h`](../include/trelvis/trlv_text.h) | Exact signatures and edge cases | While writing the screen |
| [Designing a screen](designing-a-screen.md) | Which colour role and text role to use for what, the rules a screen is checked against, chips, banners and dialogs | Before your own screens |
| [`trlv_theme.h`](../include/trelvis/trlv_theme.h) | Colour-mode switching, token getters, focus | When you need them |
| [Building a token set](building-a-token-set.md) | Your own colours, fonts and spacing | When you replace the template's values |
| [Concepts](concepts.md) | Why things work as they do, the style engine, the full mapping tables | Reference, not a prerequisite |

You don't need the library's sources, or the values in the token
template's `.c` file, to write a screen.

## Five kinds of call

The name says what a call does. Everything takes or returns a plain
`lv_obj_t *`, so raw LVGL calls mix freely with these.

| Kind | Example | Does |
|---|---|---|
| Setup | `trlv_init(disp, &tokens_light, &tokens_regular)` | Installs the tokens and the theme. Call once, before creating objects. |
| Constructor, `_create` | `trlv_row_create(parent)` | Makes a box, row, column, spacer, divider or text. Containers start with no gap and no padding, fit their content, and don't scroll or take clicks. |
| Layout setter, `trlv_set_*` | `trlv_set_gap(row, TRLV_SPACE_SM)` | Sets one layout property from a token. Never changes colour. |
| Appearance, `trlv_apply_*` | `trlv_apply_card(card, TRLV_CARD_FILLED)` | Applies a look (surface, card, button type, text role). Never changes layout. |
| Getter, `trlv_get_*` | `trlv_get_space(TRLV_SPACE_LG)` | A token value for a raw LVGL call. |

Setters and applies work on any object and replace their previous value,
so calling one again never stacks styles. LVGL widgets (`lv_button_create`,
`lv_switch_create`, …) are used as they are: the theme styles them when
they are created.

## Sizing: fit, fill and the parent's direction

What "fill" does depends on the parent's direction, as Figma's "fill
container" does. The decision is made when you call the setter, so give
the parent its direction first: `trlv_row_create` and
`trlv_column_create` already have one.

| Call | Child of a row | Child of a column | Child of a box or other object |
|---|---|---|---|
| `trlv_set_width_fill` | Grows into the row's free width, shared with other growing children (CSS `flex: 1`) | 100 % of the column's content width (CSS `align-self: stretch`) | 100 % |
| `trlv_set_height_fill` | 100 % of the row's content height | Grows into the column's free height | 100 % |
| `trlv_set_width_fit`, `trlv_set_height_fit` | Fits its content (CSS `fit-content`). The default for TreLVis containers and text. | Same | Same |
| `lv_obj_set_size` | A fixed size, in px | Same | Same |

Three consequences catch people out:

- **Growing needs free space.** A child that grows along a parent that
  fits its content gets 0, and TreLVis logs a warning. Give the parent a
  fixed size or fill it. (Stretching across the axis works in a parent
  that fits: the child takes the width of the widest sibling.)
- **Text wraps only once it has a width:** `trlv_set_width_fill` on the
  label, or a fixed width.
- **Padding is inside the size**, and so is a border, like CSS
  `box-sizing: border-box`. `trlv_set_pad` sets all four sides;
  `trlv_set_pad_horizontal` and `trlv_set_pad_vertical` change one axis.

## A first screen

A settings page: a title, a card holding a dark-mode switch, and a button.
It picks the layout tokens from the display width, works with a keypad,
and switches light and dark while running.
[`tests/layout/test_getting_started.c`](../tests/layout/test_getting_started.c)
builds this exact code at 800 × 480 and 320 × 240 and checks its layout,
focus and colours.

```c
#include <trelvis/trelvis.h>
#include "tokens.h"                     /* your copy of the token template */

static void dark_mode_changed(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target_obj(e);
    trlv_theme_set_color_mode(lv_obj_has_state(sw, LV_STATE_CHECKED) ? &tokens_dark : &tokens_light);
}

static lv_obj_t * settings_create(lv_obj_t * parent)
{
    lv_obj_t * page = trlv_column_create(parent);
    trlv_set_gap(page, TRLV_SPACE_LG);
    trlv_set_pad(page, TRLV_SPACE_LG);
    trlv_set_width_fill(page);
    trlv_set_height_fill(page);
    lv_obj_set_scrollable(page, true);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);

    trlv_text_create(page, "Settings", TRLV_TEXT_HEADLINE_SMALL);

    /* A card that is a row: text on the left, a switch on the right. */
    lv_obj_t * card = trlv_row_create(page);
    trlv_set_gap(card, TRLV_SPACE_LG);
    trlv_set_pad(card, TRLV_SPACE_LG);
    trlv_set_width_fill(card);
    trlv_set_align_items(card, TRLV_ALIGN_ITEMS_CENTER);
    trlv_apply_card(card, TRLV_CARD_FILLED);

    lv_obj_t * text = trlv_column_create(card);
    trlv_set_gap(text, TRLV_SPACE_2XS);
    trlv_set_width_fill(text);                       /* grows: the switch goes to the end */
    trlv_text_create(text, "Dark mode", TRLV_TEXT_TITLE_MEDIUM);
    lv_obj_t * hint = trlv_text_create(text, "Easier on the eyes at night.", TRLV_TEXT_BODY_MEDIUM);
    trlv_set_width_fill(hint);                       /* a width makes text wrap */
    trlv_apply_text_emphasis(hint, TRLV_TEXT_EMPHASIS_LOW);

    lv_obj_t * sw = lv_switch_create(card);
    lv_obj_add_event_cb(sw, dark_mode_changed, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t * btn = lv_button_create(page);
    trlv_apply_button_type(btn, TRLV_BUTTON_TONAL);
    trlv_text_create(btn, "Restart", TRLV_TEXT_LABEL_LARGE);

    return page;
}

static void screen_init(lv_display_t * disp, lv_indev_t * keypad)
{
    bool compact = lv_display_get_horizontal_resolution(disp) < 480;
    trlv_init(disp, &tokens_light, compact ? &tokens_compact : &tokens_regular);

    lv_group_t * group = lv_group_create();
    lv_group_set_default(group);                     /* before the widgets: they join it */
    lv_indev_set_group(keypad, group);

    settings_create(lv_display_get_screen_active(disp));
}
```

What it shows:

- **The layout tokens are fixed at `trlv_init`.** On a small display, use
  the template's Compact set for tighter spacing and smaller text. The
  colour tokens can change at any time, with `trlv_theme_set_color_mode`.
- **Text takes its colour from its surface.** The card sets `on_surface`
  for the labels in it, and `trlv_apply_text_emphasis` gives the hint the
  lower-emphasis colour. Both follow the switch to dark.
- **Room for the focus ring.** The ring is drawn outside a widget, and a
  parent clips what lies outside it. The switch sits inside the card's
  `LG` padding and the button inside the page's, so neither ring is cut
  off. A row of controls with no padding of its own would clip them
  ([Designing a screen, rule 6](designing-a-screen.md#rules-for-writing-a-screen)).
- **No pixel numbers and no colours** in the screen: spacing comes from
  `TRLV_SPACE_*`, colours from the card, the text roles and the theme.

For a list, components that recur (a status chip, a banner) and dialogs,
see [Composing what has no primitive](designing-a-screen.md#composing-what-has-no-primitive)
and the [demo](../examples/demo).

## What stays LVGL, and yours

TreLVis covers layout, tokens and looks. These stay plain LVGL or your own
code, and are where a first screen usually goes wrong:

- **Display, input, tick and timers:** set them up as for any LVGL project.
  TreLVis ships no simulator; a host build uses LVGL's own drivers (for
  example `LV_USE_SDL`).
- **Keypad focus:** make a group the default *before* creating the
  widgets, as above, and give it to the input device. `LV_KEY_NEXT` and
  `LV_KEY_PREV` move the focus; `LV_KEY_ENTER` presses the focused widget;
  other keys, the arrows included, go to the focused widget (a slider
  moves). LVGL's SDL keyboard sends Tab as `LV_KEY_NEXT` and Enter as
  `LV_KEY_ENTER`; Space is an ordinary character and presses nothing.
- **Scrolling:** TreLVis containers don't scroll. Turn it on with
  `lv_obj_set_scrollable` and `lv_obj_set_scroll_dir`. When keypad focus
  moves, LVGL scrolls the widget into view through every scrolling
  parent, with an animation. To scroll from code, or before taking a
  snapshot, use `lv_obj_scroll_to_view_recursive(obj, LV_ANIM_OFF)`:
  `lv_obj_scroll_to_view` scrolls only the widget's direct parent.
- **The top layer is not part of the screen.** A dialog on
  `lv_layer_top()` is left out of a snapshot of the screen
  ([Reviewing a screen](designing-a-screen.md#reviewing-a-screen)).
- **Glyphs:** a character the font doesn't have is drawn as a box. The
  fonts LVGL builds in cover printable ASCII, `°`, `•` and the
  `LV_SYMBOL_*` icons; for anything else (such as `‹` or `·`), check that
  your font has it.
- **Behaviour and state:** events, enabling and disabling
  (`lv_obj_set_state(obj, LV_STATE_DISABLED, true)`), values, screens and
  navigation are your application's.
