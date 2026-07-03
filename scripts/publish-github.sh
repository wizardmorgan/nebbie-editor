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
Push rifiutato: cursor[bot] non ha permesso di scrivere sul tuo repo.
════════════════════════════════════════════════════════════════════

Soluzione consigliata — push dal TUO computer con il TUO account GitHub:

  1. Apri il progetto nebbie-editor in Cursor (desktop) oppure clona il bundle:
       git clone nebbie-editor.bundle nebbie-editor -b cursor/nebbie-editor-initial-c774
     (il bundle si genera con: git bundle create nebbie-editor.bundle --all)

  2. Collega il remote (se manca):
       git remote add origin https://github.com/wizardmorgan/nebbie-editor.git

  3. Push con le tue credenziali GitHub (browser o PAT):
       git push -u origin cursor/nebbie-editor-initial-c774

  Con Personal Access Token (scope repo):
       git push https://wizardmorgan:<TOKEN>@github.com/wizardmorgan/nebbie-editor.git cursor/nebbie-editor-initial-c774

Soluzione alternativa — dare accesso all'app Cursor su GitHub:

  1. https://github.com/settings/installations
  2. Apri "Cursor" → Configure
  3. Repository access: aggiungi "nebbie-editor" (o All repositories)
  4. Verifica permesso "Contents" = Read and write
  5. Rilancia questo script

Guida completa: docs/PUBBLICAZIONE.md
EOF
  exit 1
fi

exit "$PUSH_STATUS"
