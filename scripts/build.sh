#!/bin/bash
set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
SRC_DIR="$PROJECT_DIR/src"
BUILD_DIR="$PROJECT_DIR/build"
OUT_DIR="$PROJECT_DIR/modules/weird_drum"

# Cross-compiler prefix (Docker or local toolchain)
CC="${CROSS_PREFIX:-aarch64-linux-gnu-}g++"

echo "=== Building Weird Drum for Ableton Move (ARM64) ==="
echo "Compiler: $CC"

mkdir -p "$BUILD_DIR" "$OUT_DIR"

# Source files
SOURCES=(
    "$SRC_DIR/weird_drum_move.cpp"
)

# Compile each source
OBJECTS=()
for src in "${SOURCES[@]}"; do
    obj="$BUILD_DIR/$(basename "$src" | sed 's/\.\(cpp\|cc\|c\)$/.o/')"
    echo "  CC $src"
    $CC -c "$src" -o "$obj" \
        -I"$SRC_DIR" \
        -std=c++17 \
        -O2 -fPIC -ffast-math \
        -march=armv8-a -mtune=cortex-a72 \
        -fomit-frame-pointer -fno-stack-protector \
        -fno-exceptions -fno-rtti \
        -fno-tree-loop-distribute-patterns \
        -DNDEBUG \
        -Wall -Wno-unused-variable -Wno-unused-but-set-variable
    OBJECTS+=("$obj")
done

# Link shared library
echo "  LD weird_drum.so"
$CC -shared -o "$OUT_DIR/weird_drum.so" "${OBJECTS[@]}" -lm

# Copy module files
cp "$PROJECT_DIR/module.json" "$OUT_DIR/"
cp "$PROJECT_DIR/ui_chain.js" "$OUT_DIR/"

echo "=== Build complete: $OUT_DIR/weird_drum.so ==="
ls -la "$OUT_DIR/"
