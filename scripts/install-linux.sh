#!/usr/bin/env bash
# Install Nebbie Editor on Linux via cmake --install.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
PREFIX="${1:-/usr/local}"

"${ROOT}/scripts/build.sh"

echo "==> cmake --install ${BUILD} --prefix ${PREFIX}"
cmake --install "${BUILD}" --prefix "${PREFIX}"

echo ""
echo "Installed:"
echo "  CLI:  ${PREFIX}/bin/nebbiedit"
echo "  GUI:  ${PREFIX}/bin/nebbieedit"
echo "  Menu: ${PREFIX}/share/applications/nebbieedit.desktop"
echo ""
echo "Assicurati che ${PREFIX}/bin sia nel PATH."
