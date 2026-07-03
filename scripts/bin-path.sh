#!/usr/bin/env bash
# Print paths to built binaries (handles macOS .app bundles).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${1:-$ROOT/build}"

nebbiedit_cli="$BUILD_DIR/nebbiedit/nebbiedit"
nebbieedit_gui="$BUILD_DIR/nebbie-qt/nebbieedit"
if [[ -x "$BUILD_DIR/nebbie-qt/nebbieedit.app/Contents/MacOS/nebbieedit" ]]; then
    nebbieedit_gui="$BUILD_DIR/nebbie-qt/nebbieedit.app/Contents/MacOS/nebbieedit"
fi

case "${2:-all}" in
    nebbiedit) echo "$nebbiedit_cli" ;;
    nebbieedit) echo "$nebbieedit_gui" ;;
    all)
        echo "nebbiedit=$nebbiedit_cli"
        echo "nebbieedit=$nebbieedit_gui"
        ;;
    *) echo "Usage: $0 [build-dir] [nebbiedit|nebbieedit|all]" >&2; exit 1 ;;
esac
