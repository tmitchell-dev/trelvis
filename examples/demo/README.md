# TreLVis demo

One realistic screen built with TreLVis: a header, a list of readouts, a
section of wrapping body text and a row of buttons. It is a plain function,
`trlv_demo_create(parent)`, so any host project can show it in its own
simulator or on its target. TreLVis ships no simulator.

The demo uses the token template unedited, so every text role uses
`LV_FONT_DEFAULT` until a project gives the roles real fonts.

## Calling it from a host project

Build `trlv_demo.c` and the token template together with your sources, next
to TreLVis and LVGL:

```cmake
target_sources(my_sim PRIVATE
    external/trelvis/examples/demo/trlv_demo.c
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

The demo fills the object it is given and scrolls vertically when that object
is too short. It is laid out for 320 px wide and up.

## Tests

[`tests/layout/test_demo.c`](../../tests/layout/test_demo.c) builds the demo on
800 × 480 and 320 × 480 test displays and checks its layout, so CI keeps the
screen and the library in step.
