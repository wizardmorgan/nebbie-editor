#!/usr/bin/env bash
# Install Nebbie Editor on macOS (copy .app to /Applications).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"

"${ROOT}/scripts/build.sh" --macos-bundle

APP_SRC="${BUILD}/nebbie-qt/nebbieedit.app"
APP_DST="/Applications/nebbieedit.app"

if [[ ! -d "${APP_SRC}" ]]; then
  echo "ERROR: ${APP_SRC} not found" >&2
  exit 1
fi

echo "==> Installing ${APP_DST}"
rm -rf "${APP_DST}"
cp -R "${APP_SRC}" "${APP_DST}"

echo "Done. Launch from Applications or:"
echo "  open /Applications/nebbieedit.app"
