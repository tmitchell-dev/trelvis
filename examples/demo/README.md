# TreLVis demo

A demo of the whole library, in sections: a top bar with a light/dark switch
and one button per section, and the selected section below it. It is plain
C, `trlv_demo_create(parent, light, dark)`, so any host project can show it
in its own simulator or on its target. TreLVis ships no simulator.

| Section  | File                   | Shows |
|----------|------------------------|-------|
| Readouts | `trlv_demo_readouts.c` | A screen built from boxes, rows, columns and text: fill and hug, wrapping text, a wrapping row of buttons |
| Surfaces | `trlv_demo_surfaces.c` | Every MD3 surface and container role and the custom colours, with text taking the `on_` colour by inheritance |
| Cards    | `trlv_demo_cards.c`    | Filled, elevated and outlined cards with a title, wrapping body text and actions packed at the end |
| Controls | `trlv_demo_controls.c` | The four button types and every widget the theme covers (switch, checkbox, slider, bar, arc, spinner, dropdown, textarea), default, checked and disabled |
| List     | `trlv_demo_list.c`     | A settings list built from rows and dividers, the replacement for `lv_list`: icons, supporting text, trailing switches, values and chevrons |

The frame (bar, navigation and content area) is in `trlv_demo.c`. It is built
from rows, columns and buttons, not `lv_tabview`, `lv_list` or `lv_menu`.
Only the selected section exists; choosing another deletes it and builds the
new one, so the demo needs memory for its largest section only. With
everything on show it uses about 40 KB of LVGL's memory pool, Controls being
the largest, so it runs with LVGL's default 64 KB `LV_MEM_SIZE`.

The switch calls `trlv_theme_set_color_mode` with the dark or light tokens.
Everything recolours, built-in widgets included, with no code in the
sections. Leave out the switch by passing `NULL` for `dark`.

Each section is also a function of its own, `trlv_demo_<section>_create(parent)`,
that builds a column filling the width of `parent`. Use it to show one
section on a screen of its own.

The demo uses the token template unedited. The template gives the text roles
LVGL's Montserrat fonts in whichever sizes `lv_conf.h` enables, so the type
hierarchy is only as rich as the sizes you enable.

## Calling it from a host project

Build the demo's sources and the token template together with your sources,
next to TreLVis and LVGL:

```cmake
file(GLOB TRLV_DEMO_SOURCES CONFIGURE_DEPENDS external/trelvis/examples/demo/*.c)
target_sources(my_sim PRIVATE
    ${TRLV_DEMO_SOURCES}
    external/trelvis/templates/trlv_tokens_template.c)
target_include_directories(my_sim PRIVATE
    external/trelvis/examples/demo
    external/trelvis/templates)
target_link_libraries(my_sim PRIVATE trelvis)
```

Then, after creating the display:

```c
#include "trlv_demo.h"
#include "trlv_tokens_template.h"

trlv_init(disp, &tokens_light, &tokens_regular);
trlv_demo_create(lv_display_get_screen_active(disp), &tokens_light, &tokens_dark);
```

If your project already has its own copy of the token template, pass your
tokens instead and leave out `templates/`.

The demo fills the object it is given, and the content area scrolls
vertically when a section is taller than the display. It is laid out for
320 px wide and up.

## Keypad and encoder

The demo's buttons, switches, checkboxes, sliders, arcs and fields join
LVGL's default group as they are created, as any LVGL widget does. To move
through them with a keypad or encoder, make a group the default and give it
to the input device before calling `trlv_demo_create`:

```c
lv_group_t * group = lv_group_create();
lv_group_set_default(group);
lv_indev_set_group(keypad, group);
```

The focused widget shows the theme's focus ring, in both colour modes. Every
row that holds widgets is padded `TRLV_SPACE_SM`, so the ring, drawn outside
the widget, is never clipped (see "Focus rings need clearance" in
[docs/concepts.md](../../docs/concepts.md)).

## Tests

[`tests/layout/test_demo.c`](../../tests/layout/test_demo.c) builds the demo on
800 × 480 and 320 × 480 test displays. It checks the frame, moving between
sections (and that the one left is freed), each section's layout, that
nothing overflows its parent, the light/dark switch recolouring every
section, keypad focus reaching every enabled widget with the focus ring, and
room for every ring. CI keeps the demo and the library in step.
