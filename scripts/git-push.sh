#!/usr/bin/env bash
set -euo pipefail

BRANCH="${1:-$(git branch --show-current)}"
REMOTE="${2:-origin}"

if [[ -n "${WIZARDMORGAN_GITHUB_PAT:-}" ]]; then
  bash "$(dirname "$0")/../.cursor/setup-git-auth.sh"
fi

echo "Push di $BRANCH verso $REMOTE..."
if git push -u "$REMOTE" "$BRANCH"; then
  echo "OK: https://github.com/wizardmorgan/nebbie-editor/tree/$BRANCH"
  exit 0
fi

cat >&2 <<'EOF'

════════════════════════════════════════════════════════════════════
Push fallito.
════════════════════════════════════════════════════════════════════

Soluzione definitiva — PAT nei Secrets Cursor:

  1. GitHub → Settings → Developer settings → Fine-grained tokens
     Crea token su repository "nebbie-editor" con permesso Contents: Read and write

  2. Cursor → https://cursor.com/dashboard → Cloud Agents → Secrets
     Nome: WIZARDMORGAN_GITHUB_PAT
     Valore: il token (NON chiamarlo GH_TOKEN)

  3. Riavvia l'agent cloud su questo repo

  4. Rilancia: ./scripts/git-push.sh

Guida completa: docs/GIT-PUSH-DEFINITIVO.md
EOF
exit 1
