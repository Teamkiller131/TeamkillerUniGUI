# UniGUI v2.8 — 模块化拆分（最终方案）

> 纯架构重构：代码移动 + CMake 拆分 + 统一命名空间。
> **零 breaking change**（只加 `[[deprecated]]`，不删 API）。
> 版本号 **v2.8**（不跳到 3.0——SDK 内部重组织，用户 API 不动）。

---

## 全部模块（20个）

### ★ 必需
| 模块 | 说明 | 外部依赖 |
|------|------|---------|
| **unigui-core** | Widget基类 + Theme + App + 基础Widget (Window/Panel/Button/Label/CheckBox...) | imgui, glfw3, glad, freetype, spdlog, implot |

### 🧩 扩展 Widget
| 模块 | 说明 |
|------|------|
| **unigui-widgets** | VirtualList, MultiCombo, PropertyGrid, SearchBox, Toast, PasswordInput, Wizard, TrayIcon, RichText, ImageButton, Markdown, DragDrop, ShortcutManager... |

### 🖥️ 后端（可同时编译，运行时切换）
| 模块 | 平台 | 状态 | 外部依赖 |
|------|------|------|---------|
| **unigui-backend-glfw3** | Win/Lin/Mac | ★ Production | - |
| **unigui-backend-dx11** | Windows | ★ Production | d3d11, d3dcompiler, dxgi |
| **unigui-backend-dx12** | Windows | ✓ Runtime | d3d12 |
| **unigui-backend-vulkan** | Win/Lin/Mac | ★ Production | sdl3, vulkan |
| **unigui-backend-metal** | macOS | △ Stub | (Objective-C++) |
| **unigui-backend-webgpu** | Cross | △ Stub | Dawn/WGPU |
| **unigui-backend-emscripten** | Web | △ Stub | Emscripten SDK |

### 🔧 功能模块（可选）
| 模块 | 说明 | 外部依赖 |
|------|------|---------|
| **unigui-locale** | i18n 翻译系统 | - |
| **unigui-settings** | Settings 持久化 + MRU | - |
| **unigui-sqlite** | SQLite3 wrapper | sqlite3 |
| **unigui-config** | TOML/JSON/INI 统一配置 | cpptoml, nlohmann-json |
| **unigui-network** | HTTP + WebSocket | cpp-httplib, ixwebsocket |
| **unigui-ipc** | SharedMemory + ZMQ | cppzmq |
| **unigui-plugin** | 插件系统 (原 v2::PluginManager) | - |
| **unigui-dsl** | 声明式 UI DSL (原 v2::dsl) | - |
| **unigui-events** | EventBus 发布订阅 (原 v2::EventBus) | - |
| **unigui-styling** | CSS 样式引擎 (原 v2::StyleEngine) | - |
| **unigui-fonts** | 多字体管理 (原 v2::FontManager) | - |

---

## 命名空间统一

| 旧 (v2::) | 新 |
|-----------|-----|
| `unigui::v2::PluginManager` | `unigui::plugin::Manager` |
| `unigui::v2::dsl::Window/VBox` | `unigui::dsl::Window/VBox` |
| `unigui::v2::EventBus` | `unigui::events::Bus` |
| `unigui::v2::StyleEngine` | `unigui::styling::Engine` |
| `unigui::v2::FontManager` | `unigui::fonts::Manager` |
| `unigui::v2::Database` | `unigui::sqlite::Database` |
| `unigui::v2::Config` | `unigui::config::Store` |

v2 命名空间下保留 `using` alias + `[[deprecated]]` 指向新位置。

---

## 三档预设

### `recommended`（推荐）
```
core + widgets + dx11 + dx12 + vulkan + glfw3
+ locale + settings + dsl + styling + fonts
```
理由：最常用的 4 后端 + 扩展 widgets + 声明式 UI + 样式 + 字体。vcpkg 一行安装。

### `minimal`（最小）
```
core + dx11
```
理由：Windows 开箱即用，最小体积。

### `full`（全部）
```
所有 20 个模块
```

---

## vcpkg.json

```json
{
  "name": "unigui",
  "version": "2.8.0",
  "default-features": [],
  "features": {
    "core":           { "dependencies": ["imgui","glfw3","glad","freetype","spdlog","implot"] },
    "widgets":        { "description": "Extended widgets" },
    "locale":         { "description": "i18n" },
    "settings":       { "description": "Settings" },
    "sqlite":         { "dependencies": ["sqlite3"] },
    "config":         { "dependencies": ["cpptoml","nlohmann-json"] },
    "network":        { "dependencies": ["cpp-httplib","ixwebsocket"] },
    "ipc":            { "dependencies": ["cppzmq"] },
    "plugin":         { "description": "Plugin system" },
    "dsl":            { "description": "Declarative UI" },
    "events":         { "description": "EventBus" },
    "styling":        { "description": "CSS engine" },
    "fonts":          { "description": "Font manager" },
    "backend-glfw3":  { "description": "GLFW+OpenGL3" },
    "backend-dx11":   { "platform": "windows", "dependencies": ["d3d11","d3dcompiler","dxgi"] },
    "backend-dx12":   { "platform": "windows", "dependencies": ["d3d12"] },
    "backend-vulkan": { "dependencies": ["sdl3","vulkan"] },

    "recommended": { "description": "Recommended install",
      "dependencies": ["core","widgets","backend-dx11","backend-dx12","backend-vulkan","backend-glfw3",
                       "locale","settings","dsl","styling","fonts"] },
    "full": { "dependencies": ["recommended","sqlite","config","network","ipc","plugin","events",
                               "backend-metal","backend-webgpu","backend-emscripten"] }
  }
}
```

---

## CMake 用法

```cmake
# 推荐：4 后端 + 常用模块
find_package(unigui REQUIRED)
target_link_libraries(myapp unigui::recommended)

# 最小：core + DX11
target_link_libraries(myapp unigui::core unigui::backend-dx11)

# 自定义组合
target_link_libraries(myapp
    unigui::core unigui::widgets
    unigui::backend-dx11 unigui::backend-vulkan
    unigui::sqlite unigui::dsl unigui::fonts
)

# 运行时切换后端
cfg.backend = BackendType::DX11;
cfg.backend = BackendType::DX12;
cfg.backend = BackendType::Vulkan;
```

---

## 目录结构

```
modules/
├── core/                     → unigui-core
│   ├── app/  theme/  widgets-base/  backend/
├── widgets/                  → unigui-widgets
├── locale/                   → unigui-locale
├── settings/                 → unigui-settings
├── sqlite/                   → unigui-sqlite
├── config/                   → unigui-config
├── network/                  → unigui-network
├── ipc/                      → unigui-ipc
├── plugin/                   → unigui-plugin
├── dsl/                      → unigui-dsl
├── events/                   → unigui-events
├── styling/                  → unigui-styling
├── fonts/                    → unigui-fonts
backends/
├── glfw3/                    → unigui-backend-glfw3
├── dx11/                     → unigui-backend-dx11
├── dx12/                     → unigui-backend-dx12
├── vulkan/                   → unigui-backend-vulkan
├── metal/                    → unigui-backend-metal
├── webgpu/                   → unigui-backend-webgpu
└── emscripten/               → unigui-backend-emscripten
```

---

## 实施步骤 (8 steps)

- [ ] 1. 创建 `modules/` + `backends/` 目录结构 + 每个 CMakeLists.txt
- [ ] 2. 移动源文件到对应模块目录
- [ ] 3. 统一命名空间：`v2::` → `plugin::`, `dsl::`, `events::` 等
- [ ] 4. v1 Settings/Locale 标记 `[[deprecated]]`，内部转发到新模块
- [ ] 5. 更新 `vcpkg.json` features
- [ ] 6. 更新 `find_package(unigui COMPONENTS ...)` 支持
- [ ] 7. 迁移 tests：每个模块独立 test 目录
- [ ] 8. 构建 + 测试：所有配置组合通过

---

## Scope

INCLUDE: 模块拆分、20 模块、命名空间统一、vcpkg features、deprecated 标记、recommended 预设
EXCLUDE: 删除 v1 API、单独 git repo 拆分、运行时动态加载模块
