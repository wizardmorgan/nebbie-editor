#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
TARGET="$ROOT/vendor/nebbietest"

if [[ -d "$TARGET/.git" ]]; then
  git -C "$TARGET" pull --ff-only origin develop
else
  mkdir -p "$ROOT/vendor"
  git clone --depth 1 --branch develop https://github.com/wizardmorgan/nebbietest.git "$TARGET"
fi

echo "Test data ready at $TARGET/mudroot/lib"
