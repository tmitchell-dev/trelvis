# CLAUDE.md

Guidance for AI coding agents. The contributor guide applies in full and is
imported here:

@CONTRIBUTING.md

## Project in one paragraph

TreLVis (`trelvis`, C prefix `trlv_` / `TRLV_`) is a small C99 utility layer
on LVGL 9.6. It provides CSS-style layout primitives, design token types
with Material Design 3 (MD3) role names, and a theme (MD3's look by default)
built on LVGL's theme and style engine. Its design language is inspired by
and compatible with MD3, and usable with or without Figma.
It is a utility layer, not a framework: everything takes or returns plain
`lv_obj_t *`, every modifier works on any object, and LVGL widgets are styled,
never wrapped. MIT licensed, used as a git submodule from C and C++ products
(Linux/Raspberry Pi and STM32), written to be published as open source.

## Where the design lives

- [ROADMAP.md](ROADMAP.md): milestones, v1 acceptance criteria, what's out of scope.
- [docs/concepts.md](docs/concepts.md): vocabulary, naming, tokens, fonts,
  theme/style engine, primitives and modifiers, mapping tables. Read the
  relevant section before changing an area. Once the public headers exist,
  they are the source of truth for signatures.
- [docs/getting-started.md](docs/getting-started.md): the first-use path
  (reading order, the kinds of call, fit/fill, one complete screen pinned
  by `tests/layout/test_getting_started.c`, what stays LVGL).
- [docs/designing-a-screen.md](docs/designing-a-screen.md): tool-neutral
  design rules and which role to use for what.
- [docs/building-a-token-set.md](docs/building-a-token-set.md): what each
  token is used for and what TreLVis assumes about it.
- [docs/figma-handoff.md](docs/figma-handoff.md): Figma conventions.

If an implementation decision changes the design, update the doc in the same PR.

## Current status

Milestones 1 (scaffold), 2 (vertical slice), 3 (API review), 4 (tokens
and theme), 5 (surfaces, cards and remaining primitives) and 6 (demo and
docs) are done, and 0.1.0 is the v1 feature-complete release. 0.2.0
carries the 1.0 review's breaking changes and the theme audit's fixes,
for the second product trial. 0.3.0 carries the trial-2 review (the
MD3-inspired design language, `hug` renamed `fit`), for a third product
trial before the API freeze.
The slice implemented the token template, `trlv_init`, box, row, column,
gap, pad, fill/fit and text, the demo screen `trlv_demo_create(parent)` in
`examples/demo`, and headless layout tests. TreLVis ships no simulator; host
projects run the demo in their own.

The API review wrote the full v1 public headers. They are the source of
truth for signatures, and `tests/cpp_include_test.cpp` pins every one.
The review's decisions are in ROADMAP "Settled in the API review".

Milestone 4 built the MD3 theme in `src/trlv_theme.c` over LVGL's default
theme: screen, plain objects, labels, the filled button with its states,
switch, checkbox, slider, bar, arc, spinner, dropdown and textarea, and
colour-mode switching. Its decisions are in ROADMAP "Settled in
milestone 4". Theme tests are in `tests/theme/`.

Milestone 5 implemented the rest of the v1 API: justify, align items,
spacer, divider, text roles, surfaces, custom colours, cards, button
variants and the debug outline. Every v1 function is now built. Its
decisions are in ROADMAP "Settled in milestone 5".
`tests/layout/test_replace_not_stack.c` checks that every modifier group
replaces its previous style; add a case there for any new group.
Config options are tested by building a copy of the library with the
option set (`trlv_add_option_test` in `tests/CMakeLists.txt`).
`-DTRLV_TEST_CONFIG=minimal` builds against `tests/config/minimal` (a
small product's `lv_conf.h` and `trlv_conf.h`) and runs only the tests
that need no widgets, log or extra fonts; register a new test in that
group when it can run there.

Milestone 6 grew `examples/demo` into sections that exercise every
primitive (surfaces, cards, button types and themed widgets in their
states, a list built from rows, a light/dark switch, keypad focus), added
an example Compact token set and an HMI colour scheme next to the
template, brought the README, concepts and Figma hand-off docs up to date,
and checked the v1 acceptance criteria (evidence on issue #75).

**Now: M8, 1.0** (ROADMAP "1.0", tracking issue #124): the compatibility
promise (README "What a version number promises"), CI build coverage, a
theme coverage audit and a product trial, then a 0.2.0 release for a
second trial on the 1.0 API, a 0.3.0 release with the trial-2 review's
changes, a third product trial on 0.3.0, the API freeze (#123) and 1.0.0.
The third trial found nothing breaking (ROADMAP "Settled in the trial-3
review"). Its additive outcomes (the per-axis padding setters and the
getting-started page) go in before the freeze. The API freeze review
(#123, ROADMAP "Settled in the API freeze review") renamed the button and
card enums to `trlv_button_type_t` / `trlv_card_type_t`, added the MD3
toggle button types (#116) and kept everything else; the next step is the
1.0.0 release PR. After 1.0.0 the public API changes only additively.
The 1.0 review moved the API to LVGL's grammar: `trlv_*_create`
constructors that take no spacing, `trlv_set_*` layout setters,
`trlv_apply_*` appearance functions (which warn on an object they make no
sense on) and `trlv_get_*_tokens` getters. Cards stay an applied look,
with no constructor (#141–#143, ROADMAP "Settled in the 1.0 review").
Alongside it, v1.x items are built one at a time and only when a product
needs one (ROADMAP "v1.x (on demand)"). An API change still needs the
docs, the include test and the changelog updated with it, and must keep
the compatibility promise: token fields safe at zero, templates on
designated initialisers, a Migration note for any token schema change.

Work is tracked in GitHub issues, one per item, under milestones
(`M<N>: …`, plus the `v1.x (on demand)` and `Later` backlogs) with a
tracking issue each. Findings from using TreLVis in a product are filed
with the product-trial issue form (label `product-trial`). Each PR says
`Closes #N` for the issues it completes, and the tracking issue is kept
current.

Keep the API small. The library's main job is to turn blocks of LVGL calls
that every project repeats into one standard call. Before adding a public
function, check it against "What earns a place" in [ROADMAP.md](ROADMAP.md);
a rename of a single clear LVGL call doesn't qualify. Don't create new surprises for
LVGL users: follow LVGL's natural behaviour unless the docs state a
deliberate difference.

## Commands

```sh
cmake -B build && cmake --build build     # fetches LVGL v9.6; strict warnings; C++17 include test
cmake -B build -DFETCHCONTENT_SOURCE_DIR_LVGL=../lvgl   # use a local LVGL tree
ctest --test-dir build --output-on-failure  # headless layout tests
```

Layout tests live in `tests/layout/`, one file per area, built on the plain-C
harness in `tests/harness/trlv_test.h` and registered with `trlv_add_test`
in `tests/CMakeLists.txt`. Positions in assertions are measured from the
parent's outer edge (as in Figma), not from inside its padding.
If no compiler is available locally, say so rather than claiming a build
passed; CI ([.github/workflows/ci.yml](.github/workflows/ci.yml)) is the
reference.

The LVGL pin `v9.6.0` lives in `tests/CMakeLists.txt` and the README
compatibility table. Change both together.

## Agent-specific reminders

- Check exact LVGL function names in the 9.6 headers (`include/lvgl/…` in the
  fetched tree under `build/_deps/lvgl-src`) rather than guessing. In
  particular, use the per-flag setters, not `lv_obj_add_flag`.
- Never silence warnings (including LVGL deprecations) to get a build through.
- Work on a branch, never on `main`. Commit and push to the working branch
  freely, following "Commits and pull requests" below; never commit or push
  to `main`. Open a pull request only when asked.
- Nothing company- or product-specific, and no secrets, anywhere, including
  commit messages and PR text.

## Commits and pull requests

When committing and opening a PR:

- **One PR for the block of work, made of several meaningful commits.**
  Split by issue or by step (for example a mechanical rename, then a
  behaviour change, then docs), not by file type. Each commit builds and
  passes the tests on its own, and its message ends with `Refs #N` for the
  issues it touches. The PR says `Closes #N` for each issue it completes
  (an issue settled by a decision rather than code is closed with that
  decision stated in the PR).
- **Run the hook checks.** Enable the shared hooks with
  `git config core.hooksPath .githooks` before pushing. Before pushing,
  also run what CI runs and can be run locally (see
  [CONTRIBUTING.md](CONTRIBUTING.md) "Development setup"): the GCC and
  Clang builds and tests, `-DTRLV_TEST_CONFIG=minimal`, the compile
  without CMake, `tests/consumer` and `doxygen`. In the PR, say which
  checks ran and which didn't.
- Fill in the PR template (`.github/pull_request_template.md`).
- **Never approve, accept or merge a pull request**, in any circumstances,
  and never enable auto-merge. The maintainer does the final checks and
  merges.

## Design discussions

For an API or design decision, show the options as concrete function
signatures and call sites, with a recommendation, before implementing.
Ask rather than assume. Record the outcome in ROADMAP ("Settled in …")
in the same PR.
