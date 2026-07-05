# Prepare dist/windows-staging with nebbiedit, nebbieedit, and Qt runtime DLLs.
param(
    [switch]$NoBuild
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Build = Join-Path $Root "build"
$Dist = Join-Path $Root "dist"
$Staging = Join-Path $Dist "windows-staging"

function Get-ProjectVersion {
    $line = Select-String -Path (Join-Path $Root "CMakeLists.txt") -Pattern 'project\(nebbie-editor VERSION ' | Select-Object -First 1
    if ($line -match 'VERSION ([0-9.]+)') { return $Matches[1] }
    return "0.0.0"
}

if ($PSVersionTable.PSEdition -ne "Desktop" -and $IsWindows -ne $true -and $env:OS -notlike "*Windows*") {
    Write-Error "prepare-windows-package.ps1 must run on Windows."
}

if (-not $NoBuild) {
    & (Join-Path $Root "scripts\build.ps1")
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$bash = Get-Command bash -ErrorAction SilentlyContinue
if ($bash) {
    Write-Host "==> Preparing bundled sample lib (getworldlocal)"
    & bash (Join-Path $Root "scripts/prepare-sample-lib.sh")
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
} else {
    Write-Warning "bash not found; Windows package will not include sample-mudroot"
}

$Cli = @(
    (Join-Path $Build "nebbiedit\Release\nebbiedit.exe"),
    (Join-Path $Build "nebbiedit\nebbiedit.exe")
) | Where-Object { Test-Path $_ } | Select-Object -First 1

$Gui = @(
    (Join-Path $Build "nebbie-qt\Release\nebbieedit.exe"),
    (Join-Path $Build "nebbie-qt\nebbieedit.exe")
) | Where-Object { Test-Path $_ } | Select-Object -First 1

if (-not $Cli -or -not $Gui) {
    Write-Error "Build binaries not found. Run .\scripts\build.ps1 first."
}

Remove-Item -Recurse -Force $Staging -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $Staging | Out-Null
Copy-Item $Cli (Join-Path $Staging "nebbiedit.exe")
Copy-Item $Gui (Join-Path $Staging "nebbieedit.exe")

$Icon = Join-Path $Root "nebbie-qt\icons\nebbieedit.ico"
if (Test-Path $Icon) {
    Copy-Item $Icon (Join-Path $Staging "nebbieedit.ico")
}

$Sample = Join-Path $Dist "sample-mudroot"
if (Test-Path $Sample) {
    Copy-Item -Recurse $Sample (Join-Path $Staging "sample-mudroot")
}

$Windeploy = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($Windeploy) {
    & windeployqt (Join-Path $Staging "nebbieedit.exe")
} else {
    Write-Warning "windeployqt not in PATH; package may miss Qt DLLs. Add Qt kit\bin to PATH."
}

Write-Host "Windows package staging ready: $Staging"
Write-Host "Version: $(Get-ProjectVersion)"
