# Roadmap

TreLVis is a small toolset for starting LVGL projects quickly and keeping them
consistent. It is built in milestones. Each milestone has a branch
(`m<N>-<topic>`) and usually one pull request; milestones 4 and 5 may be split
into several small PRs.

## What earns a place

TreLVis stays small on purpose. Its main job is to turn the blocks of LVGL
calls that every project repeats into one standard call. A public function is
added only if it does at least one of these:

- **Replaces a block of LVGL calls that projects repeat**, so it is written
  once and the same way everywhere (`trlv_row_create(parent)`: create the
  object, reset its theme look, padding and size, turn off scrolling and
  clicks, set the flow).
- **Takes a design token**, so screens hold no magic numbers
  (`trlv_set_pad(obj, TRLV_SPACE_LG)`).
- **Fixes an LVGL default that surprises** people coming from CSS, Figma
  or other UI toolkits
  (`trlv_box_create`: not scrollable, not clickable, no theme padding).
- **Makes a decision LVGL leaves to the caller** (`trlv_set_width_fill`: grow or
  100 %, depending on the parent's direction).

A function that only renames a single clear LVGL call does not qualify. For
those, the [mapping table](docs/concepts.md#10-mapping-figma--md3--css--trelvis--lvgl)
names the LVGL call to use.

**No new surprises for LVGL users.** Where TreLVis has no reason to differ,
LVGL's natural behaviour stands. Every deliberate difference (such as
`trlv_box_create` not scrolling) is documented on the function.

The v1 budget is about 20 layout and styling functions, plus a few for
initialisation and token access. At the API freeze the API has 29: 22 for
layout and styling (6 constructors, 10 layout setters, 6 appearance
functions) and 7 for setup and token access (`trlv_init`,
`trlv_theme_set_color_mode` and 5 getters). The trial reviews added
`trlv_apply_text_emphasis`, the three token value getters and the two
per-axis padding setters.

## Milestones

| # | Milestone | Status |
|---|---|---|
| 1 | **Scaffold**: repository structure, CMake target, `trelvis.mk`, configuration mechanism, version header and LVGL version check, changelog, NOTICE, SPDX headers, CI (ci / weekly / release workflows, Dependabot), C++ include test. Placeholder headers that compile. | done |
| 2 | **Vertical slice**: a first working slice of the library: the token template (`templates/`, copied into a project at setup and compiled in CI), a minimal `trlv_init` (no theme yet), `trlv_box_create`, `trlv_row_create`, `trlv_column_create`, `trlv_set_gap`, `trlv_set_pad`, `trlv_set_*_fill` / `trlv_set_*_fit` and `trlv_text_create`. One realistic screen is built with it as `examples/demo`: a plain `trlv_demo_create(parent)` function that any host project's simulator or target can call. A headless layout test harness (build a tree, update the layout, assert coordinates) starts running in CI. The slice is provisional: its job is to prove the approach and settle defaults before the API is fixed. | done |
| 3 | **API review**: full public headers for the v1 API, with doc comments and mapping tables, informed by the slice. The API is reviewed before further implementation. | done |
| 4 | **Tokens and theme**: full token types, the complete token template, the MD3 theme over the default parent theme, state layers, focus ring, colour-mode switching, line-height conversion. Widgets are covered in order: screen, label and button first, then one widget at a time. | done |
| 5 | **Surfaces, cards and remaining primitives**: `trlv_apply_surface`, `trlv_apply_surface_custom`, `trlv_apply_card`, `trlv_apply_button_type`, `trlv_apply_text_role`, `trlv_spacer_create`, `trlv_divider_create`, `trlv_set_justify`, `trlv_set_align_items`, the `TRLV_USE_DEBUG_OUTLINE` option. Layout tests for every primitive and modifier. | done |
| 6 | **Demo and docs**: `examples/demo` grows from the slice screen to exercise every primitive: surface container ladder; filled / elevated / outlined cards; all button types and covered widgets in each state; light/dark toggle; an example Compact token set next to the template values; visible keypad/encoder focus; a list built from rows (no `lv_list`). README quick start, `docs/concepts.md` and `docs/figma-handoff.md` brought up to date with the implementation. | done |
| 7 | **v1.x (on demand)**: additive items, each built only when a product needs it, before or after 1.0 (see below). A backlog, not a milestone with an end. | ongoing |
| 8 | **1.0**: the [compatibility promise](README.md#what-a-version-number-promises); CI that builds TreLVis the ways products do (a minimal `lv_conf.h`, a Cortex-M cross build with a footprint report, a consumer project, a check that every public symbol is documented); an audit of theme coverage for every LVGL 9.6 widget; product trials on 0.1.0, 0.2.0 and 0.3.0, with findings filed as `product-trial` issues; then the API freeze review and the 1.0.0 release. | done |

## Settled in the slice

Milestone 2 checked two behaviours on a real screen (`examples/demo`):

- **Fill inside a parent that hugs.** LVGL's result stands. Across the axis
  it works: the child stretches to the widest sibling (CSS
  `align-self: stretch`). Along the main axis the child always collapses to
  0. `trlv_set_*_fill` therefore warns only in the main-axis case; warning on
  the cross-axis case would flag a working pattern. See
  [docs/concepts.md §7.3](docs/concepts.md#73-layout-setters-trlv_set_).
- **`trlv_text_create` hugging by default.** Kept. On the demo screen every name,
  value, heading and button label wanted to hug; only the paragraph needed
  `trlv_set_width_fill`, and forgetting it shows at once as one clipped line.

Inputs for the API review (milestone 3):

- The unedited token template gave every text role `LV_FONT_DEFAULT`, so
  the demo had no type hierarchy until a project added fonts. Resolved in
  milestone 3: the template uses the Montserrat sizes LVGL ships when they
  are enabled.
- Texts of different sizes side by side in a row are top-aligned until
  `trlv_set_align_items` lands (milestone 5). LVGL flex has no baseline
  alignment; centring is the closest match. Resolved in milestone 5:
  `trlv_set_align_items(row, TRLV_ALIGN_ITEMS_CENTER)`.
- A `trlv_box_create` with children placed by `lv_obj_align` (left and right, at
  `*_MID`) hugs its height correctly, which covers a title-and-status header
  before `trlv_set_justify` exists.
- No naming changes came up. Filling both axes takes `trlv_set_width_fill` and
  `trlv_set_height_fill`; a combined call would only join two clear calls, so
  none is proposed.

## Settled in milestone 5

- **Align items places the line too.** LVGL's `flex_cross_place` places
  items within their line, which is only as deep as the deepest item and
  sits at the start of the container. `trlv_set_align_items` also sets
  `flex_track_place`, so items are placed in the container as in Figma and
  CSS. A project wanting CSS `align-content` sets the track placement
  afterwards as a local style.
- **Spacer** warns under its own name (`trlv_spacer_create`) in a parent that
  hugs its main axis, and has no size in a parent without flex.
- **Debug outline** is part of `trlv_style_box`: magenta, 1 px, outside
  the object. It costs nothing when off and no RAM when on.
- **Fill group:** surfaces, custom colours and cards are one style group.
  Every card sets every border and shadow property (to none where its type
  has none), so cards replace each other cleanly and override a theme
  border on a plain `lv_obj`.
- **Custom colours** are capped at `TRLV_CUSTOM_COLOR_MAX` 8: each style is
  a separate constant, spelled out per index.
- **Button types** style every state the theme styles (default,
  pressed, focus key, disabled), because LVGL resolves a property by state
  before style order. Outlined and text buttons have no container to
  pre-mix a state layer with, so theirs is the content colour at the state
  opacity, over whatever is behind.
- **Divider** styles are token styles built with the colours, so the line
  follows colour-mode switches and its thickness follows `trlv_init`.
- **Borders inset the content.** LVGL lays children out inside the border
  and then the padding, as Figma does for an inside stroke included in
  layout (CSS `border-box`). The docs said the border didn't add to the
  padding; they are corrected.

## Settled in milestone 4

- **Custom colours** replace the fixed success and warning roles: a
  project-defined table in the colour tokens, applied with
  `trlv_apply_surface_custom`. The token structs have one shape for every
  configuration.
- **Theme install:** the TreLVis theme sits over LVGL's default theme and
  is found again by a marker in its user data, so `trlv_init` and colour
  switches allocate nothing new. `trlv_init` restyles the active screen
  itself while it is empty, because LVGL 9.6's `lv_display_set_theme`
  restyles the bottom layer instead.
- **Buttons** lay out their content as a centred row (a documented
  difference from LVGL's default theme: `lv_obj_align` on their children
  is ignored unless the child floats or the layout is turned off).
- **Focus ring clearance** is a layout rule: containers of focusable
  widgets need padding of at least the ring's reach. LVGL's
  overflow-visible flag cannot lift the clipping for a plain box.
- **States:** pressed and focus layers are pre-mixed colours; disabled is
  real opacity, as MD3 specifies. LVGL's recolour, grow and focus outline
  are switched off. The switch, checkbox, slider and arc have no circular
  state layer around the handle.
- **Fields** (dropdown, textarea) show focus with a `primary` border
  `focus_width` thick, not the focus ring.

## Settled in the API review

Milestone 3 wrote the full v1 headers; they are the source of truth for
signatures, and the C++ include test pins them. Decisions that shape
milestones 4 and 5:

- **Cut:** `trlv_gap_xy`, `trlv_pad_xy` and `trlv_absolute`. Each joined two
  clear LVGL calls; the layout header's "Left to LVGL" table names them.
  Absolute position uses `lv_obj_set_floating`, which (unlike ignore-layout)
  is left out of a hugging parent's size, as in Figma.
- **Style catalogue:** a style is public only when no modifier can apply it
  to an existing object: `trlv_style_box`, `trlv_style_row`,
  `trlv_style_column`. Everything else is internal.
- **Theme:** `trlv_init` comes before any objects are created. LVGL's default
  theme gets its dark flag from the luminance of `surface`. The theme never
  sets a text role on plain labels (the screen carries Body Medium, a button
  Label Large), so `trlv_apply_text_role` on a container reaches its labels.
- **Surfaces and cards** are one style group and replace each other; cards
  are not interactive in v1.
- **Naming:** cross-axis values are `TRLV_ALIGN_ITEMS_*`, so they can't be
  mistaken for `lv_align_t`.
- **Token template:** text roles use LVGL's Montserrat sizes when enabled,
  falling back to the next smaller enabled size.

Changed at the start of milestone 4, in a review of the token schema:

- **Custom colours replace the fixed success and warning roles.** Colours
  beyond the MD3 roles differ per product (ISA-101 alarm priorities,
  IEC 60601-1-8 alarm levels, brand accents), so they are a project-defined
  table in the colour tokens, applied with `trlv_apply_surface_custom`. The token
  structs keep one shape: no configuration option adds or removes fields.
  `TRLV_CUSTOM_COLOR_MAX` only sizes the shared styles.

## v1 acceptance criteria

- The demo runs in a host project's simulator. Toggling light/dark recolours
  everything, including built-in widgets, with no per-screen code.
- A new project gets a themed screen by copying the token template and
  calling `trlv_init`, before it edits a single value.
- The same demo is correct in compact and regular layout modes.
- Text inside surfaces and cards gets the correct `on_` colour purely through
  inheritance.
- The public API is within the budget above, and every function passes the
  "What earns a place" test.
- Headless layout tests cover every primitive and modifier and pass in CI.
- Builds warning-free (including LVGL deprecations) with
  `-Wall -Wextra -Wpedantic -Werror` on LVGL 9.6; `trelvis.h` compiles from C++17.
- No direct `malloc` / `lv_malloc` in library code (CI grep). No allocation
  outside `trlv_init` and colour-mode switches other than LVGL's own object
  creation.
- Builds by adding `src/` + `include/` alone (CI step).
- Every public function is documented with its Figma / MD3 / CSS equivalent.

## 1.0

TreLVis 1.0.0 is released once it has been used in a real product. In 0.x
a breaking change is cheap; after 1.0.0 it costs a major version, so
anything that might break goes in before the API freeze. From 1.0.0 the
[compatibility promise](README.md#what-a-version-number-promises) applies.

- Findings from the product trial are filed with the *Product-trial
  finding* issue form. Each is fixed before 1.0.0, or moved to v1.x or
  Later as non-breaking.
- CI builds TreLVis against a minimal `lv_conf.h`, cross-builds it for a
  Cortex-M target with a footprint report, builds a consumer project the
  way products use TreLVis, and checks that every public symbol is
  documented.
- An audit of every LVGL 9.6 widget decides which ones the theme covers;
  gaps it finds become v1.x items. The bugs it finds in covered widgets
  are fixed before the freeze.
- 0.2.0 is released with the 1.0 review's breaking changes and the
  audit's fixes, and a second product trial runs on it, so the API is
  frozen as it was used, not as it was designed. Its findings are filed
  the same way.
- 0.3.0 is released with the trial-2 review's changes, and a third
  product trial runs on it before the freeze. Trial 2 changed the design
  side as well as the API (`fit`, lower-emphasis text, dim and bright
  surfaces, the token getters, and the tool-neutral
  [Designing a screen](docs/designing-a-screen.md) and
  [Building a token set](docs/building-a-token-set.md)), and none of that
  has been used in a product yet. The third trial uses 0.3.0 as released,
  ideally on a kind of screen the first two didn't cover, and checks that
  a screen can be designed from the docs alone. Its findings are filed
  the same way. Breaking ones need another 0.x release; additive ones go
  to v1.x. Done: it found nothing breaking, so no 0.4.0 is needed
  ("Settled in the trial-3 review").
- The API freeze review records its decisions here. Done ("Settled in
  the API freeze review").
- 1.0.0 is released with the trial-3 review's additions and the freeze
  review's rename and toggle buttons. Done: from 1.0.0 the public API
  changes only additively, and v1.x items are built on demand (below).

### Settled in the 1.0 review

The product trial ported a Figma screen, and the developer read
`trlv_column(screen, …)` as "make this a column": the new column was thrown
away and the layout was silently wrong. These decisions came out of it
(#141, #142, #143) and the discussion that followed. Names elsewhere in this file are the 1.0 names; the
table shows them as this review left them, before `hug` became `fit` (trial-2 review) and the button
and card enums became `trlv_button_type_t` and `trlv_card_type_t` (API freeze review).

- **Words from Figma / CSS / MD3, grammar from LVGL** (#141). The verb at
  the call site says what a call does. Constructors end in `_create`,
  layout setters are `trlv_set_<property>`, appearance functions are
  `trlv_apply_<look>`, getters use `_get_`. Setters and applies replace
  their previous value (LVGL's `set`, not `add`, which stacks). There is
  no `obj` in their names: they work on any object, so there is no class
  to name. Fill and hug follow the same pattern
  (`trlv_set_width_fill`), so the API has one grammar throughout, even
  where the Figma wording is "fill width". The token getters are
  `trlv_get_color_tokens()` and `trlv_get_layout_tokens()`, named after
  what they return, which also frees `trlv_layout` from clashing with
  `trlv_layout.h` and LVGL's "layout".

  | 0.1 | 1.0 |
  |---|---|
  | `trlv_box`, `trlv_row`, `trlv_column`, `trlv_spacer`, `trlv_divider`, `trlv_text` | `trlv_box_create`, `trlv_row_create`, `trlv_column_create`, `trlv_spacer_create`, `trlv_divider_create`, `trlv_text_create` |
  | `trlv_gap`, `trlv_pad`, `trlv_justify`, `trlv_align_items` | `trlv_set_gap`, `trlv_set_pad`, `trlv_set_justify`, `trlv_set_align_items` |
  | `trlv_fill_width`, `trlv_fill_height`, `trlv_hug_width`, `trlv_hug_height` | `trlv_set_width_fill`, `trlv_set_height_fill`, `trlv_set_width_hug`, `trlv_set_height_hug` |
  | `trlv_surface`, `trlv_surface_custom`, `trlv_card`, `trlv_text_role`, `trlv_button_variant` | `trlv_apply_surface`, `trlv_apply_surface_custom`, `trlv_apply_card`, `trlv_apply_text_role`, `trlv_apply_button_type` |
  | `trlv_color()`, `trlv_layout()` | `trlv_get_color_tokens()`, `trlv_get_layout_tokens()` |
  | `trlv_init`, `trlv_theme_set_color_mode` | unchanged |

- **Constructors take no spacing; every layout property is a setter**
  (#142). A constructor takes the parent plus only arguments whose type
  makes them clear at the call site; a bare `TRLV_SPACE_*` fails that
  test, so the gap left `trlv_row_create` and `trlv_column_create`.
  Gap, padding, fill, hug, justify and align items are one setter line
  each, for every construct, like the rows of Figma's auto layout panel.
  Putting "the common ones" in the constructor had no principled place
  to stop: in the trial screen fill width followed 13 of 14 containers,
  padding only 6.
- **Defaults** (#142). Layout containers (box, row, column) start
  empty, like a CSS `div` or a fresh Figma frame. LVGL widgets get their
  MD3 padding from the theme, as Figma kit components have theirs.
- **`set` for layout, `apply` for appearance.** Layout properties are
  set, as in Figma's auto layout panel; colour, shape and type are
  applied, as Figma applies a colour or text style (and as LVGL's
  `lv_theme_apply` gives an object its look). A set never changes colour
  and an apply never changes layout, so a call's group shows in its
  name. The button function is `trlv_apply_button_type`, MD3's term,
  matching the card types. "Style" was rejected, as it means `lv_style_t`
  to LVGL users; a module prefix (`trlv_layout_set_gap`) was rejected for
  length, and a verbless `trlv_layout_gap` would repeat #141's problem.
- **Cards are an applied look, with no constructor** (#143). A
  `trlv_card_create` that padded cards was tried and dropped:
  - having a card constructor and a card setter side by side was
    confusing;
  - a card setter that also set padding mixed appearance with layout,
    and needed special precedence rules against `trlv_set_pad`;
  - every part of an MD3 card (headline, subhead, media, supporting
    text, actions) is optional, so no card constructor fits all cards.

  A card is a row or column, `trlv_apply_card`, and `trlv_set_pad`
  (`LG` in MD3). A card with fixed content, such as a header, is a
  project component (concepts §2).
- **Appearance applies to any object, and warns on odd targets.** As
  with LVGL styles, nothing is refused. An `LV_LOG_WARN` (changing
  nothing) catches the combinations a reasonable user might try and be
  surprised by:
  - a card on an LVGL widget rather than a plain container;
  - a surface or custom colour on an `lv_button` (a red Stop button),
    where pressed and focus keep the button's colours (a custom-coloured
    button would be a v1.x item);
  - a button type on an object that can't be pressed.

  Surfaces, custom surfaces and cards stay one group; a button type
  stays its own, as the combinations a merged group would fix are
  nonsense anyway (`tests/theme/test_apply_warnings.c`).

The cost is two more lines per card and one more per container with a
gap: the README card is 15 lines against plain LVGL's 32.

Trimmed in the pre-1.0 triage, as they don't pass "What earns a place":

- **Subject-bound theme switching** (`trlv_theme_bind_subject`): following
  a subject is one observer calling `trlv_theme_set_color_mode`, a recipe
  in [docs/concepts.md §6.3](docs/concepts.md#63-theme-api).
- **Responsive helpers** (for example a row that becomes a column by
  layout mode): the layout mode is fixed at `trlv_init`, so this is one
  `if` in the screen code.

### Settled in the trial-2 review

The second product trial (0.2.0) built a process-HMI screen in the
high-performance style of ISA-101, with no designer and no Figma file. An
AI agent chose the look from the docs. The code held up: about 700 lines,
warning-free and correctly laid out on the first run. The design side
assumed Figma and MD3 knowledge: the only design rules were in the Figma
hand-off doc, role choice needed MD3, and about half the screen was off
the easy path. These decisions came out of the review (#155–#163, #144,
#145, #146).

- **Identity** (#155). TreLVis's design language is inspired by, and
  compatible with, Material Design 3. It keeps MD3's colour roles and
  type scale, so Material tools and designs map one-to-one, and explains
  them by purpose, so neither Figma nor MD3 is needed. "Inspired by" lets
  TreLVis add what MD3 lacks. Figma is one supported hand-off. MD3 is
  credited in the README, NOTICE and concepts.
- **Vocabulary** (#155). The most intuitive existing word for a reader
  who knows CSS or LVGL but not Figma; MD3 role names for tokens, for
  compatibility; still no fourth vocabulary. An audit of every public name
  kept all but `hug`. A setter is described as a layout property (size,
  spacing, flow), not "a row of Figma's auto layout panel".
- **`hug` is renamed `fit`** (#156): `trlv_set_width_fit`,
  `trlv_set_height_fit`, after CSS `fit-content`. This is the review's one
  breaking change, so it needs a 0.3.0 before the freeze, and a third
  product trial runs on 0.3.0 (see above).
- **No radius or height setters** (#145, #146). They would rename single,
  clear LVGL calls. What the trials objected to was the token expression,
  so raw calls take tokens through `trlv_get_space`, `trlv_get_radius` and
  `trlv_get_control_height` (#161).
- **Added before 1.0:** `trlv_apply_text_emphasis` (lower-emphasis text in
  `on_surface_variant`, a shared style that follows colour-mode switches;
  #144) and `TRLV_SURFACE_DIM` / `_BRIGHT` (#160). On a coloured container
  the emphasis is still `on_surface_variant`, documented as meant for
  surfaces.
- **One token template** (#158). TreLVis ships the MD3 baseline template,
  with a generic status set (fault, warning, advisory, bad data) as its
  custom colour example; it has no "error" entry, so a fault never reads
  as MD3's input-error role. The HMI example is removed: building a
  scheme for a domain is the project's job. Two docs support it:
  [Designing a screen](docs/designing-a-screen.md) for tool-neutral rules
  and role choice, and [Building a token set](docs/building-a-token-set.md)
  for what each token is used for.
- **Testing scope.** CI tests the library and what TreLVis ships (the
  template and the demo), not every scheme a project might design. A
  small test-only contrast check (#159) is an aid a project can build with
  its own tokens. External theme tools (#86, #147) may be supported more
  officially later.
- **Per-axis padding stays "Not planned"** for now (built in the trial-3
  review), with the trial's count (5 of 7 padded objects) kept as
  evidence in #162. With
  `trlv_get_space` the raw call reads cleanly.
- **Left to projects:** components outside MD3's set (alarm indicators,
  range bars, state markers, chips, badges, dialogs) are compositions of
  primitives, as Designing a screen shows. Ready-made ones would belong in
  `trelvis-starter`, not the library.

### Settled in the trial-3 review

The third product trial (0.3.0) built a controller screen with a list and
a detail view, at 800 × 480 and at 320 × 240 with the Compact tokens, in
light and dark, with a fault state, a confirmation dialog and keypad
focus. An AI agent designed it from the docs alone. The digest it wrote
before any code held up (10 of 10 rules, 6 of 6 plan items), and no
TreLVis source was needed to build the screen. Every wrong screenshot came
from LVGL, the host build or the application. The cost was in learning:
about 8.5k lines read before the first correct screenshot, roughly half
of them LVGL, where in hindsight about 3.3k would have done. These
decisions came out of the review (#162, #171, #172).

- **Nothing breaking, so no 0.4.0 and no fourth trial.** The next step is
  the API freeze review (#123).
- **Per-axis padding is built before the freeze** (#162). Trial 2 asked
  for it (5 of 7 padded objects), trial 3 needed it for its status chips,
  and the chip recipe in Designing a screen taught the raw call:
  `trlv_set_pad_horizontal(obj, space)` and
  `trlv_set_pad_vertical(obj, space)`. It earns a place because it takes a
  token and uses a shared style where the raw call adds a local one to
  every chip. Padding is now two style groups, one per axis, and
  `trlv_set_pad` sets both, so the last call wins on each axis in any
  order, as CSS `padding` and `padding-inline` do. A separate horizontal
  group beside a four-sided one was rejected: `trlv_set_pad` called
  after it would silently undo it. One-side padding and a different row
  and column gap stay raw LVGL.
- **A first-use path** (#171). [Getting started](docs/getting-started.md)
  gives the reading order, the kinds of call, how fit and fill depend on
  the parent, one complete keypad-ready screen (pinned by a test), and
  the LVGL pitfalls the trial hit. The README stays the overview, and its
  Documentation section is a map of the docs in reading order. The LVGL
  half of the reading cost is accepted: TreLVis doesn't teach LVGL, and
  projects can use LVGL's own docs or its MCP server. Doxygen
  presentation and a docs site come later, and are meant more for people
  than for agents.
- **The two recipes now agree** (#172). A screen's snapshot leaves out
  `lv_layer_top()`, where the modal recipe puts its scrim. Both recipes
  say so.
- **Left to projects and LVGL:** host integration (SDL drivers, SDL2main
  on Windows, CMake target scope), event wiring, timers, scrolling and
  dialog state. TreLVis ships no simulator; a project skeleton belongs in
  `trelvis-starter`.

### Settled in the API freeze review

The last review before 1.0.0 (#123) went through every public symbol in
`include/trelvis/`, every configuration option and every open issue
postponed to v1.x or Later, asking of each whether changing it after 1.0
would break the [compatibility promise](README.md#what-a-version-number-promises).
After three product trials the API needed one rename and one addition.

- **Renamed: `trlv_button_t` → `trlv_button_type_t`, `trlv_card_t` →
  `trlv_card_type_t`** (#123). As type names they read like a handle to a
  button or card object, which a C++ facade with handles (#85) would make
  worse. The new names match MD3's "button type" and "card type" and
  `trlv_apply_button_type`. The values (`TRLV_BUTTON_TONAL`, …) and call
  sites don't change, and every use of the old names is a compile error,
  so it ships in 1.0.0 with no further 0.x release or trial.
- **Added before 1.0: toggle buttons** (#116). Toggle buttons are common
  in HMIs, and a checkable `lv_button` had no MD3 selected look. They are
  three button types, `TRLV_BUTTON_FILLED_TOGGLE`, `_TONAL_TOGGLE` and
  `_OUTLINED_TOGGLE`, with MD3's unselected and selected
  (`LV_STATE_CHECKED`) looks; MD3 has no text toggle. A type, not a new
  function or a check of `lv_obj_is_checkable` at call time: the call
  site says it is a toggle, and the order of calls doesn't matter to the
  look (a toggle type on an object that isn't checkable warns). The theme
  removes the parent theme's checked colour from `lv_button`, so only a
  toggle type has a selected look. Looks that equal an existing type's
  share its style (about 0.5 KB of static RAM on Cortex-M4).
- **Kept: every function.** All 29 pass "What earns a place"; none was
  cut. `trlv_theme_set_color_mode` keeps its name: it is a setup call on
  no object, and `trlv_set_*` is the grammar for a layout property of an
  object.
- **Kept: `trlv_apply_surface_custom(obj, uint32_t index, …)`**, the
  question #123 started with. `uint32_t` matches `custom_count` and LVGL's
  index types, and a project's C enum (or unscoped C++ enum) converts to
  it without a cast. The cap of 8 (`TRLV_CUSTOM_COLOR_MAX`, default 4,
  which fits the template's status set) stays; raising the maximum later
  is additive.
- **Kept: `trlv_size_t`** as a general size scale, which icon sizes (#78)
  may share. The `control_height` field keeps its meaning.
- **Token structs frozen as they are.** MD3 roles TreLVis leaves out on
  purpose (`background` / `on_background`, `surface_variant`,
  `surface_tint`, the `*_fixed` roles) can be added later as fields no
  TreLVis function reads, so a copied template that lacks them still
  works. A scale can grow too: the template uses designated array indices
  (`[TRLV_SPACE_SM] = 8`), and enum numeric values are promised only
  where a header documents them.
- **Configuration options kept, with their defaults:**
  `TRLV_LVGL_INCLUDE`, `TRLV_CONF_PATH`, `TRLV_IGNORE_LVGL_VERSION`,
  `TRLV_USE_THEME`, `TRLV_THEME_PARENT` (`_NONE` / `_DEFAULT`),
  `TRLV_CUSTOM_COLOR_MAX` and `TRLV_USE_DEBUG_OUTLINE`.
- **Public styles kept:** `trlv_style_box`, `trlv_style_row`,
  `trlv_style_column`.
- **Every postponed item can be added in 1.x.** None needs a breaking
  change, so none blocks 1.0:

  | Issue | How it fits 1.x |
  |---|---|
  | #78 icons | `trlv_icon_create(parent, symbol, size)` and an icon font token per size, safe at zero (NULL falls back to LVGL's symbol font). |
  | #80 motion | Duration token fields; zero means no transition. |
  | #81 elevated button | `TRLV_BUTTON_ELEVATED` and `_ELEVATED_TOGGLE`, appended to `trlv_button_type_t`. |
  | #82 hover | A `state.hover` opacity field and an `LV_STATE_HOVERED` layer; a theme look change. |
  | #83 debug dump | A new configuration option, off by default. |
  | #132–#139 widget styles | Theme look changes, listed under *Changed* in a minor release. |
  | #163 custom colours as text | `trlv_apply_text_custom(obj, uint32_t index)`, in the same style group as `trlv_apply_text_emphasis`, so the two text colours replace each other. |
  | #174 custom-coloured button (the red Stop button the apply warnings mention) | A new appearance function, or button states for a custom surface; additive. |
  | #85 C++ facade, #86 token import, #147 Figma variables file | Tooling over the C API; no change to it. |
  | #88 LVGL v10 | TreLVis 2.0, with a `release/v1.x` branch, as the promise says. |

- **Product-trial issues closed or moved.** #145 and #146 (radius and
  height setters) are closed by the trial-2 decision: token getters
  instead. #147 stays under Later and #163 under v1.x.

## v1.x (on demand)

Additive items, each built only when a product needs it, before or after
1.0:

- `trlv_icon_create(parent, symbol, size)`: an icon-font glyph sized by a
  new icon token, with inherited colour.
- Motion tokens (`duration_short` / `duration_medium`) and style transitions
  for state changes.
- Elevated button type, and its toggle.
- Hover state layer (`LV_STATE_HOVERED`) for pointer-driven products.
- `trlv_debug_dump(obj)`: prints the object tree with sizes, padding, gaps and
  styles for comparison with Figma (`TRLV_USE_DEBUG`).
- MD3 styles for the widgets the coverage audit found looking wrong next
  to MD3 ([docs/concepts.md §6.4](docs/concepts.md#64-widget-coverage-v1)):
  roller, button matrix and keyboard, message box, spinbox, table, chart,
  calendar, and the tabview and tileview content. One issue each.
- `trlv_apply_text_custom(obj, index)`: a custom colour as text colour,
  following colour-mode switches, when a product needs it (#163).
- A custom-coloured button (a red Stop button), with its pressed, focus
  and disabled states (#174).

## Later (designed for, not yet built)

- **C++ facade:** header-only `trelvis.hpp` with non-owning handles, chainable
  setters and a lambda event helper (heap use switchable off). C++17, no
  exceptions or RTTI.
- **Token generator:** DTCG (2025.10) JSON and Material Theme Builder JSON →
  the same token structs.
- **LVGL v10:** TreLVis already follows v10 forward-compatibility rules and
  builds weekly against LVGL master. Support for v10 comes in a new major
  version, with a `release/v1.x` branch for products that stay on LVGL 9
  (see the [compatibility promise](README.md#what-a-version-number-promises)).
- **Theme role indirection:** which colour role fills which widget part,
  set in the tokens. Only if a product needs a look the role values can't
  reach; the trial-2 HMI scheme got its look from values alone.

## Related project: `trelvis-starter`

A separate template repository for new products is planned. It will pin a
TreLVis release and provide the project skeleton: LVGL and TreLVis
submodules, CMake presets, platform folders (simulator / Linux / STM32), a
`ui/` folder (globals, tokens, fonts, images, components, widgets, screens),
font tooling, and reference components. Scaffolding and templates belong
there, not in this library.

To keep that possible, TreLVis keeps primitives usable from `_create()`-style
functions, keeps every modifier working on any `lv_obj_t`, and keeps project
token values and subjects out of the library.

## Not planned

These are deliberately excluded:

- Wrappers for single clear LVGL calls: visibility, clipping, scrolling,
  wrapping, text truncation, min / max size, grid cells, different
  horizontal and vertical gaps or padding (`_xy` variants), absolute
  position, corner radius, control height. The mapping table names the
  LVGL call for each, with a token getter (`trlv_get_space`, …) where it
  takes a token. Per-axis padding has token setters
  (`trlv_set_pad_horizontal` / `_vertical`, from the trial-3 review); the
  `_xy` variants, which joined gap or padding on both axes in one call,
  stay cut.
- `trlv_page` (header / body / footer) and `trlv_grid`. Not every project
  uses them; a page is a project component, and grids use LVGL's grid API.
- Token values compiled into the library. Tokens come from a template the
  project copies and owns, so unused values cost no flash.
- A simulator. Host projects and `trelvis-starter` have their own; TreLVis
  ships a demo function they can call, and headless tests for CI.
- Ripples, MD3 motion specs and MD3 Expressive additions; full MD3 component
  specs.
- Per-corner radii, corner smoothing.
- Gradient tokens, blend modes, inner shadows, background blur.
- Aspect-ratio locking; Figma "scale" constraints.
- Breakpoints / media queries (layout modes are used instead).
- Separate `overlay` / `center` / `fixed` helpers (`trlv_box_create` with
  `lv_obj_set_floating` + `lv_obj_align`, and `lv_obj_center`, cover these).
- LVGL colour filters for state layers.
- Wrapping LVGL widgets, macro DSLs, varargs builders.
- LVGL Pro artefacts or tooling (XML, `globals.xml`, runtime loading, editor
  previews).
