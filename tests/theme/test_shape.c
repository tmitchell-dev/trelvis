/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * The covered widgets get their shape from the TreLVis theme, not only
 * their colours, so they keep it without a parent theme (#131). Built twice:
 * with the default parent, and against a copy of the library compiled with
 * TRLV_THEME_PARENT=TRLV_THEME_PARENT_NONE (see tests/CMakeLists.txt).
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#include <string.h>

#define ASSERT_TRUE(cond) \
    trlv_test_check_int(__FILE__, __LINE__, #cond, 1, (cond) ? 1 : 0)

#define FULL  (tokens_regular.radius[TRLV_RADIUS_FULL])
#define RADIUS(obj, part) lv_obj_get_style_radius(obj, part)

static lv_obj_t * init(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    return screen;
}

/* A size LVGL's default theme gives in dpx, on this display. */
static int32_t dpx(lv_obj_t * obj, int32_t n)
{
    return LV_DPX_CALC(lv_display_get_dpi(lv_obj_get_display(obj)), n);
}

static void assert_pad_all(lv_obj_t * obj, lv_part_t part, int32_t pad)
{
    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(obj, part), pad);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_bottom(obj, part), pad);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(obj, part), pad);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_right(obj, part), pad);
}

/* The build is the one intended: a plain lv_obj's padding comes from the
 * parent theme alone. */
static void theme_parent_is_as_built(lv_obj_t * screen)
{
    lv_obj_t * obj = lv_obj_create(init(screen));

#if TRLV_THEME_PARENT == TRLV_THEME_PARENT_NONE
    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(obj, LV_PART_MAIN), 0);
#else
    ASSERT_TRUE(lv_obj_get_style_pad_left(obj, LV_PART_MAIN) > 0);
#endif
}

static void switch_is_round(lv_obj_t * screen)
{
    lv_obj_t * sw = lv_switch_create(init(screen));

    TRLV_ASSERT_INT(RADIUS(sw, LV_PART_MAIN), FULL);
    TRLV_ASSERT_INT(RADIUS(sw, LV_PART_INDICATOR), FULL);
    TRLV_ASSERT_INT(RADIUS(sw, LV_PART_KNOB), FULL);
}

/* The indicator is round when the switch is off too: disabled, it is
 * drawn. */
static void disabled_switch_is_round(lv_obj_t * screen)
{
    lv_obj_t * sw = lv_switch_create(init(screen));
    lv_obj_add_state(sw, LV_STATE_DISABLED);

    TRLV_ASSERT_INT(RADIUS(sw, LV_PART_INDICATOR), FULL);
}

static void checkbox_has_a_gap_and_a_mark(lv_obj_t * screen)
{
    lv_obj_t * cb = lv_checkbox_create(init(screen));

    TRLV_ASSERT_INT(lv_obj_get_style_pad_column(cb, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_SM]);
    assert_pad_all(cb, LV_PART_INDICATOR, dpx(cb, 3));
    ASSERT_TRUE(lv_obj_get_style_bg_image_src(cb, LV_PART_INDICATOR) == NULL);

    lv_obj_add_state(cb, LV_STATE_CHECKED);
    const void * mark = lv_obj_get_style_bg_image_src(cb, LV_PART_INDICATOR);
    ASSERT_TRUE(mark != NULL && strcmp(mark, LV_SYMBOL_OK) == 0);
}

static void slider_has_a_round_knob(lv_obj_t * screen)
{
    lv_obj_t * slider = lv_slider_create(init(screen));

    TRLV_ASSERT_INT(RADIUS(slider, LV_PART_MAIN), FULL);
    TRLV_ASSERT_INT(RADIUS(slider, LV_PART_INDICATOR), FULL);
    TRLV_ASSERT_INT(RADIUS(slider, LV_PART_KNOB), FULL);
    assert_pad_all(slider, LV_PART_KNOB, dpx(slider, 6));
}

static void bar_is_round(lv_obj_t * screen)
{
    lv_obj_t * bar = lv_bar_create(init(screen));

    TRLV_ASSERT_INT(RADIUS(bar, LV_PART_MAIN), FULL);
    TRLV_ASSERT_INT(RADIUS(bar, LV_PART_INDICATOR), FULL);
}

static void assert_arc_shape(lv_obj_t * arc)
{
    TRLV_ASSERT_INT(lv_obj_get_style_arc_width(arc, LV_PART_MAIN), dpx(arc, 15));
    TRLV_ASSERT_INT(lv_obj_get_style_arc_width(arc, LV_PART_INDICATOR), dpx(arc, 15));
    ASSERT_TRUE(lv_obj_get_style_arc_rounded(arc, LV_PART_MAIN));
    ASSERT_TRUE(lv_obj_get_style_arc_rounded(arc, LV_PART_INDICATOR));
}

static void arc_is_drawn(lv_obj_t * screen)
{
    lv_obj_t * arc = lv_arc_create(init(screen));

    assert_arc_shape(arc);
    TRLV_ASSERT_INT(RADIUS(arc, LV_PART_KNOB), FULL);
    assert_pad_all(arc, LV_PART_KNOB, dpx(arc, 6));
}

static void spinner_is_drawn(lv_obj_t * screen)
{
    assert_arc_shape(lv_spinner_create(init(screen)));
}

/* MD3 outlined field: space LG at the sides, space SM at the top and
 * bottom. */
static void assert_field_padding(lv_obj_t * field)
{
    TRLV_ASSERT_INT(lv_obj_get_style_pad_top(field, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_SM]);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_bottom(field, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_SM]);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_left(field, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_LG]);
    TRLV_ASSERT_INT(lv_obj_get_style_pad_right(field, LV_PART_MAIN), tokens_regular.space[TRLV_SPACE_LG]);
}

static void fields_have_padding(lv_obj_t * screen)
{
    init(screen);

    assert_field_padding(lv_dropdown_create(screen));
    assert_field_padding(lv_textarea_create(screen));
}

int main(void)
{
    TRLV_TEST_RUN(theme_parent_is_as_built);
    TRLV_TEST_RUN(switch_is_round);
    TRLV_TEST_RUN(disabled_switch_is_round);
    TRLV_TEST_RUN(checkbox_has_a_gap_and_a_mark);
    TRLV_TEST_RUN(slider_has_a_round_knob);
    TRLV_TEST_RUN(bar_is_round);
    TRLV_TEST_RUN(arc_is_drawn);
    TRLV_TEST_RUN(spinner_is_drawn);
    TRLV_TEST_RUN(fields_have_padding);
    return trlv_test_finish();
}
