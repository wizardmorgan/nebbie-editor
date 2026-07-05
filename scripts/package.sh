#!/usr/bin/env bash
# Build platform packages (.deb on Linux, .dmg on macOS).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"

case "$(uname -s)" in
    Linux)
        exec "${ROOT}/scripts/package-deb.sh" "$@"
        ;;
    Darwin)
        exec "${ROOT}/scripts/package-dmg.sh" "$@"
        ;;
    MINGW*|MSYS*|CYGWIN*|Windows_NT)
        echo "On Windows use: .\\scripts\\package-windows.ps1" >&2
        exit 1
        ;;
    *)
        cat >&2 <<'EOF'
ERROR: Unknown platform.

Linux:   ./scripts/package-deb.sh
macOS:   ./scripts/package-dmg.sh
Windows: .\scripts\package-windows.ps1
EOF
        exit 1
        ;;
esac
