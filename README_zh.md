# TeamkillerUniGUI — 现代化 Dear ImGui C++ 封装库

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-green)](https://cmake.org/)
[![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange)](https://vcpkg.io/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Web-lightgrey)]()
[![Version](https://img.shields.io/badge/version-3.2.1-blueviolet)]()
[![Tests](https://img.shields.io/badge/tests-244%20(236%2F244%20Linux)-brightgreen)]()
[![Widgets](https://img.shields.io/badge/widgets-66-blue)]()
[![Backends](https://img.shields.io/badge/backends-7%20%284%20production%29-orange)]()

C++23 Dear ImGui 封装库——提供统一的明暗主题引擎、高层组件、声明式 DSL、CSS 样式引擎、插件系统与 EventBus。支持 7 种渲染后端：GLFW+OpenGL3、SDL3+Vulkan、DX11、DX12、Metal、WebGPU 和 Emscripten。

## 快速开始

```bash
git clone https://xbw-nas.iepose.cn/Teamkiller131/TeamkillerUniGUI.git
cd TeamkillerUniGUI

# 默认：GLFW + OpenGL3
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release

# SDL3 + Vulkan
cmake --preset windows-msvc-sdl3-vulkan-release
cmake --build --preset windows-msvc-sdl3-vulkan-release

# 运行示例
./build/windows-msvc-release/examples/hello_unigui/hello_unigui.exe --frames 10

# Linux (Fedora 43 / Rocky 9, GCC 14+, CMake 3.26+)
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=~/vcpkg/scripts/buildsystems/vcpkg.cmake -DVCPKG_TARGET_TRIPLET=x64-linux -G Ninja -DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF
cmake --build build
ctest --test-dir build
```

## 架构

```
用户代码
    ↓
unigui:: API
    ├── 主题引擎 (53 色明暗主题，StyleScope RAII)
    ├── 组件库 (66 个组件，表单校验/撤销重做/序列化)
    ├── 声明式 DSL (unigui::dsl — Window, VBox, HBox, Button, For, If)
    ├── 事件总线 (unigui::events::Bus — 发布/订阅，支持通配符)
    ├── CSS 样式引擎 (unigui::styling::Engine — 选择器引擎 + 变量)
    ├── 插件系统 (unigui::plugin::Manager — DLL 热加载/热卸载)
    ├── 字体管理器 (unigui::fonts::Manager — 多字体 + 回退链)
    ├── 配置层 (unigui::config::Store — TOML/JSON/INI 统一读取)
    ├── SQLite (unigui::sqlite::Database — 轻量封装 + 迁移)
    ├── IPC (unigui::ipc — 共享内存 + ZMQ 通道)
    ├── 后端抽象 (PlatformBackend / RendererBackend 接口)
    │   ├── GLFW + OpenGL 3.3 ★ (默认，生产级)
    │   ├── SDL3 + Vulkan 1.3 ★ (生产级)
    │   ├── GLFW + DX11 ★ (生产级，Windows 默认)
    │   ├── GLFW + DX12 ★ (生产级)
    │   ├── Metal          (macOS，Windows 上为桩)
    │   ├── WebGPU         (跨平台，桩)
    │   └── Emscripten     (Web/HTML5，桩)
    └── App 启动器 (Init / Run / NewFrame / Render)
    ↓
ImGui (v1.92.8，docking + multi-viewport)
```

## API 概览

### 核心循环

```cpp
#include <unigui/unigui.h>

unigui::AppConfig cfg;
cfg.backend = unigui::BackendType::DX11; // 或者 GLFW_GL3, SDL3_Vulkan, DX12
unigui::Init(cfg);

while (!unigui::ShouldClose()) {
    unigui::NewFrame();
    ImGui::ShowDemoWindow(); // 原生 ImGui 仍然可用，自动套用主题
    unigui::Render();
}
unigui::Shutdown();
```

### 声明式 DSL

```cpp
#include <unigui/dsl/dsl.h>
using namespace unigui::dsl;

auto ui = Window("DSL 示例", VBox({
    Text("欢迎!"),
    Separator(),
    HBox({
        Button("点击我", []{ /* 动作 */ }),
        Button("退出",   []{ std::exit(0); })
    }),
    For(5, [](int i){ return Label("第 " + std::to_string(i+1) + " 项"); })
}));

// 渲染循环中调用：
Render(ui);
```

### EventBus 事件总线

```cpp
#include <unigui/events/eventbus.h>
using namespace unigui::events;

auto id = Bus::Instance().Subscribe("window.*", [](auto& e) {
    // 响应 window.close / window.resize 等事件
});
Bus::Instance().Publish("window.close", std::string("main"));
```

### CSS 样式引擎

```cpp
#include <unigui/styling/style_engine.h>
using namespace unigui::styling;

Engine::Instance().Parse(R"(
    Window { bg: #1a1a2e; rounding: 8 }
    Button.primary { bg: #e94560; rounding: 4 }
    Button:hover { bg: #ff6b6b }
)");
Engine::Instance().ApplyAll();
```

### 插件系统

```cpp
#include <unigui/plugin/plugin_manager.h>
using namespace unigui::plugin;

auto* p = Manager::Instance().Load("my_plugin.dll");
if (p) { p->Init(); /* 每帧: p->Render(); */ }
```

### 模块化 CMake（按需编译）

```bash
# 最小构建 (核心 + DX11, ~200 目标)
cmake -DUNIGUI_MODULE_WIDGETS=OFF -DUNIGUI_MODULE_DSL=OFF \
      -DUNIGUI_MODULE_EVENTS=OFF -DUNIGUI_MODULE_PLUGIN=OFF ...

# 完整构建 (全部模块)
cmake -DUNIGUI_MODULE_SQLITE=ON -DUNIGUI_MODULE_CONFIG=ON \
      -DUNIGUI_MODULE_IPC=ON -DUNIGUI_BACKEND_DX12=ON ...
```

## 组件列表（66 个）

| 分类 | 组件 | 核心 API |
|------|------|----------|
| 容器 | Window | `AddPanel()`, `SetMenuBarEnabled()`, `SetPosition()` |
| | Panel | `SetContentCallback(fn)`, `SetWrapEnabled(bool)` |
| | GroupBox | `SetContentCallback(fn)`, `SetTitle()` |
| | TabWidget | `AddTab()`, `RemoveTab()` |
| | Card | `SetTitle()`, `SetContent(fn)`, `SetFooter(fn)` (v3.0) |
| | HeroSection | `SetTitle()`, `SetSubtitle()`, `SetActionButton()` (v3.0) |
| 输入 | LineEdit | `SetValue()`, `SetValidator()`, `Undo()`/`Redo()` |
| | MultiLine | `SetText()`, `Undo()`/`Redo()` |
| | PasswordInput | `GetStrengthScore()` (0-4)，显示/隐藏切换 |
| | ComboBox | `SetItems()`, `SetOnChange()`, `SetSearchable()` |
| | MultiCombo | `GetSelectedIndices()`, `SetSelected()` |
| | SearchBox | `SetItems(v)`, `GetQuery()`, `SetOnSelect(fn)` |
| | Slider\<T\> | `SetMin()`, `SetMax()`, `SetValue()` |
| | MultiHandleSlider | 多游标可拖拽滑块 (v3.2) |
| | InputInt/InputFloat | `GetValue()`, `SetValue()` |
| | SpinBox\<T\> | `GetValue()`, `SetRange()` |
| | DatePicker | `GetDate()`, `SetDate()` |
| | ColorPicker | `GetColor()`, `SetColor()` |
| | FilePath | `SetPath()`，文件选择 |
| | DirPath | 目录选择 |
| 展示 | Label | `GetText()`, `SetText()` |
| | Button | `WasClicked()`, `SetEnabled()` |
| | ImageButton | `SetImage(texID, w, h)`, `SetLabel()` |
| | IconButton | `WasClicked()` |
| | Hyperlink | `WasClicked()` |
| | RichText | `SetSpans()`, `AddSpan()` |
| | Markdown | `SetMarkdown()`，支持 # ** * - [链接] |
| | Image | `SetTexture(texID)`，多种缩放模式 |
| | ProgressBar | `SetFraction()`，状态颜色 + 动画填充 |
| | LoadingIndicator | `SetActive(bool)`，旋转动画 |
| | GradientText | `Render(text, leftColor, rightColor)` (v3.0) |
| 列表 | VirtualList | `SetItemCount(n)`, `SetItemGetter(fn)` — 10 万+ |
| | DataTable\<T\> | 虚拟滚动、排序、行着色、内联编辑、过滤 (v3.2) |
| | ListView | `SetItems()`, `SetOnSelect()` |
| | Table | `AddRow()`, `ExportCSV()`, `ImportCSV()` |
| | TreeView | `SetRoot()`，多选支持 |
| 布局 | Splitter | `SetOrientation()`，拖拽调整 |
| | ScrollArea | `SetContentCallback(fn)` |
| | Separator | 水平/垂直分割线 |
| | Space | `DockSpace()` 停靠布局 |
| 导航 | MenuBar | `SetMenus()`，嵌套子菜单 |
| | Breadcrumb | `SetItems()`，路径导航 |
| | Wizard | `AddStep()`, `Next()`, `Previous()` |
| 弹窗 | Dialog | `Open()`, `Close()`，模态/非模态 |
| | Tooltip | `Show(text)`，悬停提示 |
| | ContextMenu | `Show()`，右键弹出菜单 |
| | Toast | `Toast::Info()`, `Success()`, `Warn()`, `Error()` |
| 表单 | Form | `AddTextField()`, `Validate()`, `Serialize()` |
| | PropertyGrid | `AddProperty({name, type, val})` |
| | CheckBox | `SetChecked()`, `OnChange()` |
| | RadioGroup | `SetSelected()`，单选项组 |
| | ToggleSwitch | `SetOn(bool)`，带动画过渡 |
| 其他 | DragDrop | `BeginDragSource<T>()`, `AcceptDragDrop<T>()` |
| | TimeSeriesChart | 实时时序图、滑动窗口 (v3.2) |
| | ShortcutManager | `Register()`，全局快捷键 |
| | Notification | `Show()`，待处理计数 |
| | TrayIcon | `Show()`, `Hide()`, `SetMenu()`, `ShowNotification()` |
| | Tag | 彩色标签徽章 |
| | Badge | 圆点/计数/文字通知徽章 (v3.0) |
| | SkeletonScreen | 骨架屏占位 (v3.0) |
| | Shimmer | 骨架屏流光动画 (v3.0) |

## 后端选择

```cpp
// 运行时从 7 种后端中选择
cfg.backend = BackendType::GLFW_GL3;      // GLFW + OpenGL 3.3 ★
cfg.backend = BackendType::SDL3_Vulkan;   // SDL3 + Vulkan 1.3 ★
cfg.backend = BackendType::DX11;          // GLFW + DirectX 11 ★
cfg.backend = BackendType::DX12;          // GLFW + DirectX 12 ★
cfg.backend = BackendType::Metal;         // macOS Metal（Win 上为桩）
cfg.backend = BackendType::WebGPU;        // Dawn/WGPU（桩）
cfg.backend = BackendType::Emscripten;    // Web/HTML5（桩）
```

| 后端 | 平台 | 图形 API | 状态 | MSAA |
|------|------|----------|------|------|
| GLFW+GL3 | Win/Lin/Mac | OpenGL 3.3 | ★ 生产级 | 4x |
| SDL3+Vulkan | Win/Lin/Mac | Vulkan 1.3 | ★ 生产级 | 可配 |
| GLFW+DX11 | Windows | DirectX 11 | ★ 生产级 | 4x |
| GLFW+DX12 | Windows | DirectX 12 | ★ 生产级 | 可配 |
| Metal | macOS | Metal 2 | 桩 | 原生 |
| WebGPU | 跨平台 | Dawn/WGPU | 桩 | 原生 |
| Emscripten | Web | WebGL/WebGPU | 桩 | 浏览器 |

## 子模块

| 模块 | 命名空间 | 头文件 |
|------|----------|--------|
| 声明式 DSL | `unigui::dsl` | `<unigui/dsl/dsl.h>` |
| 事件总线 | `unigui::events` | `<unigui/events/eventbus.h>` |
| CSS 样式 | `unigui::styling` | `<unigui/styling/style_engine.h>` |
| 字体管理 | `unigui::fonts` | `<unigui/fonts/font_manager.h>` |
| 插件系统 | `unigui::plugin` | `<unigui/plugin/plugin_manager.h>` |
| 配置 (TOML/JSON/INI) | `unigui::config` | `<unigui/config/config.h>` |
| SQLite 数据库 | `unigui::sqlite` | `<unigui/sqlite/database.h>` |
| IPC (共享内存 + ZMQ) | `unigui::ipc` | `<unigui/ipc/shmem.h>`, `<unigui/ipc/ipc.h>` |
| HTTP / WebSocket | `unigui::network` | `<unigui/network/network.h>` |

所有子模块头文件也会通过 `<unigui/unigui.h>` 一并引入，方便使用。

## 平台说明

- **Windows**：主力平台。Visual Studio 2022 + MSVC 19.40+。DX11 为默认后端。244/244 测试通过。
- **Linux**：GCC 14+/Clang 18+，GLFW+OpenGL3。支持 X11/Wayland。236/244 测试通过（8 项 GL 上下文失败为无头环境预期）。x64-linux triplet 依赖见 [vcpkg.json](vcpkg.json)。
- **macOS**：Apple 已弃用 OpenGL（上限 4.1），推荐通过 MoltenVK 使用 Vulkan。

## 字体

UniGUI 直接将 **JetBrains Mono Nerd Font** 嵌入库二进制文件中，无需安装系统字体。

```cpp
unigui::AppConfig cfg;
cfg.theme.font_path = "C:/path/to/my-font.ttf"; // 自定义字体
cfg.theme.font_size = 20.0f;                     // 96 DPI 下的逻辑像素
```

**CJK 支持**：Windows 上自动合并微软雅黑 (msyh.ttc) 以支持中日韩文字。Linux/macOS 上跳过自动合并——用户可通过 `ThemeConfig::font_path` 提供自定义 CJK 字体。

**Nerd Font 图标**：嵌入字体内含 Nerd Font 图标（`` `` `` `` 等），可在任意 ImGui 文本中使用。

## 依赖 (vcpkg)

```
imgui (1.92.8，docking+freetype+全部后端绑定)
implot (1.0)，imgui-node-editor (0.9.3)
glfw3，sdl3，vulkan，glad，freetype，gtest，spdlog
可选：sqlite3，cpptoml，nlohmann-json，cppzmq，cpp-httplib，ixwebsocket
Windows：d3d11，d3d12，d3dcompiler，dxgi，dxguid
```

## License

MIT
