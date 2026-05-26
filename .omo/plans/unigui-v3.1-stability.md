# UniGUI v3.1 — Stability & Polish

## TL;DR

> **Quick Summary**: 巩固 v3.0 成果 — 修复 5 个 pre-existing 测试失败、CSS @media 完整实现、
> CHANGELOG 更新、多平台预研。纯稳定版，不加新功能。
>
> **Deliverables**:
> - 5 个 pre-existing 测试修复
> - CSS @media 完整 evaluate（min-width/max-width/prefers-color-scheme）
> - EventBus race condition 修复
> - CHANGELOG.md v3.0 entry
> - v4 跨平台路线图
>
> **Estimated Effort**: Small (~8 tasks, 3 waves)
> **Parallel Execution**: YES — 3 waves, 2-4 tasks per wave

## Context

### Current State (v3.0.0)

| Metric | Value |
|--------|-------|
| Build targets | 483 (recommended), 223 (minimal), 507 (full) |
| Tests | 244 total, 239 pass, 5 fail |
| Widgets | 63 |
| Themes | 13 presets |
| CSS properties | 70 |
| Easing curves | 10 |
| Examples | 9 |

### Pre-existing Failures (5)

- `AppTest.Init_WithoutDisplay_ReturnsFalse` — 无头模式下 GLFW 窗口创建失败，`Init()` 返回 false；测试期望 `true`
- `BusTest.Subscribe_Publish_Delivers` — EventBus worker thread 竞争：Publish 在 Subscribe 完成前执行
- `BusTest.Wildcard_Matches` — 同上
- `BusTest.Unsubscribe_StopsDelivery` — 同上
- `BusTest.SubscribeAll_Wildcard` — 同上

### Test Infrastructure

- **Framework**: GTest + gtest_discover_tests
- **Strategy**: Tests-after (fix existing, add regression tests)

---

## Work Objectives

### Core Objective
将测试通过率从 97.9% 提升到 100%，CSS @media 从 "仅检测" 到 "完整评估"，写出 v3.0 CHANGELOG。

### Concrete Deliverables
- 5 个测试全部通过
- `tests/regression/eventbus_race_test.cc` — 回归测试
- `src/v2/style_engine.cc` — @media evaluate 实现
- `CHANGELOG.md` — v3.0.0 条目
- `.omo/plans/unigui-v4-cross-platform.md` — v4 路线图

---

## Execution Strategy

```
Wave 1 (Test Fixes — MAX PARALLEL):
├── Task 1: AppTest headless 修复 [quick]
├── Task 2: EventBus race 修复 [unspecified-high]
└── Task 3: EventBus 回归测试 [quick]

Wave 2 (CSS + Docs):
├── Task 4: @media evaluate 完整实现 [quick]
├── Task 5: CHANGELOG.md v3.0 entry [writing]
└── Task 6: README 更新 (test count, widget count)

Wave 3 (v4 Planning):
└── Task 7: v4 跨平台路线图 (.omo/plans/)

Final Verification:
├── F1: 3-preset build verification
├── F2: Test suite 100% pass
└── F3: Tag v3.1.0 + push
```

## Success Criteria

- [ ] `ctest --preset windows-msvc-debug` → 244/244 pass
- [ ] CSS @media `(min-width: 800px)` correctly evaluates
- [ ] CHANGELOG.md has v3.0.0 section
- [ ] 3 preset builds pass
