# TreLVis demo

A demo of the whole library, in sections: a top bar with one button per
section, and the selected section below it. It is plain C,
`trlv_demo_create(parent)`, so any host project can show it in its own
simulator or on its target. TreLVis ships no simulator.

| Section  | File                   | Shows |
|----------|------------------------|-------|
| Readouts | `trlv_demo_readouts.c` | A screen built from boxes, rows, columns and text: fill and hug, wrapping text, a wrapping row of buttons |

The frame (bar, navigation and content area) is in `trlv_demo.c`. It is built
from rows, columns and buttons, not `lv_tabview`, `lv_list` or `lv_menu`.
Only the selected section exists; choosing another deletes it and builds the
new one, so the demo needs memory for its largest section only.

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
trlv_demo_create(lv_display_get_screen_active(disp));
```

If your project already has its own copy of the token template, pass your
tokens instead and leave out `templates/`.

The demo fills the object it is given, and the content area scrolls
vertically when a section is taller than the display. It is laid out for
320 px wide and up.

## Tests

[`tests/layout/test_demo.c`](../../tests/layout/test_demo.c) builds the demo on
800 × 480 and 320 × 480 test displays. It checks the frame, moving between
sections, each section's layout, and that nothing overflows its parent, so
CI keeps the demo and the library in step.
