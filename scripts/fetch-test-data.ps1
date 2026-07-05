# Clone nebbietest vendor data for integration tests.
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Vendor = Join-Path $Root "vendor\nebbietest"
$Repo = "https://github.com/wizardmorgan/nebbietest.git"

if (Test-Path (Join-Path $Vendor ".git")) {
    Write-Host "nebbietest already present at $Vendor"
    exit 0
}

New-Item -ItemType Directory -Force -Path (Split-Path $Vendor) | Out-Null
Write-Host "Cloning $Repo into $Vendor"
git clone --depth 1 $Repo $Vendor
