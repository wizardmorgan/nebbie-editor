#!/usr/bin/env bash
# Simulate packaging and installation smoke tests per platform.
set -euo pipefail

ROOT="$(cd "$(dirname "$(dirname "${BASH_SOURCE[0]}")")" && pwd)"
DIST="${ROOT}/dist"
LOG_DIR="${ROOT}/dist/install-test-logs"
REPORT="${LOG_DIR}/report.txt"
PASS=0
FAIL=0
SKIP=0

mkdir -p "$LOG_DIR"
: > "$REPORT"

log() {
    echo "$*" | tee -a "$REPORT"
}

pass() {
    PASS=$((PASS + 1))
    log "PASS: $*"
}

fail() {
    FAIL=$((FAIL + 1))
    log "FAIL: $*"
}

skip() {
    SKIP=$((SKIP + 1))
    log "SKIP: $*"
}

run_cmd() {
    local name="$1"
    shift
    local logfile="${LOG_DIR}/${name}.log"
    if "$@" >"$logfile" 2>&1; then
        pass "$name"
        return 0
    fi
    fail "$name (see ${logfile})"
    tail -20 "$logfile" >> "$REPORT" || true
    return 1
}

log "=== Nebbie Editor installation simulation ==="
log "Date: $(date -u +%Y-%m-%dT%H:%M:%SZ)"
log "Host: $(uname -s) $(uname -m)"
log ""

# --- Linux: build + .deb + simulated install ---
log "--- Linux (.deb) ---"

if [[ "$(uname -s)" == "Linux" ]]; then
  run_cmd "linux-build" "${ROOT}/scripts/build.sh" || true
  run_cmd "linux-package-deb" "${ROOT}/scripts/package-deb.sh" --no-build || true

  DEB="$(ls -1 "${DIST}"/nebbie-editor_*_amd64.deb 2>/dev/null | head -1 || true)"
  if [[ -n "${DEB}" && -f "${DEB}" ]]; then
    INSTALL_ROOT="${DIST}/simulated-linux-root"
    rm -rf "${INSTALL_ROOT}"
    mkdir -p "${INSTALL_ROOT}"
    run_cmd "linux-deb-extract" dpkg-deb -x "${DEB}" "${INSTALL_ROOT}" || true

    CLI="${INSTALL_ROOT}/usr/bin/nebbiedit"
    GUI="${INSTALL_ROOT}/usr/bin/nebbieedit"
  DESKTOP="${INSTALL_ROOT}/usr/share/applications/nebbieedit.desktop"

    if [[ -x "${CLI}" ]]; then
      run_cmd "linux-installed-cli-info" "${CLI}" info "${ROOT}/tests/fixtures" || true
      run_cmd "linux-installed-cli-validate" "${CLI}" validate "${ROOT}/tests/fixtures" || true
    else
      fail "linux-installed-cli-missing"
    fi

    if [[ -x "${GUI}" ]]; then
      pass "linux-installed-gui-present"
    else
      fail "linux-installed-gui-missing"
    fi

    if [[ -f "${DESKTOP}" ]]; then
      pass "linux-installed-desktop-file"
    else
      fail "linux-installed-desktop-file-missing"
    fi

    run_cmd "linux-deb-metadata" dpkg-deb --info "${DEB}" || true
  else
    fail "linux-deb-not-found"
  fi
else
  skip "linux-deb (not on Linux)"
fi

log ""
log "--- macOS (.dmg) ---"
if [[ "$(uname -s)" == "Darwin" ]]; then
  if command -v hdiutil >/dev/null 2>&1; then
    run_cmd "macos-package-dmg" "${ROOT}/scripts/package-dmg.sh" || true
    DMG="$(ls -1 "${DIST}"/nebbie-editor_*_macos.dmg 2>/dev/null | head -1 || true)"
    if [[ -n "${DMG}" ]]; then
      MOUNT="${DIST}/dmg-mount"
      mkdir -p "${MOUNT}"
      run_cmd "macos-dmg-attach" hdiutil attach "${DMG}" -mountpoint "${MOUNT}" -nobrowse -readonly || true
      if [[ -d "${MOUNT}/nebbieedit.app" ]]; then
        pass "macos-dmg-contains-app"
      else
        fail "macos-dmg-missing-app"
      fi
      hdiutil detach "${MOUNT}" 2>/dev/null || true
    else
      fail "macos-dmg-not-found"
    fi
  else
    skip "macos-dmg (hdiutil missing)"
  fi
else
  skip "macos-dmg (requires macOS runner)"
  if bash -n "${ROOT}/scripts/package-dmg.sh" 2>"${LOG_DIR}/macos-script-syntax.log"; then
    pass "macos-package-dmg-script-syntax"
  else
    fail "macos-package-dmg-script-syntax"
  fi
fi

log ""
log "--- Windows (zip + installer) ---"
if [[ "${OS:-}" == "Windows_NT" ]] || [[ "$(uname -s)" == MINGW* ]]; then
  run_cmd "windows-package" powershell -ExecutionPolicy Bypass -File "${ROOT}/scripts/package-windows.ps1" || true
else
  skip "windows-package (requires Windows runner)"
  for script in package-windows.ps1 package-windows-installer.ps1 prepare-windows-package.ps1; do
    if command -v pwsh >/dev/null 2>&1; then
      if pwsh -NoProfile -Command "& { \$null = [System.Management.Automation.Language.Parser]::ParseFile('${ROOT}/scripts/${script}', [ref]\$null, [ref]\$errs); if (\$errs) { exit 1 } }" \
        >"${LOG_DIR}/windows-${script%.ps1}-syntax.log" 2>&1; then
        pass "windows-${script}-syntax"
      else
        fail "windows-${script}-syntax"
      fi
    else
      skip "windows-${script}-syntax (pwsh not available)"
    fi
  done
  if [[ -f "${ROOT}/installer/windows/nebbie-editor.iss" ]]; then
    pass "windows-inno-script-present"
  else
    fail "windows-inno-script-missing"
  fi
fi

log ""
log "--- Summary ---"
log "PASS=${PASS} FAIL=${FAIL} SKIP=${SKIP}"
log "Full report: ${REPORT}"

if [[ "${FAIL}" -gt 0 ]]; then
  exit 1
fi
