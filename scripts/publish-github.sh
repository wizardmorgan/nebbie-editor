#!/usr/bin/env bash
set -euo pipefail

# Pubblica il branch corrente su wizardmorgan/nebbie-editor.
# NOTA: l'account cursor[bot] (agent cloud) spesso NON ha permesso di push
# sui repo personali. In quel caso usa le istruzioni in docs/PUBBLICAZIONE.md.

REPO="wizardmorgan/nebbie-editor"
BRANCH="${1:-cursor/nebbie-editor-initial-c774}"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "Esegui questo script dalla root del progetto nebbie-editor." >&2
  exit 1
fi

if ! gh repo view "$REPO" >/dev/null 2>&1; then
  echo "Repository $REPO non trovato." >&2
  echo "Crealo vuoto su https://github.com/new (owner: wizardmorgan, nome: nebbie-editor)." >&2
  exit 1
fi

echo "Repository $REPO trovato."

if ! git remote get-url origin >/dev/null 2>&1; then
  git remote add origin "https://github.com/$REPO.git"
fi

echo "Push di $BRANCH verso origin..."
set +e
PUSH_OUTPUT="$(git push -u origin "$BRANCH" 2>&1)"
PUSH_STATUS=$?
set -e

if [[ $PUSH_STATUS -eq 0 ]]; then
  echo
  echo "Pubblicato: https://github.com/$REPO/tree/$BRANCH"
  exit 0
fi

echo "$PUSH_OUTPUT" >&2
echo >&2

if grep -qE '403|Permission.*denied|cursor\[bot\]' <<<"$PUSH_OUTPUT"; then
  cat >&2 <<'EOF'
════════════════════════════════════════════════════════════════════
Push rifiutato: cursor[bot] non ha accesso in scrittura a QUESTO repo.
════════════════════════════════════════════════════════════════════

Se l'app Cursor è già in Read/Write su GitHub, il problema è quasi sempre
che nebbie-editor non è nel token runtime dell'agent (repo nuovo o cache).

Verifica: lo stesso agent riesce a pushare su altri repo tuoi (es. DikuEdit)
ma non su nebbie-editor.

Cosa provare:
  1. GitHub → Settings → Installations → Cursor → Configure
  2. Repository access: seleziona ESPLICITAMENTE "nebbie-editor"
  3. cursor.com/dashboard → riconnetti GitHub
  4. Rilancia questo script

Se non basta, push dal tuo PC con il tuo account GitHub:

       git push -u origin cursor/nebbie-editor-initial-c774

Guida completa: docs/PUBBLICAZIONE.md
EOF
  exit 1
fi

exit "$PUSH_STATUS"
