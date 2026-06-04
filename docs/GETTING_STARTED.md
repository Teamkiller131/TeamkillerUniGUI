# Getting Started

## Requirements

| Tool | Version |
|------|---------|
| C++ compiler | C++23 (MSVC 2022, GCC 14+, Clang 16+) |
| CMake | ≥ 3.26 (presets target 3.31+) |
| Ninja | Required by CMake presets |
| vcpkg | For dependencies ([install guide](https://github.com/microsoft/vcpkg)) |

**Windows**: Visual Studio 2022 with **Desktop development with C++**.

## Clone

```bash
git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git
cd TeamkillerUniGUI
```

## Windows (recommended)

```powershell
pwsh -File scripts/check_env.ps1
pwsh -File scripts/build.ps1
pwsh -File scripts/build.ps1 -Preset windows-msvc-debug -Test
```

`build.ps1` runs `cmake-msvc.cmd` so the MSVC toolset and Ninja are always correct.

### Manual preset build

```powershell
cmake-msvc.cmd --preset windows-msvc-release
cmake-msvc.cmd --build --preset windows-msvc-release
ctest --preset windows-msvc-release
```

### Run the demo

```powershell
.\build\windows-msvc-release\examples\hello_unigui\hello_unigui.exe --frames 300
```

## Linux

```bash
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_TARGET_TRIPLET=x64-linux \
  -G Ninja \
  -DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF
cmake --build build
ctest --test-dir build
```

## Minimal application

```cpp
#include <unigui/unigui.h>

int main() {
    unigui::AppConfig cfg;
    cfg.title = "My App";
    cfg.width = 1280;
    cfg.height = 720;
    // Windows default: BackendType::DX11
    return unigui::RunApp(cfg, [] {
        ImGui::Text("Hello, UniGUI %s", UNIGUI_VERSION_STRING);
    });
}
```

`RunApp` returns `0` on success, `1` if `Init` failed. Pass `maxFrames` for CI:

```cpp
return unigui::RunApp(cfg, uiCallback, /*maxFrames=*/60);
```

## Manual render loop

Use when you integrate with your own engine (e.g. ASIO, game loop):

```cpp
if (!unigui::Init(cfg)) return 1;
while (!unigui::ShouldClose()) {
    unigui::NewFrame();
    unigui::ProcessMainThreadTasks();
    drawUi();
    unigui::Render();
}
unigui::Shutdown();
```

## Theme at startup

```cpp
unigui::AppConfig cfg;
cfg.theme.preset = unigui::ThemePreset::Dark;  // or Light
cfg.theme.font_size = 16.f;
cfg.theme.surface = unigui::theme::SurfaceStyle::Glass;
unigui::Init(cfg);
```

Named presets (Dracula, Nord, Catppuccin, …): `unigui::theme::ThemeRegistry::Apply("dracula")` after `Init`.

## Link in your CMake project

```cmake
target_link_libraries(my_app PRIVATE unigui::unigui)
```

Embedding as a submodule with a single vcpkg triplet is documented in [INTEGRATION.md](../INTEGRATION.md).

## Next steps

- [EXAMPLES.md](EXAMPLES.md) — common patterns  
- [WIDGET_API.md](WIDGET_API.md) — full reference  
- [TROUBLESHOOTING.md](TROUBLESHOOTING.md) — build issues  
