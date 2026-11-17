#!/bin/sh
# SPDX-License-Identifier: MIT
# Copyright (c) 2026 Timothy Mitchell
#
# Footprint of the TreLVis library on a Cortex-M target: cross-compiles
# src/*.c (and the token template) with the strict warnings in several
# configurations, and prints flash and static RAM as Markdown. CI puts the
# output in the job summary; the README's "Footprint" table comes from it.
#
#   tests/footprint.sh path/to/lvgl [cortex-m4|cortex-m7]
#
# Run from the repository root. Environment: CC (arm-none-eabi-gcc), SIZE
# (arm-none-eabi-size), CPU_FLAGS (set from the CPU argument).
#
# Sizes are of the object files, before the linker drops functions a
# product doesn't call: an upper bound. LVGL, fonts and the project's
# tokens are not included.

set -eu

lvgl=${1:?usage: tests/footprint.sh path/to/lvgl [cortex-m4|cortex-m7]}
cpu=${2:-cortex-m4}
CC=${CC:-arm-none-eabi-gcc}
SIZE=${SIZE:-arm-none-eabi-size}

case $cpu in
    cortex-m4) cpu_flags="-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16" ;;
    cortex-m7) cpu_flags="-mcpu=cortex-m7 -mthumb -mfloat-abi=hard -mfpu=fpv5-d16" ;;
    *) echo "footprint.sh: unknown CPU '$cpu'" >&2; exit 2 ;;
esac
CPU_FLAGS=${CPU_FLAGS-$cpu_flags}

out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT

# LVGL's headers as system headers: their warnings are not TreLVis's.
flags="$CPU_FLAGS -std=c99 -Os -ffunction-sections -fdata-sections
       -Wall -Wextra -Wpedantic -Werror -Iinclude -isystem $lvgl/include"

# tests/config/minimal: RGB565, no widgets but the label, no default theme,
# TRLV_THEME_PARENT_NONE, TRLV_CUSTOM_COLOR_MAX 0.
minimal="-Itests/config/minimal -DLV_CONF_INCLUDE_SIMPLE"
# LVGL's defaults: every widget, the default theme as the parent theme,
# TRLV_CUSTOM_COLOR_MAX 4.
full="-DLV_CONF_SKIP"

# build NAME FLAGS...: compile the library into $out/NAME and print its
# total "text data bss". The template is compiled too, but not counted.
build() {
    dir=$out/$1
    shift
    mkdir -p "$dir"
    for src in src/*.c; do
        # shellcheck disable=SC2086
        $CC $flags "$@" -c "$src" -o "$dir/$(basename "$src" .c).o"
    done
    # shellcheck disable=SC2086
    $CC $flags "$@" -Itemplates -c templates/trlv_tokens_template.c -o "$dir/template.o"
    $SIZE -t "$dir"/trlv_*.o | tail -n 1 | awk '{ print $1, $2, $3 }'
}

# row LABEL NAME FLAGS...: one table row, flash = text + data, RAM = data + bss.
row() {
    label=$1
    shift
    # shellcheck disable=SC2046
    set -- $(build "$@")
    echo "| $label | $(($1 + $2)) | $(($2 + $3)) |"
}

echo "### TreLVis footprint, $cpu, -Os"
echo
echo "$($CC --version | head -n 1)"
echo
echo "| Configuration | Flash (bytes) | Static RAM (bytes) |"
echo "|---|---:|---:|"
row "Minimal \`lv_conf.h\`, theme on" minimal $minimal
row "Minimal \`lv_conf.h\`, theme off" minimal_no_theme $minimal -DTRLV_USE_THEME=0
row "Every widget, theme on" full $full
row "Every widget, theme off" full_no_theme $full -DTRLV_USE_THEME=0

# Each custom colour TRLV_CUSTOM_COLOR_MAX allows: 4 against 0, per colour.
set -- $(build full $full)
flash4=$(($1 + $2)) ram4=$(($2 + $3))
set -- $(build full_no_custom $full -DTRLV_CUSTOM_COLOR_MAX=0)
echo "| Each custom colour (\`TRLV_CUSTOM_COLOR_MAX\`) | $(((flash4 - $1 - $2) / 4)) | $(((ram4 - $2 - $3) / 4)) |"

echo
echo "Per object, every widget, theme on:"
echo
echo '```'
(cd "$out/full" && $SIZE trlv_*.o)
echo '```'
