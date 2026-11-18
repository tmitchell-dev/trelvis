/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_text_create and the text role styles. Sizes are computed from the font
 * with lv_text_get_size, so the tests don't depend on LV_FONT_DEFAULT's
 * metrics.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define LONG_TEXT "The quick brown fox jumps over the lazy dog, then naps in the sun."

/* Writable copy of the Regular layout, so the tests set their own text
 * tokens rather than depend on the template's. trlv_init keeps the pointer,
 * so it must outlive the test. */
static trlv_layout_tokens_t layout;

/* Body Medium in the default font, with the given line height and letter
 * spacing. */
static lv_obj_t * setup_with(lv_obj_t * screen, int32_t line_height, int32_t letter_spacing)
{
    layout = tokens_regular;
    layout.text[TRLV_TEXT_BODY_MEDIUM].font = LV_FONT_DEFAULT;
    layout.text[TRLV_TEXT_BODY_MEDIUM].line_height = line_height;
    layout.text[TRLV_TEXT_BODY_MEDIUM].letter_spacing = letter_spacing;
    trlv_init(lv_obj_get_display(screen), &tokens_light, &layout);
    return screen;
}

/* Body Medium in the default font, with its own line height. */
static lv_obj_t * setup(lv_obj_t * screen)
{
    return setup_with(screen, 0, 0);
}

static int32_t font_line_height(void)
{
    return lv_font_get_line_height(LV_FONT_DEFAULT);
}

/* Size of `text` in the default font, wrapped at `max_width`. */
static lv_point_t text_size(const char * text, int32_t letter_space, int32_t line_space, int32_t max_width)
{
    lv_point_t size;
    lv_text_get_size(&size, text, LV_FONT_DEFAULT, letter_space, line_space, max_width, LV_TEXT_FLAG_NONE);
    return size;
}

/* Fits its text by default, like any LVGL label (Figma "auto width"). */
static void fits_its_text(lv_obj_t * screen)
{
    lv_obj_t * text = trlv_text_create(setup(screen), LONG_TEXT, TRLV_TEXT_BODY_MEDIUM);
    lv_point_t size = text_size(LONG_TEXT, 0, 0, LV_COORD_MAX);

    TRLV_ASSERT_WIDTH(text, size.x);
    TRLV_ASSERT_HEIGHT(text, font_line_height());
}

/* A width from trlv_set_width_fill makes it wrap (Figma "auto height"). */
static void wraps_after_fill_width(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_set_gap(column, TRLV_SPACE_SM);
    lv_obj_set_width(column, 120);
    lv_obj_t * text = trlv_text_create(column, LONG_TEXT, TRLV_TEXT_BODY_MEDIUM);
    trlv_set_width_fill(text);
    lv_point_t size = text_size(LONG_TEXT, 0, 0, 120);

    TRLV_ASSERT_WIDTH(text, 120);
    TRLV_ASSERT_HEIGHT(text, size.y);
    TRLV_ASSERT_INT(trlv_test_height(text) > font_line_height(), true);
}

/* In a row, fill grows the text into the free space, and it wraps there. */
static void wraps_when_growing_in_a_row(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_set_gap(row, TRLV_SPACE_SM);
    lv_obj_set_width(row, 200);
    lv_obj_t * icon = trlv_box_create(row);
    lv_obj_set_size(icon, 40, 40);
    lv_obj_t * text = trlv_text_create(row, LONG_TEXT, TRLV_TEXT_BODY_MEDIUM);
    trlv_set_width_fill(text);
    lv_point_t size = text_size(LONG_TEXT, 0, 0, 200 - 40 - 8);

    TRLV_ASSERT_RECT(text, 40 + 8, 0, 200 - 40 - 8, size.y);
}

/* A Figma line height is converted to LVGL line spacing. */
static void line_height_is_converted(lv_obj_t * screen)
{
    int32_t line_height = font_line_height() + 6;
    lv_obj_t * text = trlv_text_create(setup_with(screen, line_height, 0), "one\ntwo", TRLV_TEXT_BODY_MEDIUM);

    TRLV_ASSERT_INT(lv_obj_get_style_text_line_space(text, LV_PART_MAIN), 6);
    TRLV_ASSERT_HEIGHT(text, 2 * font_line_height() + 6);
}

/* A line height smaller than the font's gives negative spacing. */
static void tighter_line_height(lv_obj_t * screen)
{
    int32_t line_height = font_line_height() - 2;
    lv_obj_t * text = trlv_text_create(setup_with(screen, line_height, 0), "one\ntwo", TRLV_TEXT_BODY_MEDIUM);

    TRLV_ASSERT_INT(lv_obj_get_style_text_line_space(text, LV_PART_MAIN), -2);
}

/* A line height of 0 keeps the font's own line height. */
static void zero_line_height_keeps_the_font(lv_obj_t * screen)
{
    lv_obj_t * text = trlv_text_create(setup(screen), "one\ntwo", TRLV_TEXT_BODY_MEDIUM);

    TRLV_ASSERT_INT(lv_obj_get_style_text_line_space(text, LV_PART_MAIN), 0);
    TRLV_ASSERT_HEIGHT(text, 2 * font_line_height());
}

/* Letter spacing comes from the token and widens the text it fits. */
static void letter_spacing_is_applied(lv_obj_t * screen)
{
    lv_obj_t * text = trlv_text_create(setup_with(screen, 0, 2), "Spacing", TRLV_TEXT_BODY_MEDIUM);
    lv_point_t size = text_size("Spacing", 2, 0, LV_COORD_MAX);

    TRLV_ASSERT_INT(lv_obj_get_style_text_letter_space(text, LV_PART_MAIN), 2);
    TRLV_ASSERT_WIDTH(text, size.x);
}

/* The role sets the token's font, over the font the parent passes down.
 * The token font is a copy of the default font: a valid font at a different
 * address, so the check can tell the two apart. */
static void role_sets_the_font(lv_obj_t * screen)
{
    static lv_font_t role_font;
    role_font = *LV_FONT_DEFAULT;
    layout = tokens_regular;
    layout.text[TRLV_TEXT_TITLE_LARGE].font = &role_font;
    trlv_init(lv_obj_get_display(screen), &tokens_light, &layout);

    lv_obj_t * box = trlv_box_create(screen);
    lv_obj_set_style_text_font(box, LV_FONT_DEFAULT, 0);
    lv_obj_t * text = trlv_text_create(box, "Font", TRLV_TEXT_TITLE_LARGE);

    TRLV_ASSERT_INT(lv_obj_get_style_text_font(text, LV_PART_MAIN) == &role_font, true);
}

/* Colour is inherited from the parent, not set by the role. */
static void colour_is_inherited(lv_obj_t * screen)
{
    lv_obj_t * box = trlv_box_create(setup(screen));
    lv_color_t red = lv_color_hex(0xff0000);
    lv_obj_set_style_text_color(box, red, 0);
    lv_obj_t * text = trlv_text_create(box, "Red", TRLV_TEXT_BODY_MEDIUM);

    TRLV_ASSERT_INT(lv_color_to_u32(lv_obj_get_style_text_color(text, LV_PART_MAIN)), lv_color_to_u32(red));
}

/* trlv_init again with new tokens updates existing text. */
static void reinit_updates_existing_text(lv_obj_t * screen)
{
    lv_obj_t * text = trlv_text_create(setup(screen), "one\ntwo", TRLV_TEXT_BODY_MEDIUM);
    TRLV_ASSERT_HEIGHT(text, 2 * font_line_height());

    setup_with(screen, font_line_height() + 4, 0);

    TRLV_ASSERT_HEIGHT(text, 2 * font_line_height() + 4);
}

/* Every one of the 15 roles has a style with its token's font, line
 * spacing and letter spacing (the template's values). */
static void every_role_has_its_style(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);

    for(int32_t i = 0; i < TRLV_TEXT_COUNT; i++) {
        const trlv_text_style_t * token = &tokens_regular.text[i];
        lv_obj_t * text = trlv_text_create(screen, "Role", (trlv_text_role_t)i);
        int32_t line_space = token->line_height == 0 ? 0
                             : token->line_height - lv_font_get_line_height(token->font);

        TRLV_ASSERT_INT(lv_obj_get_style_text_font(text, LV_PART_MAIN) == token->font, true);
        TRLV_ASSERT_INT(lv_obj_get_style_text_line_space(text, LV_PART_MAIN), line_space);
        TRLV_ASSERT_INT(lv_obj_get_style_text_letter_space(text, LV_PART_MAIN), token->letter_spacing);
    }
}

/* The conversion with a real font that isn't LV_FONT_DEFAULT (the test
 * build enables Montserrat 16): Body Large, 16 px text on a 24 px line. */
static void line_height_with_a_real_font(lv_obj_t * screen)
{
    const lv_font_t * font = &lv_font_montserrat_16;
    int32_t font_height = lv_font_get_line_height(font);
    layout = tokens_regular;
    layout.text[TRLV_TEXT_BODY_LARGE].font = font;
    layout.text[TRLV_TEXT_BODY_LARGE].line_height = 24;
    trlv_init(lv_obj_get_display(screen), &tokens_light, &layout);

    lv_obj_t * text = trlv_text_create(screen, "one\ntwo", TRLV_TEXT_BODY_LARGE);

    TRLV_ASSERT_INT(font != LV_FONT_DEFAULT, true);
    TRLV_ASSERT_INT(lv_obj_get_style_text_line_space(text, LV_PART_MAIN), 24 - font_height);
    TRLV_ASSERT_HEIGHT(text, 2 * font_height + (24 - font_height));
}

/* With the same real font, a line height of 0 keeps the font's own. */
static void zero_line_height_with_a_real_font(lv_obj_t * screen)
{
    const lv_font_t * font = &lv_font_montserrat_16;
    layout = tokens_regular;
    layout.text[TRLV_TEXT_BODY_LARGE].font = font;
    layout.text[TRLV_TEXT_BODY_LARGE].line_height = 0;
    trlv_init(lv_obj_get_display(screen), &tokens_light, &layout);

    lv_obj_t * text = trlv_text_create(screen, "one\ntwo", TRLV_TEXT_BODY_LARGE);

    TRLV_ASSERT_INT(lv_obj_get_style_text_line_space(text, LV_PART_MAIN), 0);
    TRLV_ASSERT_HEIGHT(text, 2 * lv_font_get_line_height(font));
}

/* An invalid role still gives a plain label, with a warning. */
static void invalid_role_gives_a_plain_label(lv_obj_t * screen)
{
    lv_obj_t * text = trlv_text_create(setup(screen), "Plain", TRLV_TEXT_COUNT);
    lv_point_t size = text_size("Plain", 0, 0, LV_COORD_MAX);

    TRLV_ASSERT_WIDTH(text, size.x);
}

int main(void)
{
    TRLV_TEST_RUN(fits_its_text);
    TRLV_TEST_RUN(wraps_after_fill_width);
    TRLV_TEST_RUN(wraps_when_growing_in_a_row);
    TRLV_TEST_RUN(line_height_is_converted);
    TRLV_TEST_RUN(tighter_line_height);
    TRLV_TEST_RUN(zero_line_height_keeps_the_font);
    TRLV_TEST_RUN(letter_spacing_is_applied);
    TRLV_TEST_RUN(role_sets_the_font);
    TRLV_TEST_RUN(colour_is_inherited);
    TRLV_TEST_RUN(reinit_updates_existing_text);
    TRLV_TEST_RUN(every_role_has_its_style);
    TRLV_TEST_RUN(line_height_with_a_real_font);
    TRLV_TEST_RUN(zero_line_height_with_a_real_font);
    TRLV_TEST_RUN(invalid_role_gives_a_plain_label);
    return trlv_test_finish();
}
