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

$build     = "build/$Preset"
$pkg       = "$repo/build/_pkg"
$consumer  = "$repo/build/_consumer"
$installed = "$repo/$build/vcpkg_installed"      # reused by the toolchain (classic mode)
$toolchain = "$env:VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"

if (-not (Test-Path "$build/CMakeCache.txt")) {
    throw "No build at $build — configure+build the '$Preset' preset first."
}

Write-Host "==> Installing UniGUI to $pkg"
Remove-Item -Recurse -Force $pkg, $consumer -ErrorAction SilentlyContinue
& "$repo/cmake-msvc.cmd" --install $build --prefix $pkg

# Consume through the vcpkg toolchain (classic mode, reusing the already-built
# deps) so vcpkg's find_package wrappers resolve the transitive graph (zlib via
# freetype, …). A bare CMAKE_PREFIX_PATH consume skips those wrappers.
Write-Host "==> Configuring consumer (find_package(unigui) via vcpkg toolchain)"
& "$repo/cmake-msvc.cmd" -S tests/packaging/consumer -B $consumer -G Ninja `
    -DCMAKE_BUILD_TYPE=Release `
    "-DCMAKE_TOOLCHAIN_FILE=$toolchain" `
    -DVCPKG_TARGET_TRIPLET=$Triplet `
    -DVCPKG_MANIFEST_MODE=OFF `
    "-DVCPKG_INSTALLED_DIR=$installed" `
    "-DCMAKE_PREFIX_PATH=$pkg"

Write-Host "==> Building consumer"
& "$repo/cmake-msvc.cmd" --build $consumer

Write-Host "==> Running consumer"
$env:PATH = "$installed/$Triplet/bin;$env:PATH"
& "$consumer/consumer.exe"
if ($LASTEXITCODE -ne 0) { throw "consumer exited $LASTEXITCODE" }
Write-Host "==> find_package(unigui) OK"
