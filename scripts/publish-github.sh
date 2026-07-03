#!/usr/bin/env bash
set -euo pipefail

# Crea il repository GitHub wizardmorgan/nebbie-editor e pubblica il branch corrente.
# L'integrazione Cursor non ha permesso di creare repo nuovi via API: questo script
# richiede che tu crei il repo vuoto una volta dal browser o con un PAT personale.

REPO="wizardmorgan/nebbie-editor"
BRANCH="${1:-cursor/nebbie-editor-initial-c774}"

if ! git rev-parse --is-inside-work-tree >/dev/null 2>&1; then
  echo "Esegui questo script dalla root del progetto nebbie-editor." >&2
  exit 1
fi

if gh repo view "$REPO" >/dev/null 2>&1; then
  echo "Repository $REPO trovato."
else
  echo "Repository $REPO non esiste ancora."
  echo
  echo "Crealo manualmente:"
  echo "  1. Apri https://github.com/new"
  echo "  2. Owner: wizardmorgan"
  echo "  3. Nome: nebbie-editor"
  echo "  4. Public, senza README/.gitignore (repo vuoto)"
  echo
  echo "Oppure, con un PAT con scope repo:"
  echo "  gh repo create $REPO --public --description \"Portable Nebbie Arcane world editor\""
  echo
  read -r -p "Premi Invio dopo aver creato il repo..."
fi

if ! git remote get-url origin >/dev/null 2>&1; then
  git remote add origin "https://github.com/$REPO.git"
fi

git push -u origin "$BRANCH"

echo
echo "Pubblicato: https://github.com/$REPO/tree/$BRANCH"
echo "Apri una PR verso main/master quando il branch di default è pronto."
