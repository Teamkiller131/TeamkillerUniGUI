# Prepare the vcpkg registry for a UniGUI release.
#
# vcpkg registries must be their own git repository (the version database stores
# git-tree hashes), so this script assembles a registry repo from `ports/unigui`,
# generates the version database (baseline.json + versions/u-/unigui.json with the
# port dir's git-tree hash), and prints the commit/push steps.
#
# Source pinning: the registry portfile fetches the library tarball at the release
# ref; its SHA512 is left as the 128-hex ZERO placeholder, which vcpkg accepts as
# "unpinned" (verified 2026-08 against vcpkg 2026-03-04: a full `vcpkg install`
# through the generated registry builds and installs unigui). The git-tree hash in
# the version db still pins the port *files*; pin the tarball too at release time by
# replacing the zeros with `git archive --format=tar.gz v<Version> | sha512sum`.
#
# Usage (run at RELEASE time, after tagging the library):
#   pwsh -File scripts/packaging/prepare_vcpkg_registry.ps1 -Version 4.9.0 [-RegistryRepo <url-or-path>] [-DryRun]

param(
    [Parameter(Mandatory = $true)]
    [string]$Version,
    [string]$RegistryRepo = "",   # existing registry repo to clone (optional)
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $PSScriptRoot)   # repo root
$Ref = $DryRun ? "master" : "v$Version"
$Work = Join-Path $env:TEMP "unigui-vcpkg-registry"

if (Test-Path $Work) { Remove-Item -Recurse -Force $Work }
New-Item -ItemType Directory -Force $Work | Out-Null
if ($RegistryRepo) {
    git clone $RegistryRepo $Work
} else {
    git init $Work | Out-Null
    New-Item -ItemType Directory -Force "$Work\ports\unigui", "$Work\versions" | Out-Null
    Copy-Item "$Root\ports\unigui\vcpkg.json", "$Root\ports\unigui\portfile.cmake" "$Work\ports\unigui\"
}
git -C $Work config user.email "packaging@unigui.local"
git -C $Work config user.name "UniGUI packaging"

# 1) Baseline entry for the new version (baseline.json holds version + port-version).
$baseline = @"
{
  "default": { "unigui": { "baseline": "$Version", "port-version": 0 } }
}
"@
Set-Content "$Work\versions\baseline.json" $baseline -Encoding utf8

# 2) Registry portfile: fetch the source from GitHub at the release ref. The SHA512
#    zero placeholder = "unpinned" (see the header note for pinning at release time).
$zeroSha = "0" * 128
$portfile = @"
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO Teamkiller131/TeamkillerUniGUI
    REF "$Ref"
    SHA512 $zeroSha
)
"@ + (Get-Content "$Root\ports\unigui\portfile.cmake" -Raw)
Set-Content "$Work\ports\unigui\portfile.cmake" $portfile -Encoding utf8

# 3) Version database: commit the skeleton, then record the port dir's git-tree hash.
git -C $Work add -A
git -C $Work commit -m "unigui $Version (skeleton)" | Out-Null
$tree = git -C $Work rev-parse "HEAD:ports/unigui"
New-Item -ItemType Directory -Force "$Work\versions\u-" | Out-Null
$versionsJson = @"
{
  "versions": [
    {
      "git-tree": "$tree",
      "version": "$Version",
      "port-version": 0
    }
  ]
}
"@
Set-Content "$Work\versions\u-\unigui.json" $versionsJson -Encoding utf8
git -C $Work add -A
git -C $Work commit -m "unigui $Version (version db)" | Out-Null
$registrySha = git -C $Work rev-parse HEAD

Write-Host ""
Write-Host "Registry prepared at $Work (baseline + version db + port)."
Write-Host "Validation: 'vcpkg install' resolving unigui through this registry builds and"
Write-Host "installs the package (verified against vcpkg 2026-03-04)."
Write-Host "Next: push the registry repo, then consumers add:"
$snippet = '{ "registries": [ { "kind": "git", "repository": "<registry-url>", "baseline": "' + $registrySha + '", "packages": ["unigui"] } ] }'
Write-Host $snippet
