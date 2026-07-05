#!/usr/bin/env bash
# Build a Debian package (.deb) for Nebbie Editor (CLI + Qt GUI).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD="${ROOT}/build"
DIST="${ROOT}/dist"
STAGING=""
VERSION=""
ARCH="$(dpkg --print-architecture 2>/dev/null || echo amd64)"
PREFIX="/usr"
RUN_BUILD=1

usage() {
    cat <<'EOF'
Usage: ./scripts/package-deb.sh [options]

Builds dist/nebbie-editor_<version>_<arch>.deb

Options:
  --no-build       Skip ./scripts/build.sh (use existing build/)
  --prefix PATH    Install prefix inside the package (default: /usr)
  -h, --help       Show this help

Requires: cmake build, dpkg-deb, Qt 6 runtime packages on target system.
EOF
}

cleanup() {
    if [[ -n "${STAGING}" && -d "${STAGING}" ]]; then
        rm -rf "${STAGING}"
    fi
}

read_version() {
    VERSION="$(sed -n 's/^project(nebbie-editor VERSION \([^ )]*\).*/\1/p' "${ROOT}/CMakeLists.txt")"
    if [[ -z "${VERSION}" ]]; then
        VERSION="0.0.0"
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-build) RUN_BUILD=0; shift ;;
        --prefix) PREFIX="$2"; shift 2 ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

if [[ "$(uname -s)" != "Linux" ]]; then
    echo "ERROR: package-deb.sh must run on Linux." >&2
    exit 1
fi

if ! command -v dpkg-deb >/dev/null 2>&1; then
    echo "ERROR: dpkg-deb not found (install dpkg)." >&2
    exit 1
fi

trap cleanup EXIT
read_version

if [[ "${RUN_BUILD}" -eq 1 ]]; then
    "${ROOT}/scripts/build.sh"
fi

if [[ ! -x "${BUILD}/nebbiedit/nebbiedit" ]]; then
    echo "ERROR: CLI binary missing. Build first: ./scripts/build.sh" >&2
    exit 1
fi
if [[ ! -x "${BUILD}/nebbie-qt/nebbieedit" ]]; then
    echo "ERROR: GUI binary missing. Install Qt 6 and rebuild." >&2
    exit 1
fi

STAGING="$(mktemp -d)"
mkdir -p "${DIST}"

echo "==> Installing into package staging (${PREFIX})"
DESTDIR="${STAGING}" cmake --install "${BUILD}" --prefix "${PREFIX}"

mkdir -p "${STAGING}/DEBIAN"

INSTALLED_SIZE="$(du -sk "${STAGING}" | awk '{print $1}')"

cat > "${STAGING}/DEBIAN/control" <<EOF
Package: nebbie-editor
Version: ${VERSION}
Section: editors
Priority: optional
Architecture: ${ARCH}
Depends: libc6 (>= 2.31), libstdc++6 (>= 10), libqt6core6 (>= 6.2.0) | libqt6core6t64 (>= 6.2.0), libqt6gui6 (>= 6.2.0) | libqt6gui6t64 (>= 6.2.0), libqt6widgets6 (>= 6.2.0) | libqt6widgets6t64 (>= 6.2.0), libqt6network6 (>= 6.2.0) | libqt6network6t64 (>= 6.2.0)
Maintainer: Nebbie Editor <nebbie-editor@local>
Installed-Size: ${INSTALLED_SIZE}
Description: World editor for Nebbie Arcane MUD
 Nebbie Editor provides a CLI (nebbiedit) and Qt GUI (nebbieedit) to edit
 myst.* world files and overlay directories for Nebbie Arcane servers.
EOF

DEB_FILE="${DIST}/nebbie-editor_${VERSION}_${ARCH}.deb"
echo "==> Building ${DEB_FILE}"
dpkg-deb --root-owner-group --build "${STAGING}" "${DEB_FILE}"

echo ""
echo "Package created:"
echo "  ${DEB_FILE}"
ls -lh "${DEB_FILE}"
echo ""
echo "Install:"
echo "  sudo dpkg -i ${DEB_FILE}"
echo "  sudo apt-get install -f"
