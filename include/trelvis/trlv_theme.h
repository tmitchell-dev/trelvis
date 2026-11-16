/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_theme.h
 * Initialisation, the MD3 theme for LVGL's built-in widgets, and run-time
 * colour-mode switching.
 *
 * TreLVis styles LVGL's widgets through LVGL's own theme engine; it never
 * wraps them. After trlv_init, lv_button_create() gives an MD3 filled
 * button and lv_switch_create() an MD3 switch. Widgets the theme doesn't
 * cover fall back to a parent theme seeded with the MD3 colours (see
 * TRLV_THEME_PARENT).
 *
 * Mapping: Figma / MD3 → CSS → TreLVis → LVGL mechanism
 *
 * | Figma / MD3                              | CSS                                  | TreLVis                     | LVGL mechanism                                         |
 * |------------------------------------------|--------------------------------------|-----------------------------|--------------------------------------------------------|
 * | Variable collections, starting modes     | custom properties on `:root`         | trlv_init                   | shared styles + lv_theme_t on the display              |
 * | Switch the Color collection's mode       | swap `--md-sys-color-*` values       | trlv_theme_set_color_mode   | re-set shared styles, lv_obj_report_style_change(NULL) |
 * | Read a colour variable                   | `var(--md-sys-color-primary)`        | trlv_color()->primary       | lv_color_t                                             |
 * | Read a layout variable                   | `var(--space-md)`                    | trlv_layout()->space[...]   | int32_t                                                |
 * | Component default styles                 | user-agent / component stylesheet    | the TreLVis theme           | theme apply callback, LV_PART_* / LV_STATE_* selectors |
 * | Interactive states                       | `:active`, `:focus-visible`, ...     | the TreLVis theme           | LV_STATE_PRESSED, LV_STATE_FOCUS_KEY, ...              |
 *
 * All functions must be called from the LVGL thread.
 */

#ifndef TRLV_THEME_H
#define TRLV_THEME_H

#include "trlv_tokens.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialise TreLVis with a project's tokens and install the MD3 theme on a
 * display. Figma: choose the starting mode of each variable collection.
 *
 * Call it once, after creating the display and **before creating any
 * objects on it**. LVGL themes style an object when it is created, so
 * objects that already exist keep their old theme styles. The display's
 * active screen already exists, so trlv_init restyles it while it is still
 * empty; any styles a project added to it before trlv_init are removed.
 *
 * What it does:
 *
 * 1. Keeps the token pointers. The structs are not copied, so they must
 *    stay valid for as long as TreLVis is used, which the `const` globals
 *    from the token template do.
 * 2. Writes the token values into TreLVis's shared styles (gap, pad, text
 *    roles, surfaces, cards, buttons, state layers). These styles use fixed,
 *    static memory and allocate nothing.
 * 3. With TRLV_USE_THEME 1 (the default), initialises the parent theme
 *    (TRLV_THEME_PARENT) and the TreLVis theme and sets it on `disp` with
 *    lv_display_set_theme. LVGL allocates the theme objects here, once.
 *    With TRLV_USE_THEME 0, no theme is installed: tokens, layout helpers
 *    and the shared styles work, and widgets keep whatever theme the
 *    display has.
 *
 * The theme styles the screen with `surface`, `on_surface` and the Body
 * Medium text role, so every label inherits them, and styles the covered
 * widgets (docs/concepts.md section 6.4) with their MD3 colours, shapes,
 * sizes and states:
 *
 * - a plain lv_obj keeps LVGL's container shape (radius, border, padding)
 *   in MD3 colours: `surface`, an `outline_variant` border and `on_surface`
 *   content. Boxes, rows and columns have no theme styles.
 * - lv_label gets nothing of its own: it inherits its text role and colour
 *   from what it sits in.
 * - lv_button is an MD3 filled button: `primary` / `on_primary`, radius
 *   FULL, height `control_height[TRLV_SIZE_MD]`, `space[TRLV_SPACE_XL]`
 *   side padding, no shadow, Label Large text. Unlike LVGL's default theme,
 *   it lays out its content as a centred row (flex), so a label is centred
 *   without lv_obj_center and an icon and a label sit side by side,
 *   `space[TRLV_SPACE_SM]` apart.
 *
 * The **layout mode** is chosen here and stays for the life of the
 * application: layout tokens change sizes, so switching them at run time
 * would need every screen rebuilt. The **colour mode** can be switched
 * later with trlv_theme_set_color_mode.
 *
 * Calling it again (with other tokens, or after lv_deinit and lv_init)
 * overwrites the shared styles in place and re-initialises the themes
 * without allocating more. Objects that already use the shared styles
 * update; objects keep the theme styles they were created with.
 *
 * TreLVis has one set of tokens per application: the shared styles are
 * global, so they apply on every display. The theme is installed on `disp`
 * only; for a second display, set the same theme on it with
 * lv_display_set_theme(disp2, lv_display_get_theme(disp)).
 *
 * @param disp    the display to theme. Must not be NULL: LVGL v10 no longer
 *                accepts NULL for "the default display".
 * @param color   Color collection: the starting colour mode (e.g. light)
 * @param layout  Layout collection: the layout mode (e.g. Regular)
 *
 * A NULL argument is rejected with LV_LOG_WARN and nothing changes.
 *
 * @note The theme covers the screen, plain objects, labels and the filled
 *       button so far. Pressed, focus and disabled states and the other
 *       widgets in docs/concepts.md section 6.4 follow in milestone 4; until
 *       then they keep the parent theme's styles.
 */
void trlv_init(lv_display_t * disp, const trlv_color_tokens_t * color,
               const trlv_layout_tokens_t * layout);

/**
 * Switch the colour mode at run time, for example between light and dark.
 * Figma: switch the Color collection's mode; CSS: swap the values of the
 * `--md-sys-color-*` custom properties.
 *
 * Everything recolours, including LVGL's built-in widgets, with no code in
 * the screens:
 *
 * 1. The shared styles that hold colours (surfaces, cards, buttons, state
 *    layers, the theme's widget styles) are re-set in place from `color`.
 * 2. The parent theme is re-initialised with the new `primary` and
 *    `secondary` colours. LVGL's default theme also needs to know whether
 *    the mode is dark; TreLVis decides that from the luminance of
 *    `surface` (dark below 50 %), so the token structs need no extra flag.
 * 3. lv_obj_report_style_change(NULL) tells LVGL that the styles changed,
 *    so every object redraws. TreLVis never walks the object tree.
 *
 * Nothing is allocated except by the parent theme's re-initialisation,
 * which reuses LVGL's existing theme object. Colours that a project set
 * with lv_obj_set_style_* (local styles) are not touched: read the new
 * values from trlv_color() and set them again, or better, use the
 * surfaces and the theme.
 *
 * The struct is not copied: it must stay valid while it is in use.
 *
 * @param color  the Color collection mode to switch to. NULL is rejected
 *               with LV_LOG_WARN, and so is a call before trlv_init; in
 *               both cases nothing changes.
 *
 * @note Not implemented yet (milestone 4).
 */
void trlv_theme_set_color_mode(const trlv_color_tokens_t * color);

/**
 * The colour tokens in use: the Color collection's current mode. CSS
 * `var(--md-sys-color-*)`.
 *
 * For a project's own styles, for example
 * `lv_style_set_bg_color(&style, trlv_color()->tertiary_container)`. Values
 * read this way are copies: after trlv_theme_set_color_mode, read them
 * again.
 *
 * @return the tokens passed to trlv_init or trlv_theme_set_color_mode,
 *         or NULL before trlv_init
 */
const trlv_color_tokens_t * trlv_color(void);

/**
 * The layout tokens in use: the Layout collection's mode. CSS
 * `var(--space-md)` and so on.
 *
 * For values TreLVis has no modifier for, such as padding on one side:
 * `lv_obj_set_style_pad_left(obj, trlv_layout()->space[TRLV_SPACE_LG], 0)`.
 *
 * @return the tokens passed to trlv_init, or NULL before trlv_init
 */
const trlv_layout_tokens_t * trlv_layout(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRLV_THEME_H */
