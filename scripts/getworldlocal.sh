#!/usr/bin/env bash
# Mirror of NebbieArcane/Server/getworldlocal: copy myst.* into mudroot/lib.
#
# Usage:
#   ./scripts/getworldlocal.sh [SERVER_ROOT]
#
# SERVER_ROOT defaults to vendor/nebbietest (clone via fetch-test-data.sh).
# Expects myst.zon, myst.wld, myst.mob, myst.obj, ... at SERVER_ROOT root
# or already under SERVER_ROOT/mudroot/lib (no-op copy from lib to lib).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
SERVER_ROOT="${1:-${ROOT}/vendor/nebbietest}"

if [[ ! -d "${SERVER_ROOT}" ]]; then
    echo "ERROR: server root not found: ${SERVER_ROOT}" >&2
    echo "Run: ./scripts/fetch-test-data.sh" >&2
    exit 1
fi

LIB="${SERVER_ROOT}/mudroot/lib"
mkdir -p "${LIB}"

shopt -s nullglob
MYST_SRC=("${SERVER_ROOT}"/myst.*)
if [[ ${#MYST_SRC[@]} -eq 0 ]]; then
    echo "ERROR: no myst.* files in ${SERVER_ROOT}" >&2
    exit 1
fi

echo "==> getworldlocal: ${SERVER_ROOT} -> ${LIB}"
for src in "${MYST_SRC[@]}"; do
    base="$(basename "${src}")"
  # Skip runtime pid file if present at repo root.
    if [[ "${base}" == "myst.pid" ]]; then
        continue
    fi
    cp -v "${src}" "${LIB}/${base}"
done

for subdir in objects mobiles rooms zones; do
    mkdir -p "${LIB}/${subdir}"
done

echo "Sample lib ready: ${LIB}"
