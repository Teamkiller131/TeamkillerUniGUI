<#
.SYNOPSIS
    Diagnose the local build environment for TeamkillerUniGUI (MSVC + Ninja + vcpkg).

.DESCRIPTION
    Checks for the toolchain pieces this project needs and flags the common
    pitfalls that cause confusing build failures:
      * Visual Studio with the C++ workload (located via vswhere)
      * A single, consistent MSVC toolset on PATH (stale versions are flagged)
      * CMake 3.31+
      * Ninja
      * vcpkg (VCPKG_ROOT or a discoverable install)
    It prints a clear PASS / WARN / FAIL summary and concrete fix suggestions.

.EXAMPLE
    pwsh -File scripts/check_env.ps1

.NOTES
    Read-only. This script never changes your machine; it only inspects it.
#>
[CmdletBinding()]
param(
    [switch]$Quiet
)

$ErrorActionPreference = 'Stop'
$script:Fail = 0
$script:Warn = 0

function Write-Status {
    param(
        [ValidateSet('PASS', 'WARN', 'FAIL', 'INFO')] [string]$Level,
        [string]$Message,
        [string]$Hint
    )
    $color = switch ($Level) {
        'PASS' { 'Green' }
        'WARN' { 'Yellow' }
        'FAIL' { 'Red' }
        default { 'Cyan' }
    }
    Write-Host ("[{0}] " -f $Level) -ForegroundColor $color -NoNewline
    Write-Host $Message
    if ($Hint -and -not $Quiet) {
        foreach ($line in ($Hint -split "`n")) {
            Write-Host ("       -> " + $line) -ForegroundColor DarkGray
        }
    }
    if ($Level -eq 'FAIL') { $script:Fail++ }
    if ($Level -eq 'WARN') { $script:Warn++ }
}

Write-Host ""
Write-Host "=== TeamkillerUniGUI environment check ===" -ForegroundColor White
Write-Host ""

# --- Visual Studio (vswhere) -------------------------------------------------
$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'
if (-not (Test-Path $vswhere)) {
    $vswhere = Join-Path $env:ProgramFiles 'Microsoft Visual Studio\Installer\vswhere.exe'
}

$vsInstall = $null
if (Test-Path $vswhere) {
    $vsInstall = & $vswhere -latest -prerelease -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath 2>$null | Select-Object -First 1
}

if ($vsInstall) {
    $vcvars = Join-Path $vsInstall 'VC\Auxiliary\Build\vcvars64.bat'
    if (Test-Path $vcvars) {
        Write-Status PASS "Visual Studio (C++ workload): $vsInstall"
    }
    else {
        Write-Status FAIL "Visual Studio found but vcvars64.bat is missing." `
            "Re-run the Visual Studio Installer and add 'Desktop development with C++'."
    }
}
else {
    Write-Status FAIL "No Visual Studio with the C++ workload was found." `
        ("Install 'Desktop development with C++' via the Visual Studio Installer,`n" +
         "or set VS_INSTALL_DIR to your install root before running cmake-msvc.cmd.")
}

# --- Detect stale MSVC toolsets on PATH (the #1 cache-corruption pitfall) -----
$msvcOnPath = @()
foreach ($p in ($env:PATH -split ';')) {
    if ($p -match '\\VC\\Tools\\MSVC\\(?<ver>[0-9.]+)\\bin\\') {
        $msvcOnPath += [pscustomobject]@{ Version = $Matches.ver; Path = $p }
    }
}
$distinctVer = @($msvcOnPath.Version | Sort-Object -Unique)
if ($distinctVer.Count -gt 1) {
    Write-Status WARN ("Multiple MSVC toolsets on PATH: {0}" -f ($distinctVer -join ', ')) `
        ("A stale toolset on PATH can be cached by CMake and then break after a VS update.`n" +
         "Prefer building through cmake-msvc.cmd, which re-runs vcvars64.bat to pin one toolset.`n" +
         "If a build already broke this way, delete the affected build/ dir and reconfigure.")
}
elseif ($distinctVer.Count -eq 1) {
    Write-Status INFO ("MSVC toolset on PATH: {0}" -f $distinctVer[0])
}

# --- CMake -------------------------------------------------------------------
$cmake = Get-Command cmake -ErrorAction SilentlyContinue
if ($cmake) {
    $verLine = (& cmake --version | Select-Object -First 1)
    if ($verLine -match '([0-9]+)\.([0-9]+)') {
        $major = [int]$Matches[1]; $minor = [int]$Matches[2]
        if ($major -gt 3 -or ($major -eq 3 -and $minor -ge 31)) {
            Write-Status PASS "CMake: $verLine"
        }
        else {
            Write-Status WARN "CMake $verLine is older than the required 3.31." `
                "Update CMake (the Visual Studio Installer ships a recent one)."
        }
    }
}
else {
    Write-Status FAIL "cmake not found on PATH." `
        "Install CMake 3.31+ or enable the CMake component in the VS Installer."
}

# --- Ninja -------------------------------------------------------------------
$ninja = Get-Command ninja -ErrorAction SilentlyContinue
if ($ninja) {
    Write-Status PASS ("Ninja: {0} ({1})" -f (& ninja --version), $ninja.Source)
}
else {
    Write-Status WARN "ninja not found on PATH." `
        ("All presets use the Ninja generator. Install via 'winget install Ninja-build.Ninja'`n" +
         "or note that cmake-msvc.cmd inherits Ninja bundled with Visual Studio.")
}

# --- vcpkg -------------------------------------------------------------------
$vcpkgRoot = $env:VCPKG_ROOT
$vcpkgCmd = Get-Command vcpkg -ErrorAction SilentlyContinue
if ($vcpkgRoot -and (Test-Path (Join-Path $vcpkgRoot 'scripts\buildsystems\vcpkg.cmake'))) {
    Write-Status PASS "vcpkg (VCPKG_ROOT): $vcpkgRoot"
}
elseif ($vcpkgCmd) {
    Write-Status PASS ("vcpkg on PATH: {0}" -f $vcpkgCmd.Source)
}
elseif ($vsInstall -and (Test-Path (Join-Path $vsInstall 'VC\vcpkg\scripts\buildsystems\vcpkg.cmake'))) {
    Write-Status INFO "vcpkg: using the copy bundled with Visual Studio." `
        "Set VCPKG_ROOT explicitly if you want a standalone vcpkg with its own cache."
}
else {
    Write-Status WARN "vcpkg not found (no VCPKG_ROOT, not on PATH, none bundled)." `
        ("Presets expect VCPKG_ROOT. Clone https://github.com/microsoft/vcpkg,`n" +
         "run bootstrap-vcpkg.bat, then set VCPKG_ROOT to that folder.")
}

# --- Summary -----------------------------------------------------------------
Write-Host ""
if ($script:Fail -gt 0) {
    Write-Host ("RESULT: {0} blocking issue(s), {1} warning(s)." -f $script:Fail, $script:Warn) -ForegroundColor Red
    Write-Host "Fix the FAIL items above, then re-run this script." -ForegroundColor Red
    exit 1
}
elseif ($script:Warn -gt 0) {
    Write-Host ("RESULT: ready to build, with {0} warning(s)." -f $script:Warn) -ForegroundColor Yellow
    Write-Host "Recommended: .\cmake-msvc.cmd --preset windows-msvc-release" -ForegroundColor Yellow
    exit 0
}
else {
    Write-Host "RESULT: environment looks good. Build with:" -ForegroundColor Green
    Write-Host "  .\cmake-msvc.cmd --preset windows-msvc-release" -ForegroundColor Green
    exit 0
}
