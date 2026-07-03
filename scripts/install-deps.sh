#!/usr/bin/env bash
# Install build dependencies on Linux or macOS.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
WITH_QT=1

usage() {
    cat <<'EOF'
Usage: ./scripts/install-deps.sh [--no-qt]

Installs CMake, a C++17 compiler, and optionally Qt 6 for nebbieedit.

Linux (Debian/Ubuntu): build-essential, cmake, qt6-base-dev
macOS (Homebrew): cmake, qt@6
EOF
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --no-qt) WITH_QT=0; shift ;;
        -h|--help) usage; exit 0 ;;
        *) echo "Unknown option: $1" >&2; usage; exit 1 ;;
    esac
done

uname_s="$(uname -s)"

install_linux() {
    if ! command -v apt-get >/dev/null 2>&1; then
        echo "apt-get not found. Install manually: cmake, g++, qt6-base-dev" >&2
        exit 1
    fi
    sudo apt-get update
    local packages=(build-essential cmake)
    if [[ "$WITH_QT" -eq 1 ]]; then
        packages+=(qt6-base-dev)
    fi
    sudo apt-get install -y "${packages[@]}"
}

install_macos() {
    if ! command -v brew >/dev/null 2>&1; then
        cat >&2 <<'EOF'
Homebrew is required on macOS: https://brew.sh
Install manually: brew install cmake qt@6
EOF
        exit 1
    fi
    brew install cmake
    if [[ "$WITH_QT" -eq 1 ]]; then
        brew install qt@6
        echo
        echo "Qt 6 prefix: $(brew --prefix qt@6)"
        echo "Build with: CMAKE_PREFIX_PATH=\"$(brew --prefix qt@6)\" ./scripts/build.sh"
    fi
}

case "$uname_s" in
    Linux) install_linux ;;
    Darwin) install_macos ;;
    *)
        echo "Unsupported OS: $uname_s (supported: Linux, Darwin/macOS)" >&2
        exit 1
        ;;
esac

echo "Dependencies installed for $uname_s."
