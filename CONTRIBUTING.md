# Contributing to TeamkillerUniGUI

Thanks for your interest in contributing! This document explains how to build,
test, and submit changes.

## Code of Conduct

Be respectful and constructive. Harassment or hostile behavior is not tolerated.

## Getting Started

1. Fork the repository and clone your fork.
2. Make sure you have the prerequisites: CMake 3.31+, Ninja, a vcpkg checkout
   (`$VCPKG_ROOT`), and a C++23 compiler (MSVC 19.40+ / GCC 14+ / Clang 18+).
3. Build and run the tests to confirm a clean baseline (see below).

## Building & Testing

### Windows (MSVC)

```powershell
# Verify your toolchain first
pwsh -File scripts/check_env.ps1

# Configure + build + test
pwsh -File scripts/build.ps1 -Preset windows-msvc-debug -Test
```

Or manually via the MSVC wrapper:

```powershell
cmake-msvc.cmd --preset windows-msvc-debug
cmake-msvc.cmd --build --preset windows-msvc-debug
ctest --preset windows-msvc-debug --output-on-failure
```

### Linux / macOS

```bash
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -G Ninja -DCMAKE_BUILD_TYPE=Debug -DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF
cmake --build build
ctest --test-dir build --output-on-failure
```

## Coding Standards

- **Format**: run `clang-format` before committing. A `.clang-format` is provided:
  ```bash
  clang-format -i <changed-files>
  ```
- **Lint**: `.clang-tidy` is provided; the `windows-clang-tidy` preset runs it.
- **C++23**: prefer modern, standard C++; avoid platform-specific APIs outside the
  backend layer.
- **Widget IDs**: every widget must scope its ImGui IDs via `PushID/PopID` (see
  the ID Safety section in the README).
- **Tests**: add or update GoogleTest cases under `tests/` for any new behavior.

## Submitting Changes

1. Create a topic branch: `git checkout -b feat/my-change` (or `fix/...`, `docs/...`).
2. Keep commits focused; write clear messages explaining the *why*.
3. Ensure the full test suite passes and CI is green.
4. Open a Pull Request against `master` and fill in the PR template.

## Reporting Issues

Use the issue templates for bug reports and feature requests. For bugs, include
your OS, compiler, backend, reproduction steps, and (if possible) a minimal example.

## License

By contributing, you agree that your contributions are licensed under the
project's [MIT License](LICENSE).
