<#
.SYNOPSIS
    One-command build for TeamkillerUniGUI on Windows (MSVC + Ninja + vcpkg).

.DESCRIPTION
    Wraps the recommended flow so you don't have to remember the exact incantation:
      1. (optional) run the environment self-check
      2. configure the chosen CMake preset through cmake-msvc.cmd (correct MSVC env)
      3. build it
      4. (optional) run the test suite

    All heavy lifting still goes through cmake-msvc.cmd, which pins the MSVC
    toolset via vcvars64.bat to avoid the stale-toolset cache pitfall.

.PARAMETER Preset
    CMake configure preset to use. Default: windows-msvc-release.

.PARAMETER Test
    Also run ctest after a successful build.

.PARAMETER Clean
    Delete the preset's build directory before configuring (fresh build).

.PARAMETER SkipCheck
    Skip the environment self-check step.

.EXAMPLE
    pwsh -File scripts/build.ps1

.EXAMPLE
    pwsh -File scripts/build.ps1 -Preset windows-msvc-debug -Test

.EXAMPLE
    pwsh -File scripts/build.ps1 -Clean
#>
[CmdletBinding()]
param(
    [string]$Preset = 'windows-msvc-release',
    [switch]$Test,
    [switch]$Clean,
    [switch]$SkipCheck
)

$ErrorActionPreference = 'Stop'
$repoRoot = Split-Path -Parent $PSScriptRoot
Set-Location $repoRoot

$wrapper = Join-Path $repoRoot 'cmake-msvc.cmd'
if (-not (Test-Path $wrapper)) {
    Write-Host "ERROR: cmake-msvc.cmd not found at repo root: $repoRoot" -ForegroundColor Red
    exit 1
}

function Invoke-Step {
    param([string]$Name, [scriptblock]$Action)
    Write-Host ""
    Write-Host ">>> $Name" -ForegroundColor Cyan
    & $Action
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "FAILED: $Name (exit $LASTEXITCODE)" -ForegroundColor Red
        if (-not $SkipCheck) {
            Write-Host "Tip: run 'pwsh -File scripts/check_env.ps1' to diagnose the toolchain." -ForegroundColor Yellow
        }
        exit $LASTEXITCODE
    }
}

# 1. Environment self-check (non-fatal warnings are fine).
if (-not $SkipCheck) {
    Write-Host ">>> Environment self-check" -ForegroundColor Cyan
    & pwsh -File (Join-Path $PSScriptRoot 'check_env.ps1') -Quiet
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Environment check reported blocking issues; aborting." -ForegroundColor Red
        exit $LASTEXITCODE
    }
}

# 2. Optional clean.
$buildDir = Join-Path $repoRoot ("build/" + $Preset)
if ($Clean -and (Test-Path $buildDir)) {
    Invoke-Step "Removing $buildDir" { Remove-Item -Recurse -Force $buildDir; $global:LASTEXITCODE = 0 }
}

# 3. Configure.
Invoke-Step "Configuring preset '$Preset'" { & $wrapper --preset $Preset }

# 4. Build.
Invoke-Step "Building preset '$Preset'" { & $wrapper --build --preset $Preset }

# 5. Optional tests.
if ($Test) {
    Invoke-Step "Running tests" { ctest --test-dir $buildDir --output-on-failure }
}

Write-Host ""
Write-Host "DONE: build artifacts in $buildDir" -ForegroundColor Green
