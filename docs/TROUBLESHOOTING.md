# Troubleshooting

Run the environment check first:

```powershell
pwsh -File scripts/check_env.ps1
```

It reports missing MSVC workloads, CMake/Ninja versions, `VCPKG_ROOT`, and conflicting toolsets on `PATH`.

## MSVC toolset / CreateProcess failed

**Symptom**: `cl.exe` or `link.exe` “CreateProcess failed” or path not found after a Visual Studio update.

**Fix**: CMake may cache an old toolset path. Build through `cmake-msvc.cmd` (re-runs `vcvars64.bat`). Delete the build directory and reconfigure:

```powershell
Remove-Item -Recurse -Force build/windows-msvc-release
pwsh -File scripts/build.ps1 -Clean
```

## vcpkg toolchain not found

**Symptom**: `Could not find toolchain file .../vcpkg.cmake`.

**Fix**: Set `VCPKG_ROOT` to your vcpkg checkout, or use the copy bundled with Visual Studio. `check_env.ps1` prints which vcpkg was detected.

## Ninja not found

**Symptom**: Generator errors mentioning `ninja`.

**Fix**: `winget install Ninja-build.Ninja`, or use `cmake-msvc.cmd` / Visual Studio’s bundled Ninja.

## Non-default Visual Studio location

**Fix**: `cmake-msvc.cmd` uses `vswhere`. For unusual installs, set `VS_INSTALL_DIR` to the VS installation root.

## Headless CI: AppTest hangs

**Symptom**: `AppTest.Init_WithoutDisplay_ReturnsFalse` never finishes.

**Fix**: Exclude in headless runs:

```bash
ctest --preset windows-msvc-release -E "AppTest\.Init_WithoutDisplay_ReturnsFalse"
```

## clang-cl link errors (oldnames.lib / msvcrtd.lib)

**Fix**: Run clang presets through `cmake-msvc.cmd` so `vcvars64.bat` sets library paths.

## Heap corruption at Init (CRT mismatch)

**Symptom**: Crash around `unigui::Init()` / first `NewFrame()` when embedding UniGUI.

**Cause**: Parent app and UniGUI linked against different CRTs (`/MD` vs `/MT`) or different vcpkg triplets.

**Fix**: Use the **submodule + parent vcpkg.json** workflow in [INTEGRATION.md](../INTEGRATION.md). Verify:

```bat
dumpbin /DIRECTIVES your_app.exe | findstr DEFAULTLIB
```

Expect consistent static CRT (`libcmt`) when using `x64-windows-static`.

## More help

Open an issue on [GitHub](https://github.com/Teamkiller131/TeamkillerUniGUI/issues) with preset name, OS, and the first error lines from configure/build.
