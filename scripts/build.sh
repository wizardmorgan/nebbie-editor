#!/usr/bin/env bash
# Cross-platform configure and build for Linux and macOS.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="$ROOT/build"
BUILD_TYPE=Release
WITH_QT=1
MACOS_BUNDLE=0
RUN_TESTS=0
EXTRA_CMAKE_ARGS=()

usage() {
    cat <<'EOF'
Usage: ./scripts/build.sh [options] [-- extra cmake args]

Options:
  --debug          Build Debug instead of Release
  --no-qt          Skip Qt GUI (nebbieedit)
  --macos-bundle   Build nebbieedit as .app on macOS
  --test           Run ctest after build
  -h, --help       Show this help

Environment:
  CMAKE_PREFIX_PATH   Qt 6 location (required on macOS if Qt is not auto-detected)
  CXX                 C++ compiler (default: platform default)
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --debug) BUILD_TYPE=Debug; shift ;;
        --no-qt) WITH_QT=0; shift ;;
        --macos-bundle) MACOS_BUNDLE=1; shift ;;
        --test) RUN_TESTS=1; shift ;;
        -h|--help) usage; exit 0 ;;
        --)
            shift
            EXTRA_CMAKE_ARGS+=("$@")
            break
            ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

CMAKE_ARGS=(
    -S "$ROOT"
    -B "$BUILD_DIR"
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
    -DNEBBIE_BUILD_QT="$WITH_QT"
)

if [[ "$(uname -s)" == "Darwin" ]]; then
    CMAKE_ARGS+=(-DNEBBIE_MACOS_BUNDLE="$MACOS_BUNDLE")
    if [[ -z "${CMAKE_PREFIX_PATH:-}" ]] && command -v brew >/dev/null 2>&1; then
        if brew --prefix qt@6 >/dev/null 2>&1; then
            export CMAKE_PREFIX_PATH="$(brew --prefix qt@6)"
        fi
    fi
fi

if [[ -n "${CXX:-}" ]]; then
    CMAKE_ARGS+=(-DCMAKE_CXX_COMPILER="$CXX")
fi

if [[ ${#EXTRA_CMAKE_ARGS[@]} -gt 0 ]]; then
    CMAKE_ARGS+=("${EXTRA_CMAKE_ARGS[@]}")
fi

echo "==> cmake ${CMAKE_ARGS[*]}"
cmake "${CMAKE_ARGS[@]}"

echo "==> cmake --build $BUILD_DIR"
cmake --build "$BUILD_DIR"

if [[ "$RUN_TESTS" -eq 1 ]]; then
    echo "==> ctest"
    ctest --test-dir "$BUILD_DIR" --output-on-failure
fi

echo
echo "Binaries:"
echo "  CLI:  $BUILD_DIR/nebbiedit/nebbiedit"
if [[ "$WITH_QT" -eq 1 ]]; then
    if [[ -x "$BUILD_DIR/nebbie-qt/nebbieedit.app/Contents/MacOS/nebbieedit" ]]; then
        echo "  GUI:  $BUILD_DIR/nebbie-qt/nebbieedit.app"
    elif [[ -x "$BUILD_DIR/nebbie-qt/nebbieedit" ]]; then
        echo "  GUI:  $BUILD_DIR/nebbie-qt/nebbieedit"
    else
        echo "  GUI:  (Qt not built — install Qt 6 and re-run without --no-qt)"
    fi
fi
