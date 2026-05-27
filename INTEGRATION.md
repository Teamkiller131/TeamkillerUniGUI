# UniGUI Integration Guide — Submodule Method

> **推荐方式**：Git submodule。父项目的 vcpkg 统一管理**所有**依赖（包括 UniGUI 需要的 imgui/glfw3/freetype 等），从根源消除 CRT triplet 冲突。`add_subdirectory()` 方式已废弃——具体原因见 §CRT 问题。

## 1. 添加 Submodule

```bash
cd your_project
git submodule add https://xbw-nas.iepose.cn/Teamkiller131/TeamkillerUniGUI.git third_party/TeamkillerUniGUI
git submodule update --init --recursive
```

## 2. 父项目 vcpkg.json（统一管理所有依赖）

```json
{
  "name": "trader-client",
  "version": "1.0.0",
  "dependencies": [
    { "name": "imgui", "default-features": false,
      "features": ["docking-experimental","freetype","glfw-binding","opengl3-binding","dx11-binding"] },
    { "name": "imgui", "default-features": false, "platform": "windows",
      "features": ["dx12-binding"] },
    "glfw3",
    { "name": "glad", "features": ["loader"] },
    "freetype",
    "implot",
    "imgui-node-editor",
    "spdlog",
    "gtest"
  ]
}
```

> 可选依赖（量化交易项目一般不需要）：`nlohmann-json`, `cpptoml`, `sqlite3`, `cppzmq`, `cpp-httplib`, `ixwebsocket`

## 3. 父项目 CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.26)
project(trader_client LANGUAGES CXX)

# ⚠️ 必须 BEFORE project() —— vcpkg 需要 triplet 在最前面
set(VCPKG_TARGET_TRIPLET "x64-windows-static" CACHE STRING "")
set(CMAKE_TOOLCHAIN_FILE "/path/to/your/vcpkg/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "" FORCE)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    CACHE STRING "")

# ── UniGUI 模块裁剪 ─────────────────────────────────────────────────────
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

# ── 主程序 ───────────────────────────────────────────────────────────────
target_link_libraries(trader_client PRIVATE unigui::unigui)
```

## 4. 构建

```bash
# 首次构建（vcpkg 会为 x64-windows-static triplet 编译所有依赖）
rm -rf build
cmake -B build -S . -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build

# 验证 CRT 一致
dumpbin /DIRECTIVES build/trader_client.exe | findstr DEFAULTLIB
# 期望输出：libcmt（静态 CRT）——不应出现 msvcrt
```

配置时 UniGUI 会打印 CRT 类型确认：
```
UniGUI CRT: /MT (Static)
```

## 5. 与 ASIO 主循环共存

```cpp
unigui::AppConfig cfg;
cfg.backend = unigui::BackendType::DX11;
unigui::Init(cfg);

while (running) {
    io_context.poll();              // ASIO 网络
    unigui::NewFrame();             // ImGui 帧开始（自动调用 ProcessMainThreadTasks）
    RenderTraderUI();               // 业务 UI
    unigui::Render();               // ImGui 帧结束 + Present
}
unigui::Shutdown();
```

## 6. 线程安全：网络回调 → UI

```cpp
// ASIO 回调线程
void on_market_data(const MarketData& data) {
    unigui::InvokeOnMainThread([data]() {
        positions.push_back(data);          // 主线程安全
        dataTable->ScrollToRow(positions.size() - 1);
    });
}
```

---

## CRT 问题：为什么 add_subdirectory() 已废弃

### 症状

`0xC0000374` heap corruption，崩在 `unigui::Init()` 内 DPI 检测之后、首帧 `NewFrame()` 之前。

### 根因

`add_subdirectory()` 时 UniGUI 的 `src/CMakeLists.txt` 内 `find_package(imgui)` 会在 UniGUI 自己的 `build/` 下创建独立的 `vcpkg_installed/`。即使父项目设了 `x64-windows-static`，vcpkg 为子项目单独编译依赖时**可能回退到默认 triplet（x64-windows, /MD）**。

结果：ImGui 用 /MD 编译（`malloc` 来自 `msvcrt.dll`），父项目用 /MT（`free` 来自 `libcmt.lib`）。`ImGui::CreateContext()` 内分配的帧缓冲在 `io.Fonts->Build()` 时被不同 CRT 释放 → 堆损坏。

### 为什么清 build/ 重配不能彻底解决

`find_package(imgui)` 的查找路径受 `CMAKE_PREFIX_PATH` 和 vcpkg 工具链影响。如果系统中残留了之前用 `x64-windows` triplet 安装的 imgui CMake 配置文件，`find_package` 可能匹配到错误的包——即使子项目的 build 目录是全新的。

### Submodule 方式的优势

- 父项目的 vcpkg.json 包含**所有**依赖 → 一个 triplet 编译一切
- 没有子项目的独立 `vcpkg_installed/` → 不存在包查找路径歧义
- 父项目和子项目的 `.cpp` 文件共享完全相同的 CRT → ABI 100% 一致
- `find_package()` 全部命中父项目的 vcpkg 安装树 → 无版本/ABI 漂移

---

## UniGUI Option 速查

| Option | Default | 说明 |
|--------|---------|------|
| `UNIGUI_MODULE_WIDGETS` | ON | 核心组件库 |
| `UNIGUI_MODULE_EVENTS` | ON | EventBus |
| `UNIGUI_MODULE_PLUGIN` | ON | 插件系统 |
| `UNIGUI_MODULE_DSL` | ON | 声明式 DSL |
| `UNIGUI_MODULE_STYLING` | ON | CSS 引擎 |
| `UNIGUI_MODULE_FONTS` | ON | 字体管理器 |
| `UNIGUI_MODULE_SQLITE` | OFF | SQLite |
| `UNIGUI_MODULE_CONFIG` | OFF | TOML/JSON |
| `UNIGUI_MODULE_IPC` | OFF | ZMQ + 共享内存 |
| `UNIGUI_MODULE_NETWORK` | OFF | HTTP/WebSocket |
| `UNIGUI_BACKEND_DX11` | ON | DX11 后端 |
| `UNIGUI_BACKEND_DX12` | OFF | DX12 后端 |
| `UNIGUI_BUILD_TESTS` | ON | 测试套件 |
| `UNIGUI_BUILD_EXAMPLES` | ON | 示例程序 |
