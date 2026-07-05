# Build portable zip and/or Windows setup installer.
param(
    [switch]$NoBuild,
    [switch]$ZipOnly,
    [switch]$InstallerOnly
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Dist = Join-Path $Root "dist"
$Staging = Join-Path $Dist "windows-staging"

function Get-ProjectVersion {
    $line = Select-String -Path (Join-Path $Root "CMakeLists.txt") -Pattern 'project\(nebbie-editor VERSION ' | Select-Object -First 1
    if ($line -match 'VERSION ([0-9.]+)') { return $Matches[1] }
    return "0.0.0"
}

if ($PSVersionTable.PSEdition -ne "Desktop" -and $IsWindows -ne $true -and $env:OS -notlike "*Windows*") {
    Write-Error "package-windows.ps1 must run on Windows."
}

$buildZip = -not $InstallerOnly
$buildInstaller = -not $ZipOnly

$prepareArgs = @()
if ($NoBuild) { $prepareArgs += "-NoBuild" }
& (Join-Path $Root "scripts\prepare-windows-package.ps1") @prepareArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$Version = Get-ProjectVersion

if ($buildZip) {
    $Zip = Join-Path $Dist "nebbie-editor_${Version}_windows.zip"
    Remove-Item $Zip -ErrorAction SilentlyContinue
    Compress-Archive -Path (Join-Path $Staging "*") -DestinationPath $Zip
    Write-Host "Zip package created: $Zip"
}

if ($buildInstaller) {
    $installerArgs = @("-SkipStaging")
    if ($NoBuild) { $installerArgs += "-NoBuild" }
    & (Join-Path $Root "scripts\package-windows-installer.ps1") @installerArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}
