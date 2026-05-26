# UniGUI — Auto-Wrap Default + Multi-Language Demo

## TL;DR

> **Quick Summary**: Panel auto-wrap enabled by default; hello_unigui showcases CJK + multi-language text to verify font rendering.
> 
> **Deliverables**: All text wraps automatically; demo renders Chinese/Japanese/Korean/Arabic/Emoji correctly.
> **Estimated Effort**: Quick (~3 tasks)

---

## TODOs

- [ ] 1. Panel 默认开启 auto-wrap
  **What**: `src/widgets/panel.cc` — `Panel::Panel()` 构造函数中 `wrap_ = true`（原为 `false`）。
  同时 `include/unigui/widgets/panel.h` 中 `bool wrap_ = false;` 改为 `bool wrap_ = true;`。
  **QA**: 编译通过，`Panel` 创建后 `wrap_` 为 true。

- [ ] 2. hello_unigui 多国语言 demo 面板
  **What**: `examples/hello_unigui/main.cc` — 在 `first` 初始化块中添加一个新 Panel，标题 "i18n Test"，启用 `SetWrapEnabled(true)`，内容 callback 中用 `ImGui::TextUnformatted` 展示多国文字：
  - 中文："你好，世界！这是一个 Dear ImGui C++ 封装库。"
  - 日文："こんにちは世界！これはImGuiのラッパーです。"
  - 韩文："안녕하세요 세계! 이것은 ImGui 래퍼입니다."
  - 阿拉伯文："مرحبا بالعالم! هذا هو مغلف ImGui."
  - Emoji："🎉🚀✨ 支持 Emoji 和特殊字符"
  **QA**: 编译通过，运行 `--frames 2` 不崩溃。

- [ ] 3. 构建 + 测试验证
  **What**: `cmake --build --preset windows-msvc-debug` + `ctest --preset windows-msvc-debug`。186/186 通过。
  **QA**: 测试全部通过。

---

## Scope
INCLUDE: Panel wrap 默认值, 多国语言 demo
EXCLUDE: 其他 widget 的 wrap 默认值, 真正的 i18n 翻译系统
