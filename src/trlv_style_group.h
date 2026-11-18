/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2026 Timothy Mitchell */

/**
 * @file trlv_style_group.h
 * Internal: style groups, so that modifiers replace rather than stack.
 *
 * A group is a set of alternative shared styles for one modifier: the pad
 * styles (one per space step), the gap styles, the width styles (fit, fill),
 * the text roles. An object carries at most one style of a group per
 * selector. Applying a member replaces whichever member the object already
 * has; see docs/concepts.md section 6.1.
 *
 * Not part of the public API.
 */

#ifndef TRLV_STYLE_GROUP_H
#define TRLV_STYLE_GROUP_H

#include "trelvis/trlv_version.h"

/** The members of a group: either a list of style pointers (for styles
 *  declared one by one, such as LV_STYLE_CONST_INIT styles) or a contiguous
 *  array of styles (for token styles indexed by a scale enum). */
typedef struct {
    const lv_style_t * const * list;    /**< Members as pointers, or NULL */
    const lv_style_t * array;           /**< Members as an array, or NULL */
    uint32_t count;
} trlv_style_group_t;

/** Group initialiser from a static array of style pointers. */
#define TRLV_STYLE_GROUP_LIST(list_) { (list_), NULL, sizeof(list_) / sizeof((list_)[0]) }

/** Group initialiser from a static array of styles. */
#define TRLV_STYLE_GROUP_ARRAY(array_) { NULL, (array_), sizeof(array_) / sizeof((array_)[0]) }

/**
 * Apply one member of `group` to `obj` at `selector`, removing any other
 * member the object has at that selector.
 *
 * The new style takes the place of the old one in the object's style list,
 * so its precedence relative to other styles doesn't change: a project style
 * added after the modifier still wins after the modifier is called again.
 * If the object has no member yet, the style is added like lv_obj_add_style.
 * Applying the member the object already has changes nothing.
 *
 * @param obj       any object
 * @param group     the group `style` belongs to
 * @param style     the member to apply
 * @param selector  part and state, as for lv_obj_add_style
 */
void trlv_style_group_apply(lv_obj_t * obj, const trlv_style_group_t * group,
                            const lv_style_t * style, lv_style_selector_t selector);

/** Remove every member of `group` from `obj` at `selector`. */
void trlv_style_group_remove(lv_obj_t * obj, const trlv_style_group_t * group,
                             lv_style_selector_t selector);

#endif /* TRLV_STYLE_GROUP_H */
