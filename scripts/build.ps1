# Cross-platform configure and build for Windows (PowerShell).
param(
    [switch]$Debug,
    [switch]$NoQt,
    [switch]$Test,
    [string]$Generator = "",
    [string]$QtPrefix = $env:CMAKE_PREFIX_PATH
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$BuildDir = Join-Path $Root "build"
$BuildType = if ($Debug) { "Debug" } else { "Release" }
$WithQt = if ($NoQt) { "OFF" } else { "ON" }

function Show-Usage {
    @"
Usage: .\scripts\build.ps1 [-Debug] [-NoQt] [-Test] [-Generator NAME]

Environment:
  CMAKE_PREFIX_PATH   Qt 6 kit root (e.g. C:\Qt\6.5.3\msvc2019_64)
"@
}

$CmakeArgs = @(
    "-S", $Root,
    "-B", $BuildDir,
    "-DCMAKE_BUILD_TYPE=$BuildType",
    "-DNEBBIE_BUILD_QT=$WithQt"
)

if ($Generator) {
    $CmakeArgs += @("-G", $Generator)
} elseif (-not (Get-Command ninja -ErrorAction SilentlyContinue)) {
  if (Get-Command cl -ErrorAction SilentlyContinue) {
    $CmakeArgs += @("-G", "Visual Studio 17 2022", "-A", "x64")
  }
}

if ($QtPrefix) {
    $CmakeArgs += @("-DCMAKE_PREFIX_PATH=$QtPrefix")
}

Write-Host "==> cmake $($CmakeArgs -join ' ')"
& cmake @CmakeArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

$BuildConfigArgs = @("--build", $BuildDir)
if ($Generator -match "Visual Studio" -or $CmakeArgs -contains "Visual Studio 17 2022") {
    $BuildConfigArgs += @("--config", $BuildType)
}

Write-Host "==> cmake $($BuildConfigArgs -join ' ')"
& cmake @BuildConfigArgs
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($Test) {
    $CTestArgs = @("--test-dir", $BuildDir, "--output-on-failure")
    if ($BuildType -eq "Release" -and ($Generator -match "Visual Studio" -or $CmakeArgs -contains "Visual Studio 17 2022")) {
        $CTestArgs += @("-C", "Release")
    }
    Write-Host "==> ctest $($CTestArgs -join ' ')"
    & ctest @CTestArgs
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

Write-Host ""
Write-Host "Binaries:"
$Cli = Join-Path $BuildDir "nebbiedit\nebbiedit.exe"
if (-not (Test-Path $Cli)) { $Cli = Join-Path $BuildDir "nebbiedit\Release\nebbiedit.exe" }
Write-Host "  CLI:  $Cli"
$Gui = Join-Path $BuildDir "nebbie-qt\nebbieedit.exe"
if (-not (Test-Path $Gui)) { $Gui = Join-Path $BuildDir "nebbie-qt\Release\nebbieedit.exe" }
Write-Host "  GUI:  $Gui"
