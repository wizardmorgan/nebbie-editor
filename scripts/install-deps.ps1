# Install build dependencies on Windows.
param(
    [switch]$NoQt
)

$ErrorActionPreference = "Stop"

function Install-WithWinget([string]$PackageId) {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        return $false
    }
    winget install --id $PackageId -e --accept-source-agreements --accept-package-agreements
    return $LASTEXITCODE -eq 0
}

Write-Host "Installing Windows build dependencies..."

if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    if (-not (Install-WithWinget "Kitware.CMake")) {
        Write-Warning "Install CMake manually: https://cmake.org/download/"
    }
}

if (-not (Get-Command cl -ErrorAction SilentlyContinue)) {
    Write-Warning "MSVC not found in PATH. Install Visual Studio 2022 Build Tools with C++ workload."
}

if (-not $NoQt) {
    if (-not $env:CMAKE_PREFIX_PATH) {
        Write-Host ""
        Write-Host "Qt 6 is required for nebbieedit."
        Write-Host "Install Qt 6 for MSVC from https://www.qt.io/download-open-source"
        Write-Host "Then set: `$env:CMAKE_PREFIX_PATH = 'C:\Qt\6.x.x\msvc2019_64'"
        Write-Host "Optional winget: winget install --id Qt.Qt.6"
    }
}

Write-Host "Done. Build with: .\scripts\build.ps1 -Test"
