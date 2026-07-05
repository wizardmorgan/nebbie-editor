#!/usr/bin/env bash
# Build a macOS disk image (.dmg) with nebbieedit.app and nebbiedit CLI.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
DIST="${ROOT}/dist"
STAGING="${ROOT}/dist/dmg-staging"
VERSION=""
RUN_BUILD=1

usage() {
    cat <<'EOF'
Usage: ./scripts/package-dmg.sh [options]

Builds dist/nebbie-editor_<version>_macos.dmg containing:
  - nebbieedit.app
  - bin/nebbiedit (CLI)
  - Applications symlink (drag-and-drop install)

Options:
  --no-build       Skip ./scripts/build.sh --macos-bundle
  -h, --help       Show this help

Requires: macOS with hdiutil and a Qt 6 build (--macos-bundle).
EOF
}

read_version() {
    VERSION="$(sed -n 's/^project(nebbie-editor VERSION \([^ )]*\).*/\1/p' "${ROOT}/CMakeLists.txt")"
    if [[ -z "${VERSION}" ]]; then
        VERSION="0.0.0"
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-build) RUN_BUILD=0; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

if [[ "$(uname -s)" != "Darwin" ]]; then
    echo "ERROR: package-dmg.sh must run on macOS (needs hdiutil)." >&2
    exit 1
fi

if ! command -v hdiutil >/dev/null 2>&1; then
    echo "ERROR: hdiutil not found." >&2
    exit 1
fi

read_version
mkdir -p "${DIST}"

if [[ "${RUN_BUILD}" -eq 1 ]]; then
    export CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-$(brew --prefix qt@6 2>/dev/null || true)}"
    "${ROOT}/scripts/build.sh" --macos-bundle
fi

APP_SRC="${BUILD}/nebbie-qt/nebbieedit.app"
CLI_SRC="${BUILD}/nebbiedit/nebbiedit"

if [[ ! -d "${APP_SRC}" ]]; then
    echo "ERROR: ${APP_SRC} not found. Run: ./scripts/build.sh --macos-bundle" >&2
    exit 1
fi
if [[ ! -x "${CLI_SRC}" ]]; then
    echo "ERROR: ${CLI_SRC} not found." >&2
    exit 1
fi

echo "==> Preparing DMG staging"
rm -rf "${STAGING}"
mkdir -p "${STAGING}/bin"
cp -R "${APP_SRC}" "${STAGING}/"
cp "${CLI_SRC}" "${STAGING}/bin/"
ln -sf /Applications "${STAGING}/Applications"

DMG_FILE="${DIST}/nebbie-editor_${VERSION}_macos.dmg"
rm -f "${DMG_FILE}"

echo "==> Creating ${DMG_FILE}"
hdiutil create \
    -volname "Nebbie Editor" \
    -srcfolder "${STAGING}" \
    -ov \
    -format UDZO \
    "${DMG_FILE}"

rm -rf "${STAGING}"

echo ""
echo "Disk image created:"
echo "  ${DMG_FILE}"
ls -lh "${DMG_FILE}"
echo ""
echo "Users can drag nebbieedit.app to Applications."
