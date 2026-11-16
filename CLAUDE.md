# CLAUDE.md

Guidance for AI coding agents. The contributor guide applies in full and is
imported here:

@CONTRIBUTING.md

## Project in one paragraph

TreLVis (`trelvis`, C prefix `trlv_` / `TRLV_`) is a small C99 utility layer
on LVGL 9.6. It provides Figma/CSS-style layout primitives, Material Design 3
(MD3) token types, and an MD3 theme built on LVGL's theme and style engine.
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
- [docs/figma-handoff.md](docs/figma-handoff.md): designer conventions.

If an implementation decision changes the design, update the doc in the same PR.

## Current status

Milestones 1 (scaffold), 2 (vertical slice) and 3 (API review) are done.
The slice implemented the token template, `trlv_init`, box, row, column,
gap, pad, fill/hug and text, the demo screen `trlv_demo_create(parent)` in
`examples/demo`, and headless layout tests. TreLVis ships no simulator; host
projects run the demo in their own.

The API review wrote the full v1 public headers. They are the source of
truth for signatures, and `tests/cpp_include_test.cpp` pins every one.
Functions not built yet say so in their doc comment ("Not implemented yet
(milestone N)"); remove that note when you implement one. The review's
decisions are in ROADMAP "Settled in the API review".

**Next is milestone 4: tokens and theme.** Implement against the approved
headers; an API change needs the docs, the include test and the changelog
updated with it.

Work is tracked in GitHub issues, one per item, under milestones
`M<N>: …` with a tracking issue each. Each PR says `Closes #N` for the
issues it completes, and the tracking issue is kept current.

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
- Work on a branch, never on `main`. Don't commit, push or open PRs unless asked.
- Nothing company- or product-specific, and no secrets, anywhere, including
  commit messages and PR text.
