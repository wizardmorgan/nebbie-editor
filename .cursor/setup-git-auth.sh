#!/usr/bin/env bash
# Configura git per usare il PAT personale quando l'app Cursor non ha push sul repo.
# Secret: https://cursor.com/dashboard → Cloud Agents → Secrets
# Nome: WIZARDMORGAN_GITHUB_PAT  (NON usare GH_TOKEN)

set -euo pipefail

PAT="${WIZARDMORGAN_GITHUB_PAT:-}"

if [[ -z "$PAT" ]]; then
  echo "WIZARDMORGAN_GITHUB_PAT non impostato: git userà cursor[bot] (può fallire su nebbie-editor)."
  exit 0
fi

git config --global credential.helper store
printf 'https://wizardmorgan:%s@github.com\n' "$PAT" > "${HOME}/.git-credentials"
chmod 600 "${HOME}/.git-credentials"

echo "Git configurato con WIZARDMORGAN_GITHUB_PAT (push come wizardmorgan)."
