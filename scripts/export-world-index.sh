#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LIB="${1:-$ROOT/tests/fixtures}"
OUT="${2:-/tmp/world-index.json}"
BIN="${3:-$ROOT/build/nebbiedit/nebbiedit}"

if [[ ! -x "$BIN" ]]; then
  echo "Build nebbiedit first: cmake --build build" >&2
  exit 1
fi

"$BIN" world-index export "$LIB" "$OUT"
echo "Wrote $OUT"

if [[ -n "${COORDINATOR_UPLOAD_URL:-}" ]]; then
  curl -fsS -T "$OUT" "$COORDINATOR_UPLOAD_URL"
  echo "Uploaded to $COORDINATOR_UPLOAD_URL"
fi
