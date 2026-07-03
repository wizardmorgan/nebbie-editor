#!/usr/bin/env bash
# Configura git per usare il PAT personale quando l'app Cursor non ha push sul repo.
# Il secret va aggiunto su https://cursor.com/dashboard → Cloud Agents → Secrets
# Nome consigliato: WIZARDMORGAN_GITHUB_PAT  (NON usare GH_TOKEN)

set -euo pipefail

PAT="${WIZARDMORGAN_GITHUB_PAT:-}"

if [[ -z "$PAT" ]]; then
  echo "WIZARDMORGAN_GITHUB_PAT non impostato: git userà il token cursor[bot] (può fallire su nebbie-editor)."
  exit 0
fi

git config --global credential.helper ""
git config --global --add credential.helper \
  '!f() { echo "username=wizardmorgan"; echo "password='"$PAT"'"; }; f'

echo "Git configurato con WIZARDMORGAN_GITHUB_PAT per push come wizardmorgan."
