#!/usr/bin/env bash
# Extract world data from the production Nebbie server and sync to nebbie.wizmorgan.it.
#
# Workflow (mirrors Server/getworld + getworldlocal, extended for staging):
#   1. fetch   — rsync myst.* (+ overlay dirs) from production SSH host
#   2. manifest — nebbiedit info/validate + JSON summary
#   3. push    — rsync local staging to STAGING_SSH_HOST (nebbie.wizmorgan.it)
#   4. sync    — fetch + manifest + push
#
# Usage:
#   cp scripts/production-world.env.example scripts/production-world.env
#   $EDITOR scripts/production-world.env
#   ./scripts/export-production-world.sh sync
#
# Requires: rsync, ssh, jq (optional for JSON), nebbiedit in PATH or build/
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
ENV_FILE="${ROOT}/scripts/production-world.env"

usage() {
    cat <<'EOF'
Usage: ./scripts/export-production-world.sh <command>

Commands:
  fetch      Download myst.* (and overlay dirs) from production server
  manifest   Run nebbiedit info/validate on local staging; write world-manifest.json
  push       Upload local staging lib to nebbie.wizmorgan.it (or STAGING_SSH_HOST)
  sync       fetch + manifest + push

Configuration: scripts/production-world.env (see production-world.env.example)

Environment overrides (optional):
  PROD_SSH_USER, PROD_SSH_HOST, PROD_LIB_REMOTE
  STAGING_SSH_USER, STAGING_SSH_HOST, STAGING_LIB_REMOTE
  LOCAL_STAGING_DIR, FETCH_OVERLAY_DIRS
EOF
}

load_env() {
    if [[ -f "${ENV_FILE}" ]]; then
        # shellcheck disable=SC1090
        set -a
        source "${ENV_FILE}"
        set +a
    else
        echo "WARN: ${ENV_FILE} not found; using defaults / environment only" >&2
    fi

    PROD_SSH_USER="${PROD_SSH_USER:-nebbie}"
    PROD_SSH_HOST="${PROD_SSH_HOST:-nebbie.nebbie.it}"
    PROD_LIB_REMOTE="${PROD_LIB_REMOTE:-Run/release/lib}"
    STAGING_SSH_USER="${STAGING_SSH_USER:-deploy}"
    STAGING_SSH_HOST="${STAGING_SSH_HOST:-nebbie.wizmorgan.it}"
    STAGING_LIB_REMOTE="${STAGING_LIB_REMOTE:-/var/www/nebbie/mudroot/lib}"
    STAGING_MANIFEST_REMOTE="${STAGING_MANIFEST_REMOTE:-/var/www/nebbie/world-manifest.json}"
    LOCAL_STAGING_DIR="${LOCAL_STAGING_DIR:-dist/production-world}"
    FETCH_OVERLAY_DIRS="${FETCH_OVERLAY_DIRS:-1}"
    RSYNC_SSH_OPTS="${RSYNC_SSH_OPTS:--o StrictHostKeyChecking=accept-new}"

    LOCAL_LIB="${ROOT}/${LOCAL_STAGING_DIR}/lib"
    LOCAL_MANIFEST="${ROOT}/${LOCAL_STAGING_DIR}/world-manifest.json"
}

find_nebbiedit() {
    local candidates=(
        "${ROOT}/build/nebbiedit/nebbiedit"
        "${ROOT}/build/nebbiedit/Release/nebbiedit.exe"
    )
    local c
    for c in "${candidates[@]}"; do
        if [[ -x "${c}" ]]; then
            echo "${c}"
            return 0
        fi
    done
    if command -v nebbiedit >/dev/null 2>&1; then
        command -v nebbiedit
        return 0
    fi
    return 1
}

cmd_fetch() {
    mkdir -p "${LOCAL_LIB}"
    local remote="${PROD_SSH_USER}@${PROD_SSH_HOST}:${PROD_LIB_REMOTE}/"

    echo "==> Fetching myst.* from ${remote}"
    rsync -avz -e "ssh ${RSYNC_SSH_OPTS}" \
        --include='myst.???' \
        --include='myst.????' \
        --exclude='*' \
        "${remote}" "${LOCAL_LIB}/"

    if [[ "${FETCH_OVERLAY_DIRS}" == "1" ]]; then
        echo "==> Fetching overlay directories"
        for sub in objects mobiles rooms zones; do
            rsync -avz -e "ssh ${RSYNC_SSH_OPTS}" \
                "${remote}${sub}/" "${LOCAL_LIB}/${sub}/" || true
        done
    fi

    echo "Fetched to ${LOCAL_LIB}"
    ls -1 "${LOCAL_LIB}"/myst.* 2>/dev/null | sed 's/^/  /' || echo "  (no myst.* yet)"
}

cmd_manifest() {
    if [[ ! -d "${LOCAL_LIB}" ]]; then
        echo "ERROR: ${LOCAL_LIB} missing. Run: $0 fetch" >&2
        exit 1
    fi

    local cli
  if ! cli="$(find_nebbiedit)"; then
        echo "ERROR: nebbiedit not found. Build the project or install the package." >&2
        exit 1
    fi

    mkdir -p "$(dirname "${LOCAL_MANIFEST}")"
    local info_out validate_out ts
    ts="$(date -u +%Y-%m-%dT%H:%M:%SZ)"

    info_out="$(mktemp)"
    validate_out="$(mktemp)"
    "${cli}" info "${LOCAL_LIB}" >"${info_out}" 2>&1 || true
    "${cli}" validate "${LOCAL_LIB}" >"${validate_out}" 2>&1
    local validate_rc=$?

    if command -v jq >/dev/null 2>&1; then
        jq -n \
            --arg generated_at "${ts}" \
            --arg source_host "${PROD_SSH_HOST}" \
            --arg source_path "${PROD_LIB_REMOTE}" \
            --arg staging_host "${STAGING_SSH_HOST}" \
            --arg staging_path "${STAGING_LIB_REMOTE}" \
            --arg info "$(cat "${info_out}")" \
            --arg validate "$(cat "${validate_out}")" \
            --argjson validate_ok "$([[ ${validate_rc} -eq 0 ]] && echo true || echo false)" \
            '{
                generated_at: $generated_at,
                source: {host: $source_host, lib_path: $source_path},
                destination: {host: $staging_host, lib_path: $staging_path},
                validate_ok: $validate_ok,
                nebbiedit_info: $info,
                nebbiedit_validate: $validate
            }' >"${LOCAL_MANIFEST}"
    else
        cat >"${LOCAL_MANIFEST}" <<EOF
{
  "generated_at": "${ts}",
  "source": {"host": "${PROD_SSH_HOST}", "lib_path": "${PROD_LIB_REMOTE}"},
  "destination": {"host": "${STAGING_SSH_HOST}", "lib_path": "${STAGING_LIB_REMOTE}"},
  "validate_ok": $([[ ${validate_rc} -eq 0 ]] && echo true || echo false),
  "nebbiedit_info": $(python3 -c 'import json,sys; print(json.dumps(sys.stdin.read()))' <"${info_out}"),
  "nebbiedit_validate": $(python3 -c 'import json,sys; print(json.dumps(sys.stdin.read()))' <"${validate_out}")
}
EOF
    fi

    rm -f "${info_out}" "${validate_out}"
    echo "Manifest written: ${LOCAL_MANIFEST}"
    if [[ ${validate_rc} -ne 0 ]]; then
        echo "WARN: nebbiedit validate reported issues (see manifest)" >&2
    fi
}

cmd_push() {
    if [[ ! -d "${LOCAL_LIB}" ]]; then
        echo "ERROR: ${LOCAL_LIB} missing. Run: $0 fetch" >&2
        exit 1
    fi

    local remote="${STAGING_SSH_USER}@${STAGING_SSH_HOST}"

    echo "==> Pushing lib to ${remote}:${STAGING_LIB_REMOTE}"
    rsync -avz -e "ssh ${RSYNC_SSH_OPTS}" \
        --delete \
        --exclude='players/' \
        --exclude='mud_mail/' \
        --exclude='world/' \
        "${LOCAL_LIB}/" "${remote}:${STAGING_LIB_REMOTE}/"

    if [[ -f "${LOCAL_MANIFEST}" ]]; then
        echo "==> Uploading manifest"
        rsync -avz -e "ssh ${RSYNC_SSH_OPTS}" \
            "${LOCAL_MANIFEST}" "${remote}:${STAGING_MANIFEST_REMOTE}"
    fi

    echo "Push complete -> ${STAGING_SSH_HOST}"
}

cmd_sync() {
    cmd_fetch
    cmd_manifest
    cmd_push
}

main() {
    if [[ $# -lt 1 ]]; then
        usage
        exit 1
    fi

    load_env
    case "$1" in
        fetch) cmd_fetch ;;
        manifest) cmd_manifest ;;
        push) cmd_push ;;
        sync) cmd_sync ;;
        -h|--help) usage ;;
        *) echo "Unknown command: $1" >&2; usage; exit 1 ;;
    esac
}

main "$@"
