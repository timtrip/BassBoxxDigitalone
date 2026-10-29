#!/bin/zsh
set -e

PROJECT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

if command -v cmake >/dev/null 2>&1; then
    CMAKE_BIN="$(command -v cmake)"
elif [ -x "/Applications/CMake.app/Contents/bin/cmake" ]; then
    CMAKE_BIN="/Applications/CMake.app/Contents/bin/cmake"
else
    echo "CMake was not found. Install CMake.app, then run this command again."
    exit 1
fi

if ! command -v xcrun >/dev/null 2>&1 || ! xcrun --find clang++ >/dev/null 2>&1; then
    echo "Apple Command Line Tools are required. Run: xcode-select --install"
    exit 1
fi

echo "Configuring Bass Boxx Digital One..."
"$CMAKE_BIN" -S "$PROJECT_DIR" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release

echo "Building VST3 and standalone app..."
"$CMAKE_BIN" --build "$BUILD_DIR" --config Release --parallel

PLUGIN_PATH="$(find "$BUILD_DIR" -type d -name "Bass Boxx Digital One.vst3" -print -quit)"
if [ -z "$PLUGIN_PATH" ]; then
    echo "Build finished, but the VST3 bundle was not found under: $BUILD_DIR"
    exit 1
fi

VST3_DIR="$HOME/Library/Audio/Plug-Ins/VST3"
mkdir -p "$VST3_DIR"
ditto "$PLUGIN_PATH" "$VST3_DIR/Bass Boxx Digital One.vst3"

echo "SUCCESS: Bass Boxx Digital One VST3 built and installed."
echo "Installed at: $VST3_DIR/Bass Boxx Digital One.vst3"
echo "Rescan plug-ins in Ableton Live."
