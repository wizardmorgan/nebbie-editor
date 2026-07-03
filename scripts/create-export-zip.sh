#!/usr/bin/env bash
# Crea archivio per trasferimento al PC o altro agent.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

BRANCH="$(git branch --show-current)"
STAMP="$(date -u +%Y%m%d-%H%M%S)"
WORK="/tmp/nebbie-editor-export-$$"
ARTIFACTS="/opt/cursor/artifacts/nebbie-editor-export"
ZIP_NAME="nebbie-editor-export-${STAMP}.zip"

mkdir -p "$WORK" "$ARTIFACTS"

echo "Esporto branch $BRANCH..."

# Sorgenti + .git (niente build né vendor pesante)
mkdir -p "$WORK/nebbie-editor"
tar -C "$ROOT" \
  --exclude=build \
  --exclude='cmake-build-*' \
  --exclude=.cache \
  --exclude=vendor/nebbietest \
  --exclude='dist/*.bundle' \
  -cf - . | tar -C "$WORK/nebbie-editor" -xf -

# Bundle con solo i commit non su GitHub (se esiste origin)
if git rev-parse --verify "origin/$BRANCH" &>/dev/null; then
  AHEAD="$(git rev-list --count "origin/$BRANCH..HEAD" || echo 0)"
  if [[ "$AHEAD" -gt 0 ]]; then
    git bundle create "$WORK/nebbie-editor-unpushed.bundle" "origin/$BRANCH..HEAD"
  fi
fi

# README per chi scarica
cat > "$WORK/LEGGIMI.txt" <<EOF
Nebbie Editor — archivio export ($STAMP)
======================================

CONTENUTO
---------
- nebbie-editor/     progetto completo con storico git (.git incluso)
- nebbie-editor-unpushed.bundle   (se presente) solo commit non ancora su GitHub

SUL PC — apri e pusha
---------------------
  unzip $ZIP_NAME
  cd nebbie-editor
  git log --oneline -6
  git remote set-url origin https://github.com/wizardmorgan/nebbie-editor.git
  git push -u origin cursor/nebbie-editor-initial-c774

ALTERNATIVA — solo commit mancanti (repo già clonato da GitHub)
----------------------------------------------------------------
  git pull /percorso/nebbie-editor-unpushed.bundle cursor/nebbie-editor-initial-c774
  git push -u origin cursor/nebbie-editor-initial-c774

Commit locali attuali:
$(git log --oneline -8 2>/dev/null || true)
EOF

(
  cd "$WORK"
  zip -rq "$ARTIFACTS/$ZIP_NAME" .
)

# Link stabile per l'ultima versione
ln -sf "$ZIP_NAME" "$ARTIFACTS/nebbie-editor-export.zip"

rm -rf "$WORK"

echo "OK: $ARTIFACTS/$ZIP_NAME"
ls -lh "$ARTIFACTS/$ZIP_NAME" "$ARTIFACTS/nebbie-editor-export.zip"
echo ""
echo "Commit inclusi:"
git log --oneline -6
