# TeamkillerUniGUI — 现代化 Dear ImGui C++ 封装库

[![C++23](https://img.shields.io/badge/C%2B%2B-23-blue)](https://en.cppreference.com/w/cpp/23)
[![CMake](https://img.shields.io/badge/CMake-3.31%2B-green)](https://cmake.org/)
[![vcpkg](https://img.shields.io/badge/vcpkg-managed-orange)](https://vcpkg.io/)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS%20%7C%20Web-lightgrey)]()
[![Version](https://img.shields.io/badge/version-3.8.2-blueviolet)]()
[![Tests](https://img.shields.io/badge/tests-1017-brightgreen)]()
[![Widgets](https://img.shields.io/badge/widgets-93-blue)]()
[![Backends](https://img.shields.io/badge/backends-7%20%284%20production%29-orange)]()

C++23 Dear ImGui 封装库——提供统一的明暗主题引擎、高层组件、声明式 DSL、CSS 样式引擎、插件系统与 EventBus。支持 7 种渲染后端：GLFW+OpenGL3、SDL3+Vulkan、DX11、DX12、Metal、WebGPU 和 Emscripten。

## 快速开始

```bash
git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git
cd TeamkillerUniGUI

# 默认：GLFW + OpenGL3
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release

# 或使用一键脚本（自动配置 MSVC 环境）
.\cmake-msvc.cmd

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

## 文档

| 文档 | 说明 |
|------|------|
| **[docs/README_zh.md](docs/README_zh.md)** | **文档索引（中文）** |
| [docs/GETTING_STARTED.md](docs/GETTING_STARTED.md) | 编译与首个程序 |
| [docs/EXAMPLES.md](docs/EXAMPLES.md) | 示例与常用模式 |
| [docs/WIDGET_API.md](docs/WIDGET_API.md) | 完整 Widget API |
| [docs/API_INDEX.md](docs/API_INDEX.md) | API 字母索引 |
| [docs/API_STABILITY.md](docs/API_STABILITY.md) | API 契约：语义化版本、稳定性分级、弃用流程 |
| [INTEGRATION.md](INTEGRATION.md) | 子模块集成 |
| [docs/TROUBLESHOOTING.md](docs/TROUBLESHOOTING.md) | 构建排障 FAQ |

## 架构

```
用户代码
    ↓
unigui:: API
    ├── 主题引擎 (明暗主题 + 13 套预设，统一样式/颜色令牌，毛玻璃材质，立体阴影，StyleScope RAII)
    ├── 组件库 (93 个组件，100% PushID 安全，表单校验/撤销重做/序列化)
    ├── 声明式 DSL (unigui::dsl — Window, VBox/HBox, Button, CheckBox, SliderFloat, InputText, If/For)
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
    │   ├── Metal          (macOS，桩 — 尚未实现)
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

bool enabled = true;
float gain = 0.5f;

auto ui = Window("DSL 示例", VBox({
    Text("欢迎!"),
    Separator(),
    HBox({
        Button("保存", ButtonVariant::Primary, []{ /* 动作 */ }),
        Button("退出",                          []{ std::exit(0); })
    }),
    CheckBox("启用", &enabled),             // 绑定到外部 bool
    SliderFloat("增益", &gain, 0.f, 1.f),   // 绑定到外部 float
    If([&]{ return enabled; }, Text("…运行中")),
    For(5, [](int i){ return Label("第 " + std::to_string(i+1) + " 项"); })
}));

// 渲染循环中调用：
Render(ui);
```

DSL 通过主题化的 `unigui::im` 层渲染，输出与工具包其余部分保持一致；有状态控件
（`CheckBox` / `SliderFloat` / `InputText`）既可通过指针绑定到外部变量，也可将状态
保存在保留的节点中——因此重复 `Render()` 同一棵树即可保留用户输入。

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

可选 vcpkg feature：`sqlite`、`config`、`ipc`、`network`

## 组件列表（93 个）

> 详尽的组件 API 与示例请参阅 **[docs/WIDGET_API.md](docs/WIDGET_API.md)**；
> 专题指南：**[TreeView 树形控件](docs/TREEVIEW.md)**、**[CascadingCombo 级联下拉](docs/CASCADINGCOMBO.md)**。


| 分类 | 组件 | 核心 API |
|------|------|----------|
| 容器 | Window | `AddPanel()`, `SetMenuBarEnabled()`, `SetPosition()` |
| | Panel | `SetContentCallback(fn)`, `SetWrapEnabled(bool)` |
| | GroupBox | `SetContentCallback(fn)`, `SetTitle()` |
| | TabWidget | `AddTab()`, `RemoveTab()` |
| | Card | `SetTitle()`, `SetContent(fn)`, `SetFooter(fn)` (v3.0) |
| | HeroSection | `SetTitle()`, `SetSubtitle()`, `SetActionButton()` (v3.0) |
| | CollapsingHeader | `SetContentCallback(fn)`, `SetOpen(bool)` (v3.3) |
| 输入 | LineEdit | `SetValue()`, `SetValidator()`, `Undo()`/`Redo()` |
| | MultiLine | `SetText()`, `Undo()`/`Redo()` |
| | PasswordInput | `GetStrengthScore()` (0-4)，显示/隐藏切换 |
| | ComboBox | `SetItems()`, `SetOnChange()`, `SetSearchable()` |
| | MultiCombo | `GetSelectedIndices()`, `SetSelected()` |
| | CascadingCombo | N 级联动下拉，横/纵排布 + 宽度调整（[指南](docs/CASCADINGCOMBO.md)） |
| | SearchBox | `SetItems(v)`, `GetQuery()`, `SetOnSelect(fn)` |
| | Slider\<T\> | `SetMin()`, `SetMax()`, `SetValue()` |
| | MultiHandleSlider | 多游标可拖拽滑块 (v3.2) |
| | DragFloat / DragInt | `GetValue()`, `SetValue()`, `SetSpeed()` (v3.3) |
| | InputInt/InputFloat | `GetValue()`, `SetValue()` |
| | SpinBox\<T\> | `GetValue()`, `SetRange()` |
| | DatePicker | `GetDate()`, `SetDate()` |
| | ColorPicker | `GetColor()`, `SetColor()` |
| | ColorEdit | `GetColor()`, `SetColor()` (v3.3) |
| | FilePath | `SetPath()`，文件选择 |
| | DirPath | 目录选择 |
| 选择 | Selectable | `SetLabel()`, `SetOnClick()`, `IsSelected()` (v3.3) |
| | ListBox | `SetItems()`, `GetSelectedIndex()`, `SetOnChange()` (v3.3) |
| 展示 | Label | `GetText()`, `SetText()` |
| | Button | `WasClicked()`, `SetEnabled()` |
| | ImageButton | `SetImage(texID, w, h)`, `SetLabel()` |
| | IconButton | `WasClicked()` |
| | Hyperlink | `WasClicked()` |
| | RichText | `SetSpans()`, `AddSpan()` |
| | Markdown | `SetMarkdown()`，支持 # ** * - [链接] |
| | Image | `SetTexture(texID)`，多种缩放模式 |
| | ProgressBar | `SetFraction()`，状态颜色 + 动画填充 |
| | Gauge | 环形/径向进度表盘，`SetSweepDegrees()`，中心标签 (v3.6) |
| | Sparkline | 内联折线/面积/柱状趋势图，`PushValue()`，趋势着色 (v3.6) |
| | MetricCard | KPI/持仓卡片：强调条 + 状态点 + 数值/涨跌/正文 (v3.7) |
| | ToggleButton | 启停双态按钮，可用谓词 + 禁用提示 (v3.7) |
| | ButtonGroup | 左/右/填充对齐的按钮组 (v3.7) |
| | StatusLamp | 多状态指示灯 + 辉光（`SetGlowEnabled`） |
| | RiskBar | 阈值着色的动画风险条 |
| | FuturesRiskBar | 实际/预估/隔夜多标记风险条 |
| | LoadingIndicator | `SetActive(bool)`，旋转动画 |
| | GradientText | `Render(text, leftColor, rightColor)` (v3.0) |
| 列表 | VirtualList | `SetItemCount(n)`, `SetItemGetter(fn)` — 10 万+ |
| | EditableDataGrid<T> | 列级单元格编辑器（无逐行缓存），运行时冻结 (v3.7) |
| | BasketTicket<T> | 可编辑篮子网格：增/删/导入/校验/提交 (v3.7) |
| | GroupedRiskTree | 风险树，最差/均值/求和上卷 + 阈值着色 (v3.7) |
| | DataTable\<T\> | 虚拟滚动、排序、分组行、行着色、内联编辑、复选框列、过滤 |
| | MultiSplitter | N面板可拖拽横/纵向布局 |
| | ListView | `SetItems()`, `SetOnSelect()` |
| | Table | `AddRow()`，可排序、单元格嵌入、`ExportCSV()`/`ImportCSV()` |
| | TreeView | `SetRoot()`，多选、复合行/自定义行（[指南](docs/TREEVIEW.md)） |
| 布局 | Splitter | `SetOrientation()`，拖拽调整 |
| | ScrollArea | `SetContentCallback(fn)` |
| | Separator | 水平/垂直分割线 |
| | Space | `DockSpace()` 停靠布局 |
| 导航 | MenuBar | `SetMenus()`，嵌套子菜单 |
| | Breadcrumb | `SetItems()`，路径导航 |
| | Wizard | `AddStep()`, `Next()`, `Previous()` |
| 弹窗 | Dialog | `Open()`, `Close()`，模态/非模态 |
| | ConfirmDialog | 确认弹窗，危险样式 |
| | AlertBar | 常驻动画横幅 |
| | Tooltip | `Show(text)`，悬停提示 |
| | ContextMenu | `Show()`，右键弹出菜单 |
| | Toast | `Toast::Info()`, `Success()`, `Warn()`, `Error()` |
| 表单 | Form | `AddTextField()`, `Validate()`, `Serialize()` |
| | PropertyGrid | `AddProperty({name, type, val})` |
| | CheckBox | `SetChecked()`, `OnChange()` |
| | RadioGroup | `SetSelected()`，单选项组 |
| | ToggleSwitch | `SetOn(bool)`，带动画过渡 |
| 其他 | DragDrop | `BeginDragSource<T>()`, `AcceptDragDrop<T>()` |
| | ConnectionStatusBar | 连接状态条：指示灯 + 延迟 + 走势 + 重连 (v3.7) |
| | PnlText / TagList | 极性感知盈亏文本 + 内联标签 (v3.7) |
| | TimeSeriesChart | 实时时序图、滑动窗口 |
| | PriceTicker | 滚动行情跑马灯，▲/▼ 涨跌着色，`SetSpeed()` (v3.6) |
| | SegmentedControl | 单选分段按钮组（1日/1周/1月），`SetOnChange()` (v3.6) |
| | SliderBar | 期货/价格档位条，含确认/回滚 |
| | ShortcutManager | `Register()`，全局快捷键 |
| | Notification | `Show()`，待处理计数 |
| | TrayIcon | `Show()`, `Hide()`, `SetMenu()`, `ShowNotification()` |
| | Tag | 彩色标签徽章 |
| | Badge | 圆点/计数/文字通知徽章 (v3.0) |
| | SkeletonScreen | 骨架屏占位 (v3.0) |
| | Shimmer | 骨架屏流光动画 (v3.0) |

## ID 安全

所有 93 个组件均通过 `PushID`/`PopID` 自动管理 ImGui ID 栈，无需手动处理 ID 冲突。

```cpp
auto btn1 = std::make_shared<unigui::Button>("ok", "确定");
auto btn2 = std::make_shared<unigui::Button>("cancel", "确定"); // 相同文字，不再冲突！
```

每个组件自动为其内部所有 ImGui 控件建立唯一 ID 作用域，即使创建多个相同标签的组件也能正常工作。

## 主题与表面材质

主题引擎在调色板之上叠加四层令牌（token）处理，使全部 13 套预设（以及明暗主题）
共享统一外观。默认采用毛玻璃 / glassmorphism 风格。

```cpp
unigui::AppConfig cfg;
cfg.theme.theme   = "dark";                              // 或任意注册表预设
cfg.theme.surface = unigui::theme::SurfaceStyle::Glass;  // 默认值，见下表
unigui::Init(cfg);
```

**表面材质**（`<unigui/theme/surface_style.h>`）——在调色板之上叠加一层
半透明「材质」。由 `ThemeConfig::surface` 选择：

| `SurfaceStyle` | 外观 |
|----------------|------|
| `Solid` | 平面、完全不透明——经典风格。 |
| `Glass` *(默认)* | 毛玻璃——半透明表面 + 亮边。 |
| `Frosted` | 更强半透明，更明显的亮边。 |
| `Acrylic` | Fluent 亚克力风格——更实的染色 + 边框。 |
| `Minimal` | 近乎不透明、无边框、低调。 |

`SurfaceStyleName()` / `AllSurfaceStyles()` 可用于主题选择器。半透明材质需要
衬在带色背景上：应用主循环会将各后端清屏为 `unigui::GetBackdropColor()`，
避免毛玻璃表面衬在纯黑上。

**强调色与语义色**（`<unigui/theme/color_tokens.h>`）——每套主题都从单一基础
强调色推导出完整交互调色板（accent → hover → active，以及
`Success`/`Warning`/`Danger`/`Info`）。查询当前调色板：

```cpp
ImVec4 ok = unigui::GetSemanticColor(unigui::theme::Semantic::Success);
const auto& tokens = unigui::GetColorTokens();   // accent/hover/active/success/...
```

**立体阴影 Elevation**（`<unigui/fx/elevation.h>`）——与当前表面材质联动的语义
阴影分级。毛玻璃得到柔和扩散的阴影 + 亮边；Solid 得到更硬朗的阴影。

```cpp
unigui::Button("save", "Save").WithElevation(unigui::fx::Elevation::Medium); // None/Low/Medium/High
```

## 后端选择

```cpp
// 运行时从 7 种后端中选择
cfg.backend = BackendType::GLFW_GL3;      // GLFW + OpenGL 3.3 ★
cfg.backend = BackendType::SDL3_Vulkan;   // SDL3 + Vulkan 1.3 ★
cfg.backend = BackendType::DX11;          // GLFW + DirectX 11 ★
cfg.backend = BackendType::DX12;          // GLFW + DirectX 12 ★
cfg.backend = BackendType::Metal;         // macOS Metal（桩，尚未实现）
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

- **Windows**：主力平台。Visual Studio 2022 + MSVC 19.40+。DX11 为默认后端。完整测试套件（895 项）全部通过。
- **Linux**：GCC 14+/Clang 18+，GLFW+OpenGL3。支持 X11/Wayland。无头环境下会跳过少量依赖 GL 上下文的测试。x64-linux triplet 依赖见 [vcpkg.json](vcpkg.json)。
- **macOS**：Apple 已弃用 OpenGL（上限 4.1），推荐通过 MoltenVK 使用 Vulkan。

## 开发工具

| 命令 | 说明 |
|------|------|
| `.\cmake-msvc.cmd` | 一键配置 MSVC 构建环境并运行 CMake 预设 |
| `cmake --preset windows-msvc-release` | 配置 CMake 预设（Release 模式） |
| `cmake --build --preset windows-msvc-release` | 构建项目 |
| `ctest --preset windows-msvc-release` | 运行全部测试 |

## 字体

UniGUI 直接将 **JetBrains Mono Nerd Font** 嵌入库二进制文件中，无需安装系统字体。所嵌入字体基于 SIL Open Font License 1.1 授权 —— 见 [fonts/LICENSE](fonts/LICENSE)。

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

源代码采用 **MIT License** 授权 —— 见 [LICENSE](LICENSE)。

随仓库分发的字体（`fonts/JetBrainsMonoNerdFont-Regular.ttf`）是 JetBrains Mono
的 Nerd Fonts 构建版本，单独采用 **SIL Open Font License 1.1** 授权 —— 见
[fonts/LICENSE](fonts/LICENSE)。
