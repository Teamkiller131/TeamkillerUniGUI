# UniGUI Integration Guide

## add_subdirectory() Integration

```cmake
# jzdz root CMakeLists.txt
cmake_minimum_required(VERSION 3.26)
project(trader_client LANGUAGES CXX)

# MUST be set before project() — vcpkg toolchain needs triplet first
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE "path/to/TeamkillerUniGUI/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "" FORCE)

set(UNIGUI_MODULE_PLUGIN   OFF CACHE BOOL "")
set(UNIGUI_MODULE_DSL      OFF CACHE BOOL "")
set(UNIGUI_MODULE_SQLITE   OFF CACHE BOOL "")
set(UNIGUI_MODULE_CONFIG   OFF CACHE BOOL "")
set(UNIGUI_MODULE_IPC      OFF CACHE BOOL "")
set(UNIGUI_MODULE_NETWORK  OFF CACHE BOOL "")
set(UNIGUI_MODULE_STYLING  OFF CACHE BOOL "")
set(UNIGUI_MODULE_FONTS    OFF CACHE BOOL "")
set(UNIGUI_BUILD_TESTS     OFF CACHE BOOL "")
set(UNIGUI_BUILD_EXAMPLES  OFF CACHE BOOL "")
set(UNIGUI_BACKEND_DX12    OFF CACHE BOOL "")
set(UNIGUI_BACKEND_DX11    ON  CACHE BOOL "")

add_subdirectory(third_party/TeamkillerUniGUI)
target_link_libraries(trader_client PRIVATE unigui::unigui)
```

## CRT Consistency (MUST READ)

UniGUI does NOT force any CRT. It inherits CMAKE_MSVC_RUNTIME_LIBRARY from the parent or vcpkg triplet.

### Symptoms

| Symptom | Cause |
|---------|-------|
| 0xC0000374 heap corruption | vcpkg cached .lib from previous triplet (/MD mixed into /MT) |
| LNK2038 RuntimeLibrary mismatch | /MT vs /MD mismatch across TUs |
| unresolved external __imp_* | Static CRT project links dynamic CRT deps |

### Fix

```bash
# 1. Clean UniGUI old build artifacts
rm -rf third_party/TeamkillerUniGUI/build

# 2. Clean jzdz build cache
rm -rf build

# 3. Reconfigure (vcpkg rebuilds all deps for the triplet)
cmake -B build -S . -G Ninja -DVCPKG_TARGET_TRIPLET=x64-windows-static -DCMAKE_BUILD_TYPE=Release

# 4. Verify CRT consistency
cmake --build build
```

### Verify

```powershell
dumpbin /DIRECTIVES build\trader_client.exe | Select-String "DEFAULTLIB"
# Expected: /DEFAULTLIB:libcmt   (static CRT /MT)
# Should NOT see: /DEFAULTLIB:msvcrt (dynamic CRT /MD)
```

## ASIO Main Loop Coexistence

```cpp
unigui::Init(cfg);
while (running) {
    io_context.poll();
    unigui::NewFrame();    // auto-calls ProcessMainThreadTasks()
    RenderTraderUI();
    unigui::Render();
}
unigui::Shutdown();
```

## Thread-Safe UI Updates

```cpp
void on_market_data(const MarketData& data) {
    unigui::InvokeOnMainThread([data]() {
        positions->push_back(data);
        dataTable->ScrollToRow(positions->size() - 1);
    });
}
```

## UniGUI Option Reference

| Option | Default | Notes |
|--------|---------|-------|
| UNIGUI_MODULE_WIDGETS | ON | Core widgets (KEEP ON) |
| UNIGUI_MODULE_EVENTS | ON | EventBus (recommended ON) |
| UNIGUI_MODULE_PLUGIN | ON | Plugin system |
| UNIGUI_MODULE_DSL | ON | Declarative DSL |
| UNIGUI_MODULE_STYLING | ON | CSS engine |
| UNIGUI_MODULE_FONTS | ON | Font manager |
| UNIGUI_MODULE_SQLITE | OFF | SQLite wrapper |
| UNIGUI_MODULE_CONFIG | OFF | TOML/JSON config |
| UNIGUI_MODULE_IPC | OFF | ZMQ + shared memory |
| UNIGUI_MODULE_NETWORK | OFF | HTTP/WebSocket |
| UNIGUI_BACKEND_DX11 | ON | DX11 backend |
| UNIGUI_BACKEND_DX12 | OFF | DX12 backend |
| UNIGUI_BUILD_TESTS | ON | Test suite |
| UNIGUI_BUILD_EXAMPLES | ON | Example programs |
