# UniGUI 日志系统（spdlog 集成）

## TL;DR

> **Quick Summary**: 集成 spdlog v1.17.0，在所有关键路径添加结构化日志，解决问题定位。
> 
> **Deliverables**: `core/log.h`, `core/log.cc`, 6 个模块的日志调用
> **Estimated Effort**: Quick (~8 tasks, 全部并行)
> **Test Target**: 185 测试不回归

---

## Context

### 原始问题
`hello_unigui.exe` 启动后显示黑窗口但没有 ImGui 内容。需要日志来定位是 Init 失败、渲染失败、还是其他原因。

### 技术方案
spdlog v1.17.0（vcpkg），console + 文件双 sink，debug 级别。

---

## Work Objectives

### Core Objective
在每个关键路径点添加日志调用，使开发者能通过 `unigui.log` 追踪完整执行流程。

### Must Have
- spdlog 依赖
- 日志初始化
- App Bootstrap 完整日志
- GLFW 平台完整日志
- OpenGL3 渲染器日志
- 工厂和主题日志

### Must NOT Have
- 修改任何业务逻辑
- 改变现有行为
- 日志影响性能（trace 级别默认关闭，debug 默认开启）

---

## TODOs

- [ ] 1. 添加 spdlog 依赖 + CMake 配置

  **What to do**:
  - `vcpkg.json`: 添加 `{"name": "spdlog"}`
  - `src/CMakeLists.txt`: `find_package(spdlog CONFIG REQUIRED)`, `target_link_libraries(unigui PUBLIC spdlog::spdlog)`
  - `unigui.h`: `#include <unigui/core/log.h>`

  **QA**: `cmake --preset windows-msvc-debug` 配置通过，`Select-String "spdlog"` 找到链接

- [ ] 2. 创建日志核心 (`include/unigui/core/log.h` + `src/core/log.cc`)

  **What to do**:
  - `log.h`: `InitLogging(level)` 函数声明 + `UNIGUI_LOG_*` 宏
  - `log.cc`: console sink (彩色) + file sink (`unigui.log`)
  - `GetLogger("unigui")` 工厂函数

  **QA**: 编译通过，`#include <unigui/core/log.h>` 无报错

- [ ] 3. App Bootstrap 日志 (`src/app/app.cc`)

  **What to do**:
  - `Init()` 入口: `UNIGUI_LOG_INFO("Init backend={}", ...)` 
  - `Init()` 平台失败: `UNIGUI_LOG_ERROR("Platform init failed")`
  - `Init()` 渲染器失败: `UNIGUI_LOG_ERROR("Renderer init failed")`
  - `Init()` 成功: `UNIGUI_LOG_INFO("Init OK — {}x{} '{}'", w, h, title)`
  - `Init()`: 初始化日志系统 `InitLogging("debug")`
  - `Shutdown()`: `UNIGUI_LOG_INFO("Shutdown")`
  - `NewFrame()`: `UNIGUI_LOG_TRACE("NewFrame #{}", frame)`
  - `Render()`: `UNIGUI_LOG_TRACE("Render: {} CmdLists, {} Vtx, {} Idx")`

  **QA**: 运行 `hello_unigui --frames 3`，`unigui.log` 包含 Init/Shutdown 日志

- [ ] 4. GLFW 平台日志 (`src/backend/glfw_platform.cc`)

  **What to do**:
  - `Init()`: glfwInit 结果, 窗口创建 `{}x{} "{}"`, ImGui GLFW init 结果
  - `Init()` 失败: WARN 级别 + 原因
  - `NewFrame()`: TRACE
  - `PollEvents()`: TRACE
  - `SwapBuffers()`: TRACE
  - `Shutdown()`: INFO "GLFW shutdown"

  **QA**: `unigui.log` 包含窗口创建日志

- [ ] 5. OpenGL3 渲染器日志 (`src/backend/opengl3_renderer.cc`)

  **What to do**:
  - `Init()`: gladLoadGL 结果, ImGui_ImplOpenGL3_Init 结果, GL 版本
  - `Init()` 失败: ERROR
  - `RenderDrawData()`: draw data valid/invalid, CmdLists count
  - `Shutdown()`: INFO

  **QA**: `unigui.log` 包含 OpenGL 初始化日志

- [ ] 6. 工厂 + 主题日志

  **What to do**:
  - `backend_factory.h`: `CreateBackend()` 打印选择的 backend 类型 (DEBUG)
  - `theme.cc`: `ApplyTheme()` 打印 preset 名称 (DEBUG)
  - `window.cc`: `Render()` 打印 Begin/End + panel count (TRACE)

  **QA**: `unigui.log` 包含 backend type 和 theme preset

- [ ] 7. 源码注册 + 编译验证

  **What to do**:
  - `src/CMakeLists.txt` 添加 `core/log.cc` 到 `target_sources`
  - 编译全部 252 目标
  - 运行 185 测试

  **QA**: 252/252 编译, 185/185 测试通过

- [ ] 8. 运行验证 + 日志检查

  **What to do**:
  - `hello_unigui --frames 3`
  - `Select-String "unigui.log" -Pattern "(Init|Render|Shutdown)"`
  - 验证日志包含完整生命周期

  **QA**: 日志文件包含 ≥20 条日志记录，覆盖 Init → NewFrame → Render → Shutdown

---

## Scope

### INCLUDE
- spdlog 依赖
- 日志核心模块
- App/GLFW/OpenGL3/Factory/Theme/Window 日志点

### EXCLUDE
- 修改任何业务逻辑
- 性能优化
- 日志轮转/压缩
