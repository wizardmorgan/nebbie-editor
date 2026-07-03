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

# Il remote clonato dall'agent ha spesso x-access-token nell'URL (cursor[bot]).
# Senza questo reset, git ignora il PAT e il push fallisce comunque.
if git rev-parse --is-inside-work-tree &>/dev/null; then
  origin_url="$(git remote get-url origin 2>/dev/null || true)"
  if [[ "$origin_url" == *"x-access-token"* || "$origin_url" == *"cursor"* ]]; then
    git remote set-url origin "https://github.com/wizardmorgan/nebbie-editor.git"
    echo "Remote origin reimpostato senza token cursor[bot]."
  fi
fi

echo "Git configurato con WIZARDMORGAN_GITHUB_PAT (push come wizardmorgan)."
