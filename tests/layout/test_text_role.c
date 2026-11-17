/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * trlv_text_role: the role style on any object. On a container it reaches
 * every label inside through inheritance. Uses the token template's fonts.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#define ASSERT_TRUE(cond) \
    trlv_test_check_int(__FILE__, __LINE__, #cond, 1, (cond) ? 1 : 0)

/* Two roles with different fonts in the template, so a test can tell them
 * apart; checked in every test. */
#define ROLE_A TRLV_TEXT_HEADLINE_SMALL
#define ROLE_B TRLV_TEXT_LABEL_SMALL

static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    ASSERT_TRUE(tokens_regular.text[ROLE_A].font != tokens_regular.text[ROLE_B].font);
    return screen;
}

static const lv_font_t * font_of(lv_obj_t * obj)
{
    return lv_obj_get_style_text_font(obj, LV_PART_MAIN);
}

static const lv_font_t * role_font(trlv_text_role_t role)
{
    return tokens_regular.text[role].font;
}

/* A plain label inside a container takes the container's role, at any depth. */
static void container_role_reaches_labels(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_t * row = trlv_row(column, TRLV_SPACE_SM);
    lv_obj_t * near = lv_label_create(column);
    lv_obj_t * deep = lv_label_create(row);
    trlv_text_role(column, ROLE_A);

    ASSERT_TRUE(font_of(near) == role_font(ROLE_A));
    ASSERT_TRUE(font_of(deep) == role_font(ROLE_A));
}

/* Line and letter spacing inherit with the font. */
static void spacing_is_inherited(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_t * label = lv_label_create(column);
    trlv_text_role(column, ROLE_A);

    const trlv_text_style_t * token = &tokens_regular.text[ROLE_A];
    int32_t line_space = token->line_height == 0 ? 0
                         : token->line_height - lv_font_get_line_height(token->font);
    TRLV_ASSERT_INT(lv_obj_get_style_text_line_space(label, LV_PART_MAIN), line_space);
    TRLV_ASSERT_INT(lv_obj_get_style_text_letter_space(label, LV_PART_MAIN), token->letter_spacing);
}

/* A label with a role of its own keeps it inside a container with another. */
static void own_role_wins_over_the_container(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_t * text = trlv_text(column, "Own", ROLE_B);
    trlv_text_role(column, ROLE_A);

    ASSERT_TRUE(font_of(text) == role_font(ROLE_B));
}

/* A second call replaces the first, on a container and on a trlv_text. */
static void a_second_call_replaces_the_first(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_t * label = lv_label_create(column);
    trlv_text_role(column, ROLE_A);
    trlv_text_role(column, ROLE_B);
    ASSERT_TRUE(font_of(label) == role_font(ROLE_B));

    lv_obj_t * text = trlv_text(screen, "Text", ROLE_A);
    trlv_text_role(text, ROLE_B);
    ASSERT_TRUE(font_of(text) == role_font(ROLE_B));
}

/* Replaced in place: a project style added after the role still wins when
 * the role is set again. */
static void replaces_in_place(lv_obj_t * screen)
{
    static const lv_style_const_prop_t project_props[] = {
        LV_STYLE_CONST_TEXT_FONT(LV_FONT_DEFAULT),
        LV_STYLE_CONST_PROPS_END
    };
    static LV_STYLE_CONST_INIT(project_style, project_props);

    lv_obj_t * label = lv_label_create(setup(screen));
    trlv_text_role(label, ROLE_A);
    lv_obj_add_style(label, &project_style, LV_PART_MAIN);
    trlv_text_role(label, ROLE_B);

    ASSERT_TRUE(font_of(label) == LV_FONT_DEFAULT);
}

/* On a button it replaces the theme's Label Large for the button's label. */
static void replaces_the_button_role(lv_obj_t * screen)
{
    lv_obj_t * btn = lv_button_create(setup(screen));
    lv_obj_t * label = lv_label_create(btn);
    ASSERT_TRUE(font_of(label) == role_font(TRLV_TEXT_LABEL_LARGE));

    trlv_text_role(btn, ROLE_A);
    ASSERT_TRUE(font_of(label) == role_font(ROLE_A));
}

/* The label's size follows the role's font. */
static void label_size_follows_the_role(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_t * label = lv_label_create(column);
    lv_label_set_text(label, "Ag");
    trlv_text_role(column, ROLE_A);

    TRLV_ASSERT_HEIGHT(label, lv_font_get_line_height(role_font(ROLE_A)));
}

/* A value outside the enum changes nothing. */
static void invalid_role_changes_nothing(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column(setup(screen), TRLV_SPACE_SM);
    lv_obj_t * label = lv_label_create(column);
    trlv_text_role(column, ROLE_A);
    trlv_text_role(column, (trlv_text_role_t)99);

    ASSERT_TRUE(font_of(label) == role_font(ROLE_A));
}

int main(void)
{
    TRLV_TEST_RUN(container_role_reaches_labels);
    TRLV_TEST_RUN(spacing_is_inherited);
    TRLV_TEST_RUN(own_role_wins_over_the_container);
    TRLV_TEST_RUN(a_second_call_replaces_the_first);
    TRLV_TEST_RUN(replaces_in_place);
    TRLV_TEST_RUN(replaces_the_button_role);
    TRLV_TEST_RUN(label_size_follows_the_role);
    TRLV_TEST_RUN(invalid_role_changes_nothing);
    return trlv_test_finish();
}
