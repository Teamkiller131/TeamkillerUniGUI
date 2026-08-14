# TeamkillerUniGUI 文档索引

> **4.7.0** · C++23 · Dear ImGui 1.92 · [GitHub](https://github.com/Teamkiller131/TeamkillerUniGUI)

## 入门与总览

| 文档 | 说明 |
|------|------|
| [GETTING_STARTED.md](GETTING_STARTED.md) | 安装、编译、首个程序（即时层 / 控件 / 框架） |
| [ARCHITECTURE.md](ARCHITECTURE.md) | 分层架构总览：后端 → 主题 → 四个 UI 层 → 模块 |
| [EXAMPLES.md](EXAMPLES.md) | 场景食谱（组合、主题、表格、DSL、线程） |

## 框架与各层参考

| 文档 | 说明 |
|------|------|
| [FRAMEWORK.md](FRAMEWORK.md) | **应用框架**：Component + State + Store + Navigator 黄金路径 |
| [REACTIVE.md](REACTIVE.md) | 响应式：Observable / Computed / Bind / State / Store / 控件绑定 |
| [LAYOUT.md](LAYOUT.md) | 布局：SolveFlex / SolveFlexWrap / Layout::FlexRow 等 |
| [IM_API.md](IM_API.md) | 即时层 `unigui::im` 完整参考（约 250 个函数） |
| [DSL.md](DSL.md) | 声明式 DSL（视图语言）参考 |
| [THEMING.md](THEMING.md) | 主题引擎 + CSS 样式 + 热重载 |
| [BACKENDS.md](BACKENDS.md) | 后端抽象与应用主循环（平台 × 渲染器） |

## 控件与模块

| 文档 | 说明 |
|------|------|
| [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md) | **每个 Widget 一条最小示例**（95 条） |
| [WIDGET_API.md](WIDGET_API.md) | **完整控件 API** |
| [API_INDEX.md](API_INDEX.md) | 总索引：Widget + `im` + DSL + 核心 API |
| [MODULES.md](MODULES.md) | CMake 可选模块（EventBus / 插件 / SQLite / IPC / 网络 …） |
| [TRADING.md](TRADING.md) | 交易工具箱 |
| [PRESETS.md](PRESETS.md) | UI 预设脚手架（AppShell / SettingsPage / Dashboard / MasterDetail / LogConsole）— 约 30 行代码搭出一个像样的应用 |
| [ACCESSIBILITY.md](ACCESSIBILITY.md) | 无障碍层 `unigui::a11y` — 焦点追踪、元素树、朗读通告与屏幕阅读器桥 |
| [../INTEGRATION.md](../INTEGRATION.md) | 子模块 + vcpkg 集成 |
| [TROUBLESHOOTING.md](TROUBLESHOOTING.md) | 构建排障 FAQ |

## 推荐阅读顺序

1. [GETTING_STARTED.md](GETTING_STARTED.md) → [ARCHITECTURE.md](ARCHITECTURE.md)
2. 构建应用 → [FRAMEWORK.md](FRAMEWORK.md)（+ [REACTIVE.md](REACTIVE.md) / [LAYOUT.md](LAYOUT.md)）
3. 查单个控件 → [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md) / [API_INDEX.md](API_INDEX.md) → [WIDGET_API.md](WIDGET_API.md)

**「95 widgets」** 指 `widgets/` 下保留式组件；另有约 **250** 个 `unigui::im` 函数、声明式 DSL 与组件框架、响应式与布局层等，见各专题文档。

## 仓库

```bash
git clone https://github.com/Teamkiller131/TeamkillerUniGUI.git
```
