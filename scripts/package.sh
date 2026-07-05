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
    *)
        cat >&2 <<'EOF'
ERROR: Windows is not supported yet.

Nebbie Editor targets Linux and macOS only.
On Linux:   ./scripts/package-deb.sh
On macOS:   ./scripts/package-dmg.sh
EOF
        exit 1
        ;;
esac
