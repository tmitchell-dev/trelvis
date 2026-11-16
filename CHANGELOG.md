# Changelog

All notable changes to TreLVis are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).
While the version is 0.x, minor releases may contain breaking changes.

## [Unreleased]

### Added

- Project scaffold: CMake target `trelvis`, `trelvis.mk` for non-CMake builds,
  `trlv_conf.h` configuration mechanism with defaults in `trlv_conf_default.h`,
  `trlv_version.h` with a compile-time check for LVGL 9.6+ (9.x).
- Placeholder public headers (`trelvis.h` and modules), compiling from C99 and C++17.
- Provisional MD3 token types in `trlv_tokens.h`: spacing, radius, size,
  elevation and type-scale enums, and the colour and layout token structs.
- CI: GCC and Clang builds against LVGL 9.6 with warnings as errors, C++17
  include test, non-CMake build, allocation and SPDX checks; weekly build
  against LVGL master; tag-driven release workflow.
