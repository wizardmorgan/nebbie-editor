# Build a Windows setup installer (.exe) with Inno Setup.
param(
    [switch]$NoBuild,
    [switch]$SkipStaging
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Dist = Join-Path $Root "dist"
$Iss = Join-Path $Root "installer\windows\nebbie-editor.iss"

function Get-ProjectVersion {
    $line = Select-String -Path (Join-Path $Root "CMakeLists.txt") -Pattern 'project\(nebbie-editor VERSION ' | Select-Object -First 1
    if ($line -match 'VERSION ([0-9.]+)') { return $Matches[1] }
    return "0.0.0"
}

function Find-InnoSetupCompiler {
    $candidates = @(
        "${env:ProgramFiles(x86)}\Inno Setup 6\ISCC.exe",
        "$env:ProgramFiles\Inno Setup 6\ISCC.exe",
        (Get-Command ISCC.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source)
    )
    foreach ($candidate in $candidates) {
        if ($candidate -and (Test-Path $candidate)) {
            return $candidate
        }
    }
    return $null
}

if ($PSVersionTable.PSEdition -ne "Desktop" -and $IsWindows -ne $true -and $env:OS -notlike "*Windows*") {
    Write-Error "package-windows-installer.ps1 must run on Windows."
}

$prepareArgs = @()
if ($NoBuild) { $prepareArgs += "-NoBuild" }
if (-not $SkipStaging) {
    & (Join-Path $Root "scripts\prepare-windows-package.ps1") @prepareArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$Iscc = Find-InnoSetupCompiler
if (-not $Iscc) {
    Write-Error @"
Inno Setup 6 not found. Install from https://jrsoftware.org/isinfo.php
or run: winget install --id JRSoftware.InnoSetup
"@
}

$Version = Get-ProjectVersion
$Staging = Join-Path $Dist "windows-staging"
$Output = Join-Path $Dist "nebbie-editor_${Version}_windows_setup.exe"

Write-Host "==> Building installer with $Iscc"
& $Iscc "/DAppVersion=$Version" "/DStagingDir=$Staging" $Iss
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if (-not (Test-Path $Output)) {
    $built = Get-ChildItem -Path $Dist -Filter "nebbie-editor_*_windows_setup.exe" | Sort-Object LastWriteTime -Descending | Select-Object -First 1
    if ($built) {
        Write-Host "Installer created: $($built.FullName)"
    } else {
        Write-Error "Installer build finished but output file was not found in $Dist"
    }
} else {
    Write-Host "Installer created: $Output"
}
