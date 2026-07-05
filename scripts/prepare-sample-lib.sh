#!/usr/bin/env bash
# Build dist/sample-mudroot for bundling inside Linux/macOS/Windows packages.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DIST="${ROOT}/dist"
OUT="${DIST}/sample-mudroot"
SERVER_ROOT="${ROOT}/vendor/nebbietest"
SOURCE_LIB=""

usage() {
    cat <<'EOF'
Usage: ./scripts/prepare-sample-lib.sh [options]

Creates dist/sample-mudroot/lib with myst.* and overlay directories
(getworldlocal workflow on vendor/nebbietest).

Options:
  --server-root PATH   Nebbie server checkout (default: vendor/nebbietest)
  --source-lib PATH    Copy from an existing mudroot/lib (skips getworldlocal)
  -h, --help           Show help
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --server-root) SERVER_ROOT="$2"; shift 2 ;;
        --source-lib) SOURCE_LIB="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

if [[ -z "${SOURCE_LIB}" ]]; then
    "${ROOT}/scripts/fetch-test-data.sh"
    "${ROOT}/scripts/getworldlocal.sh" "${SERVER_ROOT}"
    SOURCE_LIB="${SERVER_ROOT}/mudroot/lib"
fi

if [[ ! -d "${SOURCE_LIB}" ]]; then
    echo "ERROR: source lib not found: ${SOURCE_LIB}" >&2
    exit 1
fi

rm -rf "${OUT}"
mkdir -p "${OUT}/lib"

echo "==> Staging sample lib from ${SOURCE_LIB}"
if command -v rsync >/dev/null 2>&1; then
    rsync -a \
        --exclude='players/' \
        --exclude='mud_mail/' \
        --exclude='world/' \
        --exclude='REBOOT*' \
        --exclude='asshole.list' \
        --exclude='killfile' \
        --exclude='.git/' \
        "${SOURCE_LIB}/" "${OUT}/lib/"
else
    shopt -s dotglob nullglob
    for entry in "${SOURCE_LIB}"/*; do
        base="$(basename "${entry}")"
        case "${base}" in
            players|mud_mail|world|asshole.list|killfile) continue ;;
            REBOOT*) continue ;;
        esac
        cp -a "${entry}" "${OUT}/lib/"
    done
    shopt -u dotglob nullglob
fi

for subdir in objects mobiles rooms zones; do
    mkdir -p "${OUT}/lib/${subdir}"
done

CLI="${ROOT}/build/nebbiedit/nebbiedit"
if [[ ! -x "${CLI}" ]]; then
    CLI="${ROOT}/build/nebbiedit/Release/nebbiedit.exe"
fi
if [[ ! -x "${CLI}" ]]; then
    CLI="$(command -v nebbiedit || true)"
fi

MANIFEST="${OUT}/lib-manifest.txt"
if [[ -x "${CLI}" ]]; then
    echo "==> Writing manifest via nebbiedit info"
    "${CLI}" info "${OUT}/lib" >"${MANIFEST}" 2>&1 || true
else
    echo "(nebbiedit not built; skipping manifest)" >"${MANIFEST}"
fi

cat >"${OUT}/LEGGIMI.txt" <<'EOF'
Libreria di esempio Nebbie Editor (sample-mudroot)
=================================================

Questa cartella simula mudroot/lib del server Nebbie Arcane.
I file myst.* sono allineati al workflow getworldlocal del repository Server.

Percorsi tipici dopo l'installazione:

  Linux (.deb)
    /usr/share/nebbie-editor/sample-mudroot/lib

  macOS (.dmg)
    <volume DMG>/sample-mudroot/lib
    oppure /Applications/nebbieedit.app/../sample-mudroot/lib se copiato

  Windows (zip o installer)
    <cartella installazione>\sample-mudroot\lib

Avvio rapido GUI:
  nebbieedit <percorso>/sample-mudroot/lib

Avvio rapido CLI:
  nebbiedit info <percorso>/sample-mudroot/lib
  nebbiedit validate <percorso>/sample-mudroot/lib

Non modificare questa copia se volete conservare il dataset di prova originale:
duplicate la cartella sample-mudroot prima di editare.
EOF

echo ""
echo "Sample mudroot ready:"
du -sh "${OUT}/lib"
echo "  ${OUT}/lib"
ls -1 "${OUT}/lib"/myst.* 2>/dev/null | sed 's/^/    /' || true
