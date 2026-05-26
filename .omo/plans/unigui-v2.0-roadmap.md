# UniGUI v2.0 — 架构改造计划 (Revised)

> **所有 v1.x API 保留向下兼容。v2.0 新增，不删除。**

---

## 🔧 技术栈确认

| 模块 | 技术选型 | vcpkg | 理由 |
|------|---------|-------|------|
| Config | **TOML** (`cpptoml`) + **JSON** (`nlohmann/json`) | ✅ | TOML 人工可读，JSON 机器交换 |
| 数据库 | **手写 SQLite3 wrapper** | ✅ (sqlite3 内置) | 零 ORM 依赖，薄封装 |
| IPC | **ZeroMQ** (`cppzmq`) + **Shared Memory** (OS原生) | ✅ | ZMQ 跨平台抽象，SHM 高性能数据通道 |
| 网络 | `cpp-httplib` (HTTP) + `ixwebsocket` (WS) | ✅ | 均 header-only |
| CSS | **手写微型解析器** | 0 | ~400 行，CSS 极小子集 |
| 字体 | **手写 FontManager** | 0 | ImGui 字体 API 封装 |
| 插件 | **OS 原生 dlopen/LoadLibrary** | 0 | 零依赖 |
| DSL | **C++ 表达式模板 + Builder** | 0 | 编译期类型安全 |

**vcpkg.json 新增依赖**:
```json
"cpptoml", "nlohmann-json", "cppzmq", "cpp-httplib", "ixwebsocket"
```

---

## 🏗️ 施工顺序 (M1 → M8) + 每个里程碑内部按优先级排列

### M1: Plugin System + M2: DSL (并行施工，v2.0.0-alpha)

**理由**: 插件系统是架构地基，DSL 是最能体现 v2 价值的特性。并行施工无依赖。

**M1 任务清单** (按优先级):

- [ ] 1.1 PluginInterface — 纯虚基类 `IPlugin`: `Init()/Shutdown()/GetVersion()/GetName()`
- [ ] 1.2 PluginManager — `Load(path)`, `Unload(name)`, `Reload(name)`, `List()`
- [ ] 1.3 OS 原生 dlopen/LoadLibrary 封装 (无第三方依赖)
- [ ] 1.4 Plugin sandbox — 独立 OS 进程 + IPC 隔离
- [ ] 1.5 Example plugin: `ExtraWidgets` — 包含 1 个自定义 widget
- [ ] 1.6 Plugin hot-reload — 文件变化→自动重载 (Win32/ inotify)
- [ ] 1.7 单元测试 (插件加载→卸载→重载)
- [ ] 1.8 文档 `docs/plugins.md`

**M2 任务清单** (按优先级):

- [ ] 2.1 DSL AST 节点类型: `WidgetNode`, `ContainerNode(VBox/HBox/Grid)`, `IfNode`, `ForNode`
- [ ] 2.2 Builder 函数: `Window()`, `VBox()`, `HBox()`, `Grid()`, `Button()`, `Label()`, `If()`, `For()`
- [ ] 2.3 DSL→ImGui 编译器: Walk AST, 生成 ImGui::Begin/End/Button 调用
- [ ] 2.4 数据绑定 `Bind(model.field, widget)` — 单向绑定 v1, 双向 v2
- [ ] 2.5 `hello_unigui` 重写为 DSL 版本 + 旧版保留为 `hello_unigui_v1`
- [ ] 2.6 单元测试 (AST 构建→编译→验证 ImGui 调用顺序)
- [ ] 2.7 文档 `docs/dsl.md`

---

### M3: EventBus (v2.1.0)

**理由**: 解耦 M1 插件间通信和 M2 Widget 间通信的最佳方案。

- [ ] 3.1 EventBus 核心: `Publish<T>(topic, event)`, `Subscribe<T>(topic, handler)`, `Unsubscribe(id)`
- [ ] 3.2 Topic hierarchy: `"window.*"`, `"window.close"` 通配符
- [ ] 3.3 异步事件: `PublishAsync()` → 线程池
- [ ] 3.4 内置 topic: `"app.init"`, `"window.close"`, `"theme.changed"`, `"plugin.loaded"`
- [ ] 3.5 单元测试 (发布→订阅→取消订阅→通配符)
- [ ] 3.6 文档 `docs/eventbus.md`

---

### M4: CSS-like Styling (v2.2.0)

**理由**: M1 M2 产生大量 widget，需要统一外观管理。

- [ ] 4.1 CSS 词法分析器: tokenizer (选择器/属性/值)
- [ ] 4.2 CSS 解析器: AST→StyleRule
- [ ] 4.3 StyleEngine: `LoadFile("theme.css")`, `Apply(widget)`, `ApplyAll()`
- [ ] 4.4 选择器: 类型 (`Window`), 类 (`.primary`), ID (`#submit`), hover/focus 伪类
- [ ] 4.5 属性映射: `bg→ImGuiCol_`, `rounding→ImGuiStyleVar_`, `font→FontManager`
- [ ] 4.6 热重载: 文件变化→自动 `LoadFile`
- [ ] 4.7 内置 Dark/Light CSS 主题文件
- [ ] 4.8 单元测试 (解析→应用→验证 ImGuiStyle 值)
- [ ] 4.9 文档 `docs/styling.md`

---

### M5: FontManager (v2.3.0)

**理由**: M4 CSS 需要 `font: "code"` 引用，字体管理是 CSS 的前提。

- [ ] 5.1 FontManager 单例: `Load(name,path,size)`, `Get(name)`, `Unload(name)`
- [ ] 5.2 Fallback chain: `SetFallback(name, fallbackName)`
- [ ] 5.3 Hot-reload: 字体文件变化自动重载 (Win32 `ReadDirectoryChangesW` / inotify)
- [ ] 5.4 Variable font: `LoadVF(name,path,size,{{"wght",700}})`
- [ ] 5.5 单元测试 (加载→获取→卸载→fallback)

---

### M6: 配置系统升级 (v2.4.0)

**理由**: 旧 INI 格式不区分类型。TOML 为人类编写，JSON 为程序交换。

- [ ] 6.1 TOML 支持: `Config::LoadTOML("app.toml")`, `Config::SaveTOML(path)`
- [ ] 6.2 JSON 支持: `Config::LoadJSON("app.json")`, `Config::SaveJSON(path)`
- [ ] 6.3 向后兼容: `Config::LoadINI(path)` 保留，内部自动迁移到 TOML
- [ ] 6.4 类型安全访问: `Config::Get<int>("window.width")`, `Config::Get<std::string>("app.title")`
- [ ] 6.5 层级合并: 系统默认 → 用户配置 → 项目配置 → 命令行覆盖
- [ ] 6.6 单元测试 (TOML 解析→JSON 导出→INI 导入→层级合并)
- [ ] 6.7 文档 `docs/config.md`

---

### M7: SQLite (v2.5.0)

**理由**: 手写 wrapper，零 ORM 依赖，薄封装。

- [ ] 7.1 Database 类: `Open(path)`, `Close()`, `Execute(sql, params...)`
- [ ] 7.2 Query: `Query(sql, params...)`, `Each([](Row&){})`, `First()`
- [ ] 7.3 Transaction RAII: `Transaction txn(db);` 自动 commit/rollback
- [ ] 7.4 Migration: `Migrate(version, sql)` — 版本化 schema
- [ ] 7.5 参数绑定: `?` 占位符 + `int/string/double/blob` 类型
- [ ] 7.6 单元测试 (建表→插入→查询→迁移→事务回滚)
- [ ] 7.7 文档 `docs/database.md`

---

### M8: IPC (v2.6.0)

- [ ] 8.1 ZMQ 通道: `IPCServer(addr)`, `IPCClient(addr)`, `Send(msg)`, `OnRecv(cb)`
- [ ] 8.2 SHM 通道: `SharedMemory(name, size)`, `Write(offset, data)`, `Read(offset, size)`
- [ ] 8.3 统一 API: 上层不关心底层是 ZMQ 还是 SHM
- [ ] 8.4 Plugin sandbox: 插件进程通过 IPC 与主机通信
- [ ] 8.5 单元测试 (ZMQ server→client→SHM read/write)

---

### M9: Network (v2.7.0)

- [ ] 9.1 HTTP: `HttpClient::Get(url)`, `Post(url, body, headers)`, 异步回调
- [ ] 9.2 WebSocket: `WebSocket::Connect(url)`, `OnMessage(cb)`, `Send(msg)`
- [ ] 9.3 同步 + 异步双模式
- [ ] 9.4 超时 + 重试
- [ ] 9.5 单元测试 (HTTP GET→POST→WebSocket echo)

---

### M10: v2.8.0 发布 (收尾)

- [ ] 10.1 全模块集成测试 (≥300 tests)
- [ ] 10.2 Migration Guide 1.x → 2.0
- [ ] 10.3 API reference (Doxygen + 自动生成)
- [ ] 10.4 Release artifacts (static/DLL + plugin SDK)
- [ ] 10.5 CHANGELOG v2.0.0

---

## 📊 新增依赖 (vcpkg.json v2.0)

```json
{
  "name": "cpptoml",
  "name": "nlohmann-json", 
  "name": "cppzmq",
  "name": "cpp-httplib",
  "name": "ixwebsocket"
}
```

**总增量** ~3MB (均 header-only 或轻量)

---

## 🏷️ 版本序列

| 版本 | 里程碑 | 核心 | 测试 |
|------|--------|------|------|
| v2.0.0-alpha | M1+M2 | Plugin + DSL | 202→220 |
| v2.1.0 | M3 | EventBus | 220→230 |
| v2.2.0 | M4 | CSS Styling | 230→240 |
| v2.3.0 | M5 | FontManager | 240→250 |
| v2.4.0 | M6 | TOML/JSON Config | 250→260 |
| v2.5.0 | M7 | SQLite | 260→270 |
| v2.6.0 | M8 | IPC (ZMQ+SHM) | 270→280 |
| v2.7.0 | M9 | Network | 280→290 |
| v2.8.0 | M10 | Release | 290→300 |

---

## 🚫 v2 工程标准

1. **每个 milestone PR 独立** — 单 milestone 合并到 `master`, tag 独立
2. **测试先行** — 每个新模块 `tests/module/` 下至少 5 个测试
3. **无 breaking 1.x API** — 现有 `unigui::` namespace 下所有公开 API 不删不改
4. **新 API 放 `namespace unigui::v2`** — 避免符号冲突
5. **Doxygen 每个公开函数** — `/// @brief` 必须
6. **spdlog 分层** — DEBUG 开关默认关 (运行时 `SetLogLevel`)
7. **内存**: 无 `new/delete` 裸指针, 全程 `std::unique_ptr`
8. **线程安全**: 所有全局状态用 `std::mutex` 保护
9. **CI**: 每个 milestone 合并前 `cmake --build + ctest` 全绿
10. **docs/**: 每个模块有独立 `.md` 文档
