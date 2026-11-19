/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/*
 * Appearance functions work on any object, as LVGL styles do, but warn when
 * the object is one the look makes no sense on: a card on an LVGL widget, a
 * surface on a button, a button type on an object that can't be pressed.
 * The look is applied all the same. The test build turns LVGL logging on;
 * warnings are caught with a log callback.
 */

#include "trlv_test.h"
#include "trlv_tokens_template.h"

#include <string.h>

#define ASSERT_COLOR(actual, expected) \
    trlv_test_check_int(__FILE__, __LINE__, #actual, (int32_t)lv_color_to_u32(expected), \
                        (int32_t)lv_color_to_u32(actual))

static int warnings;
static char last_warning[256];

static void log_cb(lv_log_level_t level, const char * buf)
{
    if(level == LV_LOG_LEVEL_WARN && strstr(buf, "trlv_apply_") != NULL) {
        warnings++;
        lv_strlcpy(last_warning, buf, sizeof(last_warning));
    }
}

/* lv_deinit clears the log callback, so it is registered in every test. */
static lv_obj_t * setup(lv_obj_t * screen)
{
    trlv_init(lv_obj_get_display(screen), &tokens_light, &tokens_regular);
    warnings = 0;
    last_warning[0] = '\0';
    lv_log_register_print_cb(log_cb);
    return screen;
}

static bool warning_names(const char * func, lv_obj_t * obj)
{
    char text[64];
    lv_snprintf(text, sizeof(text), "%s(%p)", func, (void *)obj);
    return strstr(last_warning, text) != NULL;
}

/* The intended targets don't warn. */
static void intended_targets_are_silent(lv_obj_t * screen)
{
    lv_obj_t * column = trlv_column_create(setup(screen));
    trlv_apply_card(column, TRLV_CARD_OUTLINED);
    trlv_apply_card(trlv_box_create(screen), TRLV_CARD_FILLED);
    trlv_apply_card(lv_obj_create(screen), TRLV_CARD_ELEVATED);

    trlv_apply_surface(trlv_row_create(screen), TRLV_SURFACE_CONTAINER);
    lv_obj_t * badge = trlv_text_create(screen, "3", TRLV_TEXT_LABEL_SMALL);
    trlv_apply_surface(badge, TRLV_SURFACE_ERROR_CONTAINER);
    trlv_apply_surface_custom(badge, TOKEN_CUSTOM_WARNING, TRLV_CUSTOM_SURFACE_COLOR);

    trlv_apply_button_type(lv_button_create(screen), TRLV_BUTTON_TONAL);
    lv_obj_t * list_row = trlv_row_create(screen);
    lv_obj_set_clickable(list_row, true);
    trlv_apply_button_type(list_row, TRLV_BUTTON_TEXT);

    TRLV_ASSERT_INT(warnings, 0);
}

/* A card on an LVGL widget warns, naming the object, and still applies. */
static void card_on_a_widget_warns(lv_obj_t * screen)
{
    lv_obj_t * btn = lv_button_create(setup(screen));
    trlv_apply_card(btn, TRLV_CARD_FILLED);

    TRLV_ASSERT_INT(warnings, 1);
    TRLV_ASSERT_INT(warning_names("trlv_apply_card", btn), true);
    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.surface_container_highest);

    lv_obj_t * label = trlv_text_create(screen, "Label", TRLV_TEXT_BODY_MEDIUM);
    trlv_apply_card(label, TRLV_CARD_OUTLINED);
    TRLV_ASSERT_INT(warnings, 2);
}

/* A surface or custom colour on a button warns: pressed and focus keep the
 * button's colours. It still applies the colour at rest. */
static void surface_on_a_button_warns(lv_obj_t * screen)
{
    lv_obj_t * btn = lv_button_create(setup(screen));
    trlv_apply_surface(btn, TRLV_SURFACE_ERROR_CONTAINER);

    TRLV_ASSERT_INT(warnings, 1);
    TRLV_ASSERT_INT(warning_names("trlv_apply_surface", btn), true);
    TRLV_ASSERT_INT(strstr(last_warning, "trlv_apply_button_type") != NULL, true);
    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.error_container);

    trlv_apply_surface_custom(btn, TOKEN_CUSTOM_WARNING, TRLV_CUSTOM_SURFACE_COLOR);
    TRLV_ASSERT_INT(warnings, 2);
    TRLV_ASSERT_INT(warning_names("trlv_apply_surface_custom", btn), true);
    ASSERT_COLOR(lv_obj_get_style_bg_color(btn, LV_PART_MAIN), tokens_light.custom[TOKEN_CUSTOM_WARNING].color);
}

/* A button type on an object that isn't clickable warns: its states never
 * show. The colours still apply. */
static void button_type_on_a_plain_row_warns(lv_obj_t * screen)
{
    lv_obj_t * row = trlv_row_create(setup(screen));
    trlv_apply_button_type(row, TRLV_BUTTON_TONAL);

    TRLV_ASSERT_INT(warnings, 1);
    TRLV_ASSERT_INT(warning_names("trlv_apply_button_type", row), true);
    ASSERT_COLOR(lv_obj_get_style_bg_color(row, LV_PART_MAIN), tokens_light.secondary_container);
}

/* An invalid value logs its own warning only, not a target warning too. */
static void invalid_value_warns_once(lv_obj_t * screen)
{
    lv_obj_t * btn = lv_button_create(setup(screen));
    trlv_apply_card(btn, TRLV_CARD_COUNT);

    TRLV_ASSERT_INT(warnings, 1);
    TRLV_ASSERT_INT(strstr(last_warning, "is not a trlv_card_type_t") != NULL, true);
}

int main(void)
{
    TRLV_TEST_RUN(intended_targets_are_silent);
    TRLV_TEST_RUN(card_on_a_widget_warns);
    TRLV_TEST_RUN(surface_on_a_button_warns);
    TRLV_TEST_RUN(button_type_on_a_plain_row_warns);
    TRLV_TEST_RUN(invalid_value_warns_once);
    return trlv_test_finish();
}
