#!/usr/bin/env bash
# Crea un bundle git con i commit locali non ancora su GitHub.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

BRANCH="$(git branch --show-current)"
UPSTREAM="origin/$BRANCH"

if ! git rev-parse --verify "$UPSTREAM" &>/dev/null; then
  echo "Branch remoto $UPSTREAM non trovato. Esegui: git fetch origin"
  exit 1
fi

AHEAD="$(git rev-list --count "$UPSTREAM..HEAD")"
if [[ "$AHEAD" -eq 0 ]]; then
  echo "Nessun commit da esportare: locale e GitHub sono allineati."
  exit 0
fi

mkdir -p dist
OUT="dist/nebbie-editor-unpushed.bundle"

git bundle create "$OUT" "$UPSTREAM..HEAD"
git bundle verify "$OUT" >/dev/null

echo "Bundle creato: $OUT ($(du -h "$OUT" | cut -f1))"
echo ""
echo "Commit inclusi ($AHEAD):"
git log --oneline "$UPSTREAM..HEAD"
echo ""
echo "Per importare nel nuovo agent (dopo git clone/fetch):"
echo "  git pull $OUT $BRANCH"
echo "  ./scripts/git-push.sh"
