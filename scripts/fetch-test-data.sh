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

LIB="$TARGET/mudroot/lib"
for file in myst.zon myst.wld myst.mob myst.obj myst.shp myst.spe; do
  if [[ -f "$TARGET/$file" && ! -f "$LIB/$file" ]]; then
    cp "$TARGET/$file" "$LIB/$file"
    echo "Installed $file -> mudroot/lib/"
  fi
done

"$ROOT/scripts/getworldlocal.sh" "$TARGET"
