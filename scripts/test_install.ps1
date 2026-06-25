# ─────────────────────────────────────────────────────────────────────────────
# test_install.ps1 — verify the find_package(unigui) packaging contract locally.
#
# Installs an already-configured Release build to a staging prefix, then builds
# and runs the standalone consumer in tests/packaging/consumer against the
# install tree (NOT the source build). Mirrors the `install-consume` CI job.
#
#   pwsh -File scripts/test_install.ps1 [-Preset windows-msvc-release]
# ─────────────────────────────────────────────────────────────────────────────
param(
    [string]$Preset = "windows-msvc-release",
    [string]$Triplet = "x64-windows"
)
$ErrorActionPreference = "Stop"
$repo = Split-Path -Parent $PSScriptRoot
Set-Location $repo

$build    = "build/$Preset"
$pkg      = "$repo/build/_pkg"
$consumer = "$repo/build/_consumer"
$vcpkg    = "$repo/$build/vcpkg_installed/$Triplet"

if (-not (Test-Path "$build/CMakeCache.txt")) {
    throw "No build at $build — configure+build the '$Preset' preset first."
}

Write-Host "==> Installing UniGUI to $pkg"
Remove-Item -Recurse -Force $pkg, $consumer -ErrorAction SilentlyContinue
& "$repo/cmake-msvc.cmd" --install $build --prefix $pkg

Write-Host "==> Configuring consumer (find_package(unigui))"
& "$repo/cmake-msvc.cmd" -S tests/packaging/consumer -B $consumer -G Ninja `
    -DCMAKE_BUILD_TYPE=Release "-DCMAKE_PREFIX_PATH=$pkg;$vcpkg"

Write-Host "==> Building consumer"
& "$repo/cmake-msvc.cmd" --build $consumer

Write-Host "==> Running consumer"
$env:PATH = "$vcpkg/bin;$env:PATH"
& "$consumer/consumer.exe"
if ($LASTEXITCODE -ne 0) { throw "consumer exited $LASTEXITCODE" }
Write-Host "==> find_package(unigui) OK"
