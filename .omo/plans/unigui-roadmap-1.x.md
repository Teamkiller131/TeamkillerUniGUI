# UniGUI 1.x 分版本迭代计划

> v1.0.0 已发布（2026-05-26）。以下为 1.x 系列全部后续迭代。
> 每个版本目标：**~5-10 个 tasks、2-3 天完成、测试不回归、每个版本独立 tag**。

---

## v1.1 — Embedded Fonts（嵌入字体，跨平台一致渲染）

**目标**: 消除系统字体依赖，任何平台渲染效果完全一致。

| # | Task | 说明 |
|---|------|------|
| 1.1.1 | CMake 字体嵌入 pipeline | `cmake/FontEmbed.cmake`，`xxd -i` 将 TTF → C 数组 header |
| 1.1.2 | 嵌入 JetBrains Mono Nerd Font Regular | 文件放入 `fonts/`，~200KB，SIL OFL license |
| 1.1.3 | LoadDefaultFont 改为从内存加载 | `AddFontFromMemoryTTF(jb_mono_data, jb_mono_size, base_size)` |
| 1.1.4 | 移除 LoadDefaultFont 的系统字体回退逻辑 | 简化：始终用嵌入字体 |
| 1.1.5 | CJK 从 memory 合并方案 | 调研：Noto Sans SC 是否适合嵌入（5MB），或保留系统字体 merge 作为备选 |
| 1.1.6 | Thai 字符修复 | 调查 JetBrains Mono 是否含 Thai，若不规则 merge 系统字体 |
| 1.1.7 | README 字体文档 | 自定义字体加载指南 |
| 1.1.8 | CHANGELOG + 版本号 1.1.0 | |

**Test target**: 186+ tests
**Tag**: `v1.1.0`
**Release assets**: 静态 lib + DLL + 嵌入字体的 include

---

## v1.2 — Widget Ecosystem（小部件扩展）

**目标**: 填补常用 UI 控件空白，提升实际开发体验。

| # | Task | 说明 |
|---|------|------|
| 1.2.1 | VirtualList | 虚拟滚动列表，支持 10万+ 条目。只渲染可见行。API: `SetItemCount()`, `SetItemCallback()` |
| 1.2.2 | MultiSelect ComboBox | 多选下拉框。API: `GetSelectedIndices()`, `SetSelectedIndices()` |
| 1.2.3 | PropertyGrid | 属性编辑面板。类似 VS 属性窗口。`AddProperty(name, type, getter, setter)` |
| 1.2.4 | SearchBox | 搜索输入框 + 下拉建议列表。`SetSuggestions()`, `OnSearch` callback |
| 1.2.5 | Toast / SnackBar | 轻量通知弹窗。`Show("message", duration, type)` |
| 1.2.6 | PasswordInput | 密码输入框（显/隐切换、强度指示器）。Extends LineEdit |
| 1.2.7 | Wizard / Stepper | 多步骤向导。`AddStep()`, `Next()`, `Previous()`, `Finish()` |
| 1.2.8 | CHANGELOG + 版本号 1.2.0 | |

**Test target**: ≥195 tests
**Tag**: `v1.2.0`

---

## v1.3 — Platform Hardening（平台加固 + 测试强化）

**目标**: 非 Windows 平台可用性验证 + 测试覆盖率提升。

| # | Task | 说明 |
|---|------|------|
| 1.3.1 | Linux 平台完整构建测试 | GitHub Actions CI 扩展：Ubuntu 22.04/24.04, GCC 14 + Clang 18 |
| 1.3.2 | macOS 平台构建测试 | GitHub Actions CI 扩展：macOS 14, Apple Clang |
| 1.3.3 | 渲染集成测试 | GTest 中添加 RenderDrawData 后检查 GL/DX11 errors 的测试（已在 OpenGL 上做了，扩展到 DX11） |
| 1.3.4 | 字体渲染回归测试 | 截图对比测试：嵌入字体 vs 期望输出（需要 golden image） |
| 1.3.5 | DX12 后端运行时验证 | 在 Windows CI 上实际运行 DX12 路径（目前仅编译通过） |
| 1.3.6 | 内存泄漏检查 | `vcpkg` 集成 `lsan`/`asan`，CI 中运行 |
| 1.3.7 | Widget 模糊测试 | 随机参数 + 随机序列 Render，检查不崩溃 |
| 1.3.8 | CHANGELOG + 版本号 1.3.0 | |

**Test target**: ≥200 tests
**Tag**: `v1.3.0`

---

## v1.4 — Localization & Settings（国际化 + 配置系统）

**目标**: i18n 文件化 + Settings 持久化，让框架具备产品级配置能力。

| # | Task | 说明 |
|---|------|------|
| 1.4.1 | Locale JSON 文件加载 | `Locale::LoadFromFile("zh_CN.json")`，JSON 格式 `{"key":"value"}` |
| 1.4.2 | 内置 en_US / zh_CN 翻译表 | 覆盖常见 widget labels/button texts |
| 1.4.3 | Widget Tr() 集成 | 所有 widget label 支持翻译 key（`SetLabel("##welcome")` → `Tr("##welcome")`） |
| 1.4.4 | Settings 自动持久化 | `Settings::EnableAutoSave("unigui.ini")`，程序退出时自动写入 |
| 1.4.5 | Settings 分层（global/project/user） | 不同优先级合并 |
| 1.4.6 | 最近文件列表（MRU） | `Settings::GetRecentFiles()` / `Settings::AddRecentFile()` |
| 1.4.7 | README i18n 指南 | 如何创建翻译、加载自定义语言文件 |
| 1.4.8 | CHANGELOG + 版本号 1.4.0 | |

**Test target**: ≥210 tests
**Tag**: `v1.4.0`

---

## v1.5 — Developer Experience（开发者体验）

**目标**: 降低上手门槛，完善文档和 API 设计。

| # | Task | 说明 |
|---|------|------|
| 1.5.1 | API 参考文档（Doxygen 发布） | 生成 HTML 文档，托管在 Gitea Pages 或 docs/ 目录 |
| 1.5.2 | Getting Started 教程 | `docs/getting-started.md`，5 分钟从零到运行 |
| 1.5.3 | Widget Gallery 交互增强 | 每个 widget 旁显示实时属性编辑器 |
| 1.5.4 | 错误诊断增强 | 常见错误自动检测 + 建议（"Did you forget to call Init?"） |
| 1.5.5 | CMake 集成简化 | `find_package(unigui)` 一键集成，不需要手动配置 |
| 1.5.6 | Snippet / Template | 常用代码模板（"新建 UniGUI 项目"的 CMake 模板） |
| 1.5.7 | Custom Widget 开发指南 | `docs/custom-widget.md`，如何继承 Widget |
| 1.5.8 | CHANGELOG + 版本号 1.5.0 | |

**Test target**: ≥210 tests
**Tag**: `v1.5.0`

---

## v1.6 — Advanced Features（高级功能）

**目标**: 接近 2.0 的能力层级，引入更多框架级功能。

| # | Task | 说明 |
|---|------|------|
| 1.6.1 | Layout 引擎（HBox / VBox / Grid） | 声明式布局。`Layout::HBox({widget1, widget2})` |
| 1.6.2 | 动画过渡系统 | `Animate::FadeIn(widget, 0.3s)`, `Animate::SlideDown(panel, 0.2s)` |
| 1.6.3 | 快捷键全局管理器 v2 | 改进 ShortcutManager：冲突检测、作用域、可配置 |
| 1.6.4 | 剪贴板 API | `Clipboard::Copy(text)`, `Clipboard::Paste()` |
| 1.6.5 | 文件拖放 | `SetDropCallback([](std::vector<std::string> files){...})` |
| 1.6.6 | 系统托盘图标 | 最小化到托盘，右键菜单 |
| 1.6.7 | 截图/录屏 | `GrabFrame()` 输出 PNG |
| 1.6.8 | CHANGELOG + 版本号 1.6.0 | |

**Test target**: ≥220 tests
**Tag**: `v1.6.0`

---

## v1.7 — Performance & Optimization（性能优化）

**目标**: 大规模场景（1000+ widgets）流畅运行，启动时间优化。

| # | Task | 说明 |
|---|------|------|
| 1.7.1 | Widget 渲染性能 Benchmark | 100/500/1000/5000 widgets 帧时间测试 |
| 1.7.2 | 增量渲染优化 | 跳过不可见 widget 的 ImGui 调用（visibility culling） |
| 1.7.3 | 启动时间优化 | Profile Init → First Frame，减少不必要的初始化 |
| 1.7.4 | 字体 atlas 懒加载 | 首次使用 CJK 字符时才扩展 atlas |
| 1.7.5 | DX11 双缓冲优化 | 减少 GPU flush，合并 draw calls |
| 1.7.6 | Binary size 优化 | 检查编译选项（/Os, LTCG），对比各版本体积变化 |
| 1.7.7 | Memory pool（可选） | Widget 级内存池，减少碎片 |
| 1.7.8 | CHANGELOG + 版本号 1.7.0 | |

**Test target**: ≥230 tests
**Tag**: `v1.7.0`

---

## v1.8 — Pre-2.0 Stabilization（发布前稳定化）

**目标**: 为 2.0 做准备，修复所有 known issues，完成文档。

| # | Task | 说明 |
|---|------|------|
| 1.8.1 | 全平台全后端兼容性矩阵测试 | Windows/Linux/macOS × GL3/Vulkan/DX11/DX12/Metal |
| 1.8.2 | Deprecation 标记 | 标记即将移除的 API（`[[deprecated]]`） |
| 1.8.3 | Migration Guide（1.x → 2.0） | 列出所有 breaking changes |
| 1.8.4 | 安全审计 | 输入验证、buffer overflow、null deref 检查 |
| 1.8.5 | API 冻结 | 锁定 2.0 的公开 API，不再添加新的 public 方法 |
| 1.8.6 | 最终文档审校 | README、CHANGELOG、Doxygen、Tutorial 全部过一遍 |
| 1.8.7 | 1.x 系列总结博客 | 技术博客：架构演进、踩坑记录 |
| 1.8.8 | CHANGELOG + 版本号 1.8.0（1.x 收尾版） | |

**Test target**: ≥240 tests
**Tag**: `v1.8.0`（1.x 系列最后一个版本）

---

## 📊 版本矩阵总览

| 版本 | 主题 | Tasks | 测试目标 | 预计工时 | 依赖 |
|------|------|-------|---------|---------|------|
| v1.1 | 嵌入字体 | 8 | 186+ | ~3h | 无 |
| v1.2 | Widget 扩展 | 8 | 195+ | ~6h | 无 |
| v1.3 | 平台加固 | 8 | 200+ | ~8h | 需 Linux/macOS 设备 |
| v1.4 | i18n+Settings | 8 | 210+ | ~5h | v1.1 (字体) |
| v1.5 | 开发体验 | 8 | 210+ | ~4h | v1.2 (widgets) |
| v1.6 | 高级功能 | 8 | 220+ | ~8h | v1.4 (i18n) |
| v1.7 | 性能优化 | 8 | 230+ | ~6h | v1.6 (稳定) |
| v1.8 | 2.0 准备 | 8 | 240+ | ~4h | 全部前序 |

**总计**: 8 个迭代版本，64 个 tasks，1.x 系列最终达到 240+ 测试。

---

## 建议路线

```
v1.0 ──→ v1.1 ──→ v1.2 ──→ v1.4 ──→ v1.3 ──→ v1.5 ──→ v1.6 ──→ v1.7 ──→ v1.8 ──→ v2.0
          字体     组件      i18n     平台     体验     高级     性能     收尾     NEXT
```

v1.3（平台加固）可与其他版本并行进行（只需要 CI 环境搭建，不阻塞功能开发）。

---

## 2.0 预告（不做具体规划）

- 插件系统（`.dll/.so` 动态加载）
- 网络功能（HTTP client、WebSocket）
- 数据库集成（SQLite binding）
- 完整 Metal/WebGPU 后端
- 声明式 UI DSL
- 远程渲染（client-server 模式）
