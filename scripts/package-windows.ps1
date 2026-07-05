# Build a portable Windows zip with nebbiedit, nebbieedit, and Qt runtime DLLs.
param(
    [switch]$NoBuild
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Build = Join-Path $Root "build"
$Dist = Join-Path $Root "dist"
$Staging = Join-Path $Dist "windows-staging"

function Get-Version {
    $line = Select-String -Path (Join-Path $Root "CMakeLists.txt") -Pattern 'project\(nebbie-editor VERSION ' | Select-Object -First 1
    if ($line -match 'VERSION ([0-9.]+)') { return $Matches[1] }
    return "0.0.0"
}

if ($PSVersionTable.PSEdition -ne "Desktop" -and $IsWindows -ne $true -and $env:OS -notlike "*Windows*") {
    Write-Error "package-windows.ps1 must run on Windows."
}

if (-not $NoBuild) {
    & (Join-Path $Root "scripts\build.ps1")
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
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

$Version = Get-Version
Remove-Item -Recurse -Force $Staging -ErrorAction SilentlyContinue
New-Item -ItemType Directory -Force -Path $Staging | Out-Null
Copy-Item $Cli (Join-Path $Staging "nebbiedit.exe")
Copy-Item $Gui (Join-Path $Staging "nebbieedit.exe")

$Windeploy = Get-Command windeployqt -ErrorAction SilentlyContinue
if ($Windeploy) {
    & windeployqt (Join-Path $Staging "nebbieedit.exe")
} else {
    Write-Warning "windeployqt not in PATH; zip may miss Qt DLLs. Set PATH to Qt kit\bin."
}

$Zip = Join-Path $Dist "nebbie-editor_${Version}_windows.zip"
Remove-Item $Zip -ErrorAction SilentlyContinue
Compress-Archive -Path (Join-Path $Staging "*") -DestinationPath $Zip

Write-Host "Package created: $Zip"
