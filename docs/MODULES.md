# CMake Modules & Optional Sub-systems

TeamkillerUniGUI (v4.9.0) is built as a **single library target** (`unigui`) whose
feature surface is sliced into **modules**. Each module is a `UNIGUI_MODULE_*`
CMake option that toggles a group of source files in `src/CMakeLists.txt` and,
where relevant, pulls in extra [vcpkg](https://vcpkg.io) dependencies. Modules
that need third-party libraries (SQLite, TOML/JSON, ZeroMQ, HTTP/WebSocket) are
**off by default** so the base library stays lean; the always-available layers
(widgets, DSL, styling, fonts, events, plugins) are **on by default**.

This document is the canonical reference for every optional module: what it does,
the CMake option that enables it, the vcpkg packages it requires, the **exact**
public API (taken verbatim from the headers under `include/unigui/<module>/`),
and a runnable usage example.

> All options must be set **before** `add_subdirectory(TeamkillerUniGUI)` (or
> baked into a CMake preset). Toggling a module also defines a
> `UNIGUI_HAS_<MODULE>` compile definition on the `unigui` target, which the
> umbrella header `<unigui/unigui.h>` uses to gate the dependency-heavy includes.

---

## Module summary

| CMake option | Default | Namespace | Purpose | Extra vcpkg deps |
|---|---|---|---|---|
| `UNIGUI_MODULE_WIDGETS` | **ON** | `unigui::` | 92 retained-mode widgets (tables, trees, charts, dialogs, …) | — |
| `UNIGUI_MODULE_DSL` | **ON** | `unigui::dsl` | Declarative UI builders + component framework | — |
| `UNIGUI_MODULE_STYLING` | **ON** | `unigui::styling` | CSS-like style engine | — |
| `UNIGUI_MODULE_FONTS` | **ON** | `unigui::fonts` | Font manager, fallback chains, emoji, gradient text | — |
| `UNIGUI_MODULE_EVENTS` | **ON** | `unigui::events` | Thread-safe publish/subscribe `Bus` | — |
| `UNIGUI_MODULE_PLUGIN` | **ON** | `unigui::plugin` | Dynamic (DLL/.so) plugin loader + lifecycle | — |
| `UNIGUI_MODULE_SQLITE` | OFF | `unigui::sqlite` | SQLite database wrapper (params, transactions, migrations) | `sqlite3` |
| `UNIGUI_MODULE_CONFIG` | OFF | `unigui::config` | Unified TOML / JSON / INI config store | `cpptoml`, `nlohmann-json` |
| `UNIGUI_MODULE_IPC` | OFF | `unigui::ipc` | Inter-process messaging (ZeroMQ) + shared memory | `zeromq` |
| `UNIGUI_MODULE_NETWORK` | OFF | `unigui::network` | HTTP client + WebSocket client | `cpp-httplib`, `ixwebsocket` |
| `UNIGUI_MODULE_TRADING` | OFF | `unigui::trading` | Trading-client toolkit (models + widgets) | — |

The base dependencies pulled in unconditionally are `imgui`, `glfw3`, `glad`,
`Freetype`, `implot`, and `spdlog` (see `src/CMakeLists.txt`). When you consume
UniGUI as a **submodule**, the parent project should list these in its own
`vcpkg.json`; when you build the standalone repo, the root `vcpkg.json` declares
them, and the optional modules map to vcpkg **features** (`sqlite`, `config`,
`ipc`, `network`).

### Enabling optional modules

Via the standalone `vcpkg.json` features when configuring the repo:

```bash
# Linux/macOS — request the optional features in the manifest install
cmake -B build -S . \
  -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
  -DVCPKG_MANIFEST_FEATURES="sqlite;config;network" \
  -DUNIGUI_MODULE_SQLITE=ON \
  -DUNIGUI_MODULE_CONFIG=ON \
  -DUNIGUI_MODULE_NETWORK=ON \
  -G Ninja
```

Or, as a submodule, set the option before `add_subdirectory`:

```cmake
set(UNIGUI_MODULE_SQLITE ON)
set(UNIGUI_MODULE_NETWORK ON)
add_subdirectory(third_party/TeamkillerUniGUI)
target_link_libraries(my_app PRIVATE unigui::unigui)
```

### Compile-time feature detection

When a module is enabled, the `unigui` target carries a public
`UNIGUI_HAS_<MODULE>` definition. The umbrella header includes the
dependency-heavy modules behind these guards, so you can write portable code:

```cpp
#include <unigui/unigui.h>

#ifdef UNIGUI_HAS_SQLITE
    unigui::sqlite::Database db;
    db.Open("app.db");
#endif
```

The header-only-friendly modules (`events`, `plugin`, `fonts`, `dsl`, `styling`)
are included by `<unigui/unigui.h>` unconditionally — their declarations are safe
even when the corresponding `.cc` is not compiled — whereas `config`, `network`,
`sqlite`, `ipc`, and `trading` are gated by their `UNIGUI_HAS_*` macros because
their headers transitively include third-party headers (`cpptoml.h`,
`httplib.h`, `sqlite3.h`, etc.). If you include a module header directly without
enabling its module, you will get unresolved symbols at link time.

---

## EventBus — `UNIGUI_MODULE_EVENTS` (default ON)

**Header:** `<unigui/events/eventbus.h>` · **Namespace:** `unigui::events` ·
**Source:** `src/events/eventbus.cc` · **vcpkg deps:** none

A thread-safe, process-local publish/subscribe message bus. It supports
topic-string subscriptions with `*` wildcards, synchronous and asynchronous
(worker-thread-backed) publishing, RAII-scoped subscriptions, and a catch-all
"subscribe to everything" hook for logging/debugging. Events are carried as
`std::any`, so any copyable type can be published.

### API

`Bus` is a singleton accessed via `Bus::Instance()`.

```cpp
namespace unigui::events {

class Bus {
public:
    using Handler = std::function<void(const std::any& event)>;
    using SubID   = uint64_t;

    static Bus& Instance();

    // Subscribe to a topic. Wildcards: "window.*" matches "window.close".
    SubID Subscribe(const std::string& topic, Handler handler);

    // RAII auto-unsubscribe; returns a move-only handle.
    Subscription SubscribeScoped(const std::string& topic, Handler handler);

    void Unsubscribe(SubID id);

    // Deliver synchronously to all matching subscribers.
    void Publish(const std::string& topic, const std::any& event);

    // Enqueue for delivery on the bus worker thread.
    void PublishAsync(const std::string& topic, const std::any& event);

    // Catch-all subscription (every topic) for debug/logging.
    SubID SubscribeAll(Handler handler);

    void Shutdown();   // stop the async worker thread
};

// RAII subscription handle — unsubscribes on destruction; move-only.
class Subscription {
public:
    Subscription() noexcept = default;
    Subscription(Bus& bus, Bus::SubID id) noexcept;
    ~Subscription();

    Subscription(const Subscription&) = delete;
    Subscription& operator=(const Subscription&) = delete;

    Subscription(Subscription&&) noexcept;
    Subscription& operator=(Subscription&&) noexcept;

    bool        Valid() const noexcept;   // true while still subscribed
    void        Unsubscribe();            // manual; no-op if already gone
    Bus::SubID  GetID() const noexcept;
};

} // namespace unigui::events
```

Notes:
- **Wildcards** are matched per `MatchTopic`: a pattern like `"window.*"` matches
  `"window.close"`, `"window.resize"`, etc.
- **`Subscribe`** returns a `SubID` you must keep if you want to `Unsubscribe`
  manually. Prefer **`SubscribeScoped`** for member-lifetime subscriptions — the
  returned `Subscription` unsubscribes automatically when it goes out of scope.
- **`PublishAsync`** queues the event for a background worker thread; handlers run
  off the publishing thread, so they must be thread-safe.
- The singleton's destructor calls `Shutdown()`, joining the worker thread.

### Example

```cpp
#include <unigui/events/eventbus.h>
#include <any>
#include <string>
#include <iostream>

struct WindowClosed { std::string name; };

void wire_events() {
    auto& bus = unigui::events::Bus::Instance();

    // Wildcard subscription, retained for the program lifetime.
    bus.Subscribe("window.*", [](const std::any& ev) {
        if (ev.type() == typeid(WindowClosed))
            std::cout << "closed: " << std::any_cast<WindowClosed>(ev).name << "\n";
    });

    // RAII subscription — drops automatically when `sub` is destroyed.
    unigui::events::Subscription sub =
        bus.SubscribeScoped("app.tick", [](const std::any&) { /* per-tick work */ });

    // Synchronous dispatch.
    bus.Publish("window.close", WindowClosed{"settings"});

    // Asynchronous dispatch (runs on the bus worker thread).
    bus.PublishAsync("app.tick", 0);
}
```

---

## Plugin system — `UNIGUI_MODULE_PLUGIN` (default ON)

**Headers:** `<unigui/plugin/plugin_interface.h>`,
`<unigui/plugin/plugin_manager.h>` · **Namespace:** `unigui::plugin` ·
**Source:** `src/plugin/plugin_manager.cc` · **vcpkg deps:** none

A dynamic plugin loader. A plugin is a shared library (DLL on Windows, `.so`/
`.dylib` elsewhere) that exports a `CreatePlugin`/`DestroyPlugin` factory pair
and implements the `IPlugin` interface. The `Manager` singleton loads, lists,
hot-reloads, and unloads plugins, owning each instance via a custom-deleter
`unique_ptr`. Statically-linked (built-in) plugins can also be registered. The
plugin example/target is only built on Windows (`examples/plugin_example`).

### API

```cpp
namespace unigui::plugin {

struct PluginInfo {
    std::string name;
    std::string version;
    std::string author;
    std::string description;
};

// Implemented by every plugin DLL.
class IPlugin {
public:
    virtual ~IPlugin() = default;
    virtual PluginInfo GetInfo() const = 0;
    virtual bool Init() = 0;
    virtual void Shutdown() = 0;
    virtual void Update(float dt) {}   // optional per-frame update
    virtual void Render() {}           // optional per-frame render
};

// Factory signatures each DLL must export.
using CreatePluginFn  = IPlugin* (*)();
using DestroyPluginFn = void (*)(IPlugin*);

struct LoadedPlugin {
    std::string path, name;
    void* handle = nullptr;
    IPlugin* raw = nullptr;
    std::unique_ptr<IPlugin, std::function<void(IPlugin*)>> ptr; // RAII cleanup
};

class Manager {
public:
    static Manager& Instance();

    IPlugin* Load(const std::string& path);      // nullptr on failure
    bool     Unload(const std::string& name);    // true if found
    IPlugin* Reload(const std::string& name);    // unload + load
    std::vector<std::string> List() const;
    IPlugin* Get(const std::string& name) const; // nullptr if not found
    IPlugin* Register(IPlugin* plugin);          // adopt a built-in plugin

    void SetOnReload(std::function<void(const std::string& name)> cb);
    void Shutdown();                             // unload all
};

} // namespace unigui::plugin
```

Notes:
- **`Load`** opens the library, resolves the `CreatePlugin`/`DestroyPlugin`
  symbols, constructs the instance, and calls its `Init()`. It returns `nullptr`
  if the library can't be opened or the symbols are missing.
- **`Reload`** unloads then re-loads by name — useful during development when you
  rebuild a plugin DLL. Use `SetOnReload` to react (e.g. re-wire callbacks).
- **`Register`** lets you mix statically-compiled plugins into the same manager;
  the `Manager` takes ownership.
- **`Shutdown`** calls each plugin's `Shutdown()` and frees every library handle.

### Authoring a plugin DLL

The shipped example (`examples/plugin_example/plugin.cc`) exports the factory
pair with `extern "C"` plus the platform export marker (`__declspec(dllexport)`
on Windows; on Linux/macOS use `__attribute__((visibility("default")))` or
default visibility):

```cpp
// my_plugin.cc — compiled to a shared library
#include <unigui/plugin/plugin_interface.h>

class MyPlugin : public unigui::plugin::IPlugin {
public:
    unigui::plugin::PluginInfo GetInfo() const override {
        return { "my_plugin", "1.0.0", "me", "A sample plugin" };
    }
    bool Init() override   { /* one-time setup */ return true; }
    void Shutdown() override { /* teardown */ }
    void Update(float dt) override { /* per-frame logic */ }
    void Render() override { /* draw ImGui here */ }
};

extern "C" {
    __declspec(dllexport) unigui::plugin::IPlugin* CreatePlugin() { return new MyPlugin(); }
    __declspec(dllexport) void DestroyPlugin(unigui::plugin::IPlugin* p) { delete p; }
    __declspec(dllexport) std::int32_t PluginInterfaceVersion() { return unigui::plugin::kPluginInterfaceVersion; }
}
```

### The ABI version gate

The plugin interface is C++ (a vtable contract), so it is compiler-ABI-sensitive:
a plugin DLL built by a different toolchain or against a different interface
revision must never be instantiated. The gate:

- `kPluginInterfaceVersion` (in `plugin_interface.h`) is the frozen interface
  revision. Within a version the interface is immutable; **only additive changes
  appended to the END of `IPlugin`** are allowed without a bump.
- Every plugin DLL exports `PluginInterfaceVersion()` (see above). The manager
  resolves it before calling `CreatePlugin` and rejects a mismatch with a clear
  log line — a missing export counts as version 0 (a plugin that predates
  versioning) and is rejected the same way.
- Bump `kPluginInterfaceVersion` on ANY ABI-breaking change (adding/removing/
  reordering virtuals, changing the exported symbol set), and re-export the new
  version from every plugin.

### Host side

```cpp
#include <unigui/plugin/plugin_manager.h>

void load_plugins() {
    auto& mgr = unigui::plugin::Manager::Instance();
    mgr.SetOnReload([](const std::string& name) {
        // re-bind anything that pointed at the old instance
    });

    if (unigui::plugin::IPlugin* p = mgr.Load("plugins/my_plugin.dll")) {
        for (const auto& name : mgr.List())
            /* show loaded plugins */ (void)name;
    }
    // ... per frame: for each plugin, call Update(dt) / Render()
    mgr.Shutdown();
}
```

---

## Config — `UNIGUI_MODULE_CONFIG` (default OFF)

**Header:** `<unigui/config/config.h>` · **Namespace:** `unigui::config` ·
**Source:** `src/config/config.cc` · **vcpkg deps:** `cpptoml`,
`nlohmann-json`

A unified configuration store supporting **TOML**, **JSON**, and **INI** input,
with layered merging (later layers override earlier ones) and typed accessors.
Internally all values are stored as strings keyed by a flat key path; typed
getters/setters convert on access. `Store` is a singleton via `Store::Instance()`.

### API

```cpp
namespace unigui::config {

class Store {
public:
    static Store& Instance();

    // Load — Result<void> (4.0): Err(ErrorCode::FileNotFound) if the path can't be
    // opened, Err(ErrorCode::ParseFailed) if the contents are malformed.
    Result<void> LoadTOML(const std::string& path);
    Result<void> LoadJSON(const std::string& path);
    Result<void> LoadINI(const std::string& path);

    // Save
    bool SaveTOML(const std::string& path) const;
    bool SaveJSON(const std::string& path) const;

    // Generic typed access
    template <typename T> T    Get(const std::string& key, T defaultVal = T{}) const;
    template <typename T> void Set(const std::string& key, const T& value);

    // Convenience typed accessors
    std::string GetString(const std::string& key, const std::string& def = "") const;
    void        SetString(const std::string& key, const std::string& value);
    int         GetInt(const std::string& key, int def = 0) const;
    void        SetInt(const std::string& key, int value);
    double      GetDouble(const std::string& key, double def = 0.0) const;
    void        SetDouble(const std::string& key, double value);
    bool        GetBool(const std::string& key, bool def = false) const;
    void        SetBool(const std::string& key, bool value);

    // Keys
    bool                     Has(const std::string& key) const;
    std::vector<std::string> Keys() const;

    // Layering
    void Merge(const Store& other);   // higher-priority `other` overwrites
    void Clear();
};

} // namespace unigui::config
```

Notes:
- The three `Load*` functions return `false` on parse/IO failure. Loading is
  additive — call `Clear()` first if you want a clean slate.
- **`Merge`** overlays another `Store` on top of this one (the argument wins on
  key collisions), which is how you layer defaults → user config → environment.
- `SaveTOML` / `SaveJSON` serialize the current flat key/value set.

### Example

```cpp
#include <unigui/config/config.h>

void load_config() {
    auto& cfg = unigui::config::Store::Instance();

    cfg.LoadTOML("defaults.toml");   // base layer
    cfg.LoadJSON("user.json");       // user overrides (additive)

    std::string theme = cfg.GetString("ui.theme", "dark");
    int    width      = cfg.GetInt("window.width", 1280);
    bool   vsync      = cfg.GetBool("render.vsync", true);
    double scale      = cfg.GetDouble("ui.scale", 1.0);

    cfg.SetInt("window.width", 1920);
    if (cfg.Has("window.height"))
        cfg.SaveTOML("user.toml");

    for (const auto& key : cfg.Keys())
        /* enumerate */ (void)key;
}
```

---

## SQLite — `UNIGUI_MODULE_SQLITE` (default OFF)

**Header:** `<unigui/sqlite/database.h>` · **Namespace:** `unigui::sqlite` ·
**Source:** `src/sqlite/database.cc` · **vcpkg deps:** `sqlite3`

A thin, safe wrapper over the SQLite C API. It provides parameterized
`Execute`/`Query` (binding via a `std::variant` `Param`), a single-value helper,
RAII transactions, and an idempotent schema-`Migrate` helper. Bound parameters
defend against SQL injection — prefer them over string concatenation.

### API

```cpp
namespace unigui::sqlite {

using Param = std::variant<int, double, std::string, std::nullptr_t, const char*>;

struct Row {
    std::vector<std::string> columns;
    std::string Get(int i) const;             // column by index (safe-bounded)
    std::string Get(const char* name);        // column by name
};

class Database {
public:
    Database() = default;
    ~Database();

    Result<void> Open(const std::string& path); // 4.0: Err(ErrorCode::OpenFailed) on failure
    void Close();
    bool IsOpen() const;

    // INSERT/UPDATE/DELETE — returns affected rows, or -1 on error.
    int Execute(const std::string& sql, const std::vector<Param>& params = {});

    // SELECT — invokes callback per row, returns row count.
    int Query(const std::string& sql, const std::vector<Param>& params,
              std::function<void(Row&)> callback);

    // First column of first row.
    std::string QueryValue(const std::string& sql, const std::vector<Param>& params = {});

    // Run `sql` only if `version` hasn't been applied yet.
    bool Migrate(int version, const std::string& sql);

    int64_t  LastInsertId();
    sqlite3* Raw();                            // escape hatch to the C handle
};

// RAII transaction — BEGIN on construct, ROLLBACK on destruct unless Commit()'d.
class Transaction {
public:
    Transaction(Database& db);
    ~Transaction();
    void Commit();
};

} // namespace unigui::sqlite
```

Notes:
- **`Param`** accepts `int`, `double`, `std::string`, `nullptr` (binds SQL
  `NULL`), and `const char*`.
- **`Transaction`** issues `BEGIN` in its constructor and `ROLLBACK` in its
  destructor — call `Commit()` to make the changes stick. This guarantees
  atomicity even if an exception unwinds the scope.
- **`Migrate`** is idempotent per `version`: it tracks applied versions, so
  re-running your migration list on startup is safe.
- **`Raw()`** exposes the underlying `sqlite3*` for advanced use.

### Example

```cpp
#include <unigui/sqlite/database.h>

void use_db() {
    unigui::sqlite::Database db;
    if (!db.Open("app.db")) return;

    db.Migrate(1, "CREATE TABLE users(id INTEGER PRIMARY KEY, name TEXT, age INT);");

    {
        unigui::sqlite::Transaction tx(db);
        db.Execute("INSERT INTO users(name, age) VALUES(?, ?)",
                   {std::string("Ada"), 36});
        db.Execute("INSERT INTO users(name, age) VALUES(?, ?)",
                   {std::string("Linus"), 54});
        tx.Commit();                       // omit to roll back
    }

    int n = db.Query("SELECT id, name FROM users WHERE age > ?",
                     {30},
                     [](unigui::sqlite::Row& row) {
                         std::string id   = row.Get(0);
                         std::string name = row.Get(1);
                         (void)id; (void)name;
                     });

    std::string count = db.QueryValue("SELECT COUNT(*) FROM users");
    int64_t last = db.LastInsertId();
    (void)n; (void)count; (void)last;
}
```

---

## IPC — `UNIGUI_MODULE_IPC` (default OFF)

**Headers:** `<unigui/ipc/ipc.h>`, `<unigui/ipc/shmem.h>` ·
**Namespace:** `unigui::ipc` · **Sources:** `src/ipc/ipc.cc`,
`src/ipc/shmem.cc` · **vcpkg deps:** `zeromq`

Two complementary inter-process mechanisms:

1. A **message channel** over ZeroMQ — a `Server` (publisher) and `Client`
   (subscriber) over a TCP transport, sharing a common `Channel` interface.
2. A cross-platform **shared-memory segment** (`SharedMemory`) using the Win32
   file-mapping API on Windows and POSIX `mmap`/`shm` elsewhere.

### Message channel API (`ipc.h`)

```cpp
namespace unigui::ipc {

class Channel {
public:
    virtual ~Channel() = default;
    virtual bool Send(const std::string& msg) = 0;
    virtual void OnReceive(std::function<void(const std::string&)> cb) = 0;
    virtual void Close() = 0;
};

// ZMQ publisher (server).
class Server : public Channel {
public:
    Server(const std::string& address = "tcp://*:5555");
    ~Server();
    bool Send(const std::string& msg) override;
    void OnReceive(std::function<void(const std::string&)> cb) override;
    void Close() override;
    bool Start();
};

// ZMQ subscriber (client).
class Client : public Channel {
public:
    Client(const std::string& address = "tcp://localhost:5555");
    ~Client();
    bool Connect();
    bool Send(const std::string& msg) override;
    void OnReceive(std::function<void(const std::string&)> cb) override;
    void Close() override;
    bool Poll(int timeoutMs = 0);   // pump received messages into OnReceive
};

} // namespace unigui::ipc
```

### Shared-memory API (`shmem.h`)

```cpp
namespace unigui::ipc {

class SharedMemory {
public:
    SharedMemory(const std::string& name, size_t size);
    ~SharedMemory();
    void   Write(const void* data, size_t size, size_t offset = 0);
    void   Read(void* data, size_t size, size_t offset = 0);
    size_t Size() const;
    void*  Data();                 // raw pointer into the mapping
};

} // namespace unigui::ipc
```

Notes:
- **`Server::Start()`** binds the socket; **`Client::Connect()`** connects it.
  The `Client` is poll-driven — call **`Poll(timeoutMs)`** (e.g. each frame) to
  dispatch any pending messages into the callback registered via `OnReceive`.
- **`SharedMemory`** maps a named segment of `size` bytes shared across
  processes; `Write`/`Read` move bytes at a given `offset`, and `Data()` exposes
  the mapping directly for in-place structures. Synchronize access yourself (the
  class provides no locking).

### Example — ZMQ pub/sub

```cpp
#include <unigui/ipc/ipc.h>

// Process A — publisher
void run_server() {
    unigui::ipc::Server srv("tcp://*:5555");
    if (srv.Start())
        srv.Send("hello from server");
}

// Process B — subscriber
void run_client() {
    unigui::ipc::Client cli("tcp://localhost:5555");
    cli.OnReceive([](const std::string& msg) { /* handle msg */ (void)msg; });
    if (cli.Connect()) {
        // pump messages, e.g. once per frame
        cli.Poll(10 /*ms*/);
    }
}
```

### Example — shared memory

```cpp
#include <unigui/ipc/shmem.h>
#include <cstring>

struct Telemetry { float fps; int frame; };

void writer() {
    unigui::ipc::SharedMemory shm("unigui.telemetry", sizeof(Telemetry));
    Telemetry t{60.0f, 1};
    shm.Write(&t, sizeof(t));
}

void reader() {
    unigui::ipc::SharedMemory shm("unigui.telemetry", sizeof(Telemetry));
    Telemetry t{};
    shm.Read(&t, sizeof(t));
}
```

---

## Network — `UNIGUI_MODULE_NETWORK` (default OFF)

**Header:** `<unigui/network/network.h>` · **Namespace:** `unigui::network` ·
**Source:** `src/network/network.cc` · **vcpkg deps:** `cpp-httplib`,
`ixwebsocket`

An HTTP client (synchronous GET/POST, built on cpp-httplib) and a WebSocket
client (event-driven, built on IXWebSocket).

### API

```cpp
namespace unigui::network {

struct HttpResponse {
    int                                status = 0;
    std::string                        body;
    std::map<std::string, std::string> headers;
};

// Synchronous HTTP client (static methods).
class HttpClient {
public:
    static HttpResponse Get(const std::string& url,
                            const std::map<std::string, std::string>& headers = {});

    static HttpResponse Post(const std::string& url,
                             const std::string& body = "",
                             const std::string& contentType = "application/json",
                             const std::map<std::string, std::string>& headers = {});
};

// Event-driven WebSocket client.
class WebSocketClient {
public:
    WebSocketClient();
    ~WebSocketClient();

    bool Connect(const std::string& url);
    void Send(const std::string& msg);
    void OnMessage(std::function<void(const std::string&)> cb);
    void OnOpen(std::function<void()> cb);
    void OnClose(std::function<void()> cb);
    bool IsConnected() const;
    void Disconnect();
};

} // namespace unigui::network
```

Notes:
- **`HttpClient`** is fully static — `Get`/`Post` block until the response
  arrives and return an `HttpResponse` (`status`, `body`, `headers`). Register
  callbacks **before** calling `Connect` on the WebSocket so you don't miss the
  open event.
- **`WebSocketClient`** runs its own background thread (via IXWebSocket);
  callbacks fire off that thread, so marshal back to the UI thread before
  touching ImGui state.

### Example

```cpp
#include <unigui/network/network.h>

void fetch() {
    using namespace unigui::network;

    HttpResponse r = HttpClient::Get("https://api.example.com/status",
                                     {{"Authorization", "Bearer TOKEN"}});
    if (r.status == 200)
        /* parse r.body */ (void)r.body;

    HttpClient::Post("https://api.example.com/orders",
                     R"({"symbol":"BTCUSD","qty":1})",
                     "application/json");
}

void stream() {
    auto ws = std::make_unique<unigui::network::WebSocketClient>();
    ws->OnOpen([]    { /* connected */ });
    ws->OnMessage([](const std::string& m) { /* push to UI queue */ (void)m; });
    ws->OnClose([]   { /* reconnect logic */ });
    if (ws->Connect("wss://stream.example.com/feed"))
        ws->Send(R"({"subscribe":"ticker"})");
    // ... later: ws->Disconnect();
}
```

---

## Fonts — `UNIGUI_MODULE_FONTS` (default ON)

**Headers:** `<unigui/fonts/font_manager.h>`,
`<unigui/fonts/gradient_text.h>` · **Namespace:** `unigui::fonts` /
`unigui::` · **Sources:** `src/fonts/font_manager.cc`,
`src/fonts/gradient_text.cc` · **vcpkg deps:** none (uses the always-present
`imgui` + `Freetype`)

A font registry on top of ImGui's font atlas: load fonts from TTF files or
memory, name them, define **fallback chains** for missing glyphs, push/pop the
active font per scope, and load the platform emoji font. The `gradient_text`
helper draws horizontally colour-interpolated text. Note that
`src/fonts/gradient_text.cc` is compiled as part of the **widgets** module group,
while `font_manager.cc` is compiled under `UNIGUI_MODULE_FONTS`.

### Font manager API

```cpp
namespace unigui::fonts {

struct FontEntry {
    std::string              name;
    ImFont*                  font     = nullptr;
    void*                    data     = nullptr;   // owned (AddFontFromMemoryTTF)
    int                      dataSize = 0;
    std::vector<std::string> fallbacks;
};

class Manager {
public:
    static Manager& Instance();

    ImFont* Load(const std::string& name, const std::string& path, float size);
    ImFont* LoadFromMemory(const std::string& name, const void* data,
                           int size, float fontSize);
    ImFont* Get(const std::string& name) const;     // nullptr if not found
    bool    Unload(const std::string& name);

    void SetDefault(const std::string& name);
    void SetFallback(const std::string& name, const std::string& fallbackName);

    std::vector<std::string> List() const;

    void Push(const std::string& name);             // scoped push/pop
    void Pop();

    void Build();                                   // (re)build the atlas
    void LoadSystemEmoji(float size = 0);           // platform emoji as fallback
};

} // namespace unigui::fonts
```

> **Important:** `Build()` rebuilds the ImGui font atlas and **invalidates all
> previously obtained `ImFont*` pointers**. Either load every font first and
> `Build()` once, or call `Get()` again after each `Build()` to re-fetch valid
> pointers (the `Manager` re-caches them from the new atlas). `LoadSystemEmoji`
> uses "Segoe UI Emoji" on Windows, "Apple Color Emoji" on macOS, and attempts
> "Noto Color Emoji" on Linux, wiring it as a fallback of the default font.

### Gradient text API

```cpp
namespace unigui {

struct GradientText {
    // Left-to-right per-character colour interpolation.
    static void Render(const char* text, ImU32 leftColor, ImU32 rightColor);

    // Same, with hex channel components.
    static void RenderHex(const char* text,
                          unsigned lr, unsigned lg, unsigned lb,
                          unsigned rr, unsigned rg, unsigned rb);
};

} // namespace unigui
```

### Example

```cpp
#include <unigui/fonts/font_manager.h>
#include <unigui/fonts/gradient_text.h>

void setup_fonts() {
    auto& fm = unigui::fonts::Manager::Instance();

    fm.Load("ui",   "fonts/Inter.ttf",          18.0f);
    fm.Load("mono", "fonts/JetBrainsMono.ttf",   16.0f);
    fm.SetFallback("ui", "mono");      // missing glyphs fall through to mono
    fm.LoadSystemEmoji();              // emoji as a fallback of the default
    fm.SetDefault("ui");
    fm.Build();                        // build once, after all loads
}

void draw_text() {
    auto& fm = unigui::fonts::Manager::Instance();
    fm.Push("mono");
    ImGui::TextUnformatted("monospaced");
    fm.Pop();

    // Gradient heading (cyan → magenta)
    unigui::GradientText::Render("UniGUI",
                                 IM_COL32(0, 200, 255, 255),
                                 IM_COL32(255, 0, 200, 255));
}
```

---

## Always-on framework modules (brief)

These ship enabled by default and require no extra vcpkg packages. They are
documented in depth elsewhere; this section notes only their gating and entry
points.

### Widgets — `UNIGUI_MODULE_WIDGETS` (default ON)

The retained-mode widget library — 92 widget classes in `unigui::`
(tables, tree/list views, color pickers, date pickers, charts, dialogs, toasts,
command palette, etc.). The fundamental primitives (`Button`, `Label`, `Panel`,
`Form`, `Window`, `Checkbox`, …) are compiled into the **core** group and are
always available; `UNIGUI_MODULE_WIDGETS` adds the extended set (`Table`,
`TreeView`, `ListView`, `ColorPicker`, `DatePicker`, `VirtualList`,
`PropertyGrid`, `CommandPalette`, `FileDialog`, and many more — see
`src/CMakeLists.txt`). Enabling it defines `UNIGUI_HAS_WIDGETS`. Full reference:
[`WIDGET_API.md`](WIDGET_API.md) and [`WIDGET_EXAMPLES.md`](WIDGET_EXAMPLES.md).

### DSL — `UNIGUI_MODULE_DSL` (default ON)

The declarative UI builders and component framework in `unigui::dsl`
(`<unigui/dsl/dsl.h>`, `<unigui/dsl/component.h>`, `<unigui/dsl/app.h>`).
Implementation: `src/dsl/dsl.cc`; defines `UNIGUI_HAS_DSL`. Enables the
`hello_unigui_v2` and `framework_demo` examples.

### Styling — `UNIGUI_MODULE_STYLING` (default ON)

The CSS-like style engine in `unigui::styling` (`<unigui/styling/style_engine.h>`).
Implementation: `src/styling/style_engine.cc`; defines `UNIGUI_HAS_STYLING`.

### Trading — `UNIGUI_MODULE_TRADING` (default OFF)

The trading-client toolkit in `unigui::trading`: header-only models
(`order_book`, `ohlc_series`, `quote`) plus presentation widgets
(`candlestick_chart`, `depth_ladder`, `order_ticket`, blotters). Sources:
`src/trading/*.cc`; defines `UNIGUI_HAS_TRADING`; requires `UNIGUI_MODULE_WIDGETS`
for the `trading_dashboard` example. Full reference: [`TRADING.md`](TRADING.md).

---

## Backends (orthogonal to modules)

Renderer/platform backends are separate `UNIGUI_BACKEND_*` options, not module
toggles, and select which graphics path the app loop uses:

| Option | Default | Notes |
|---|---|---|
| `UNIGUI_BACKEND_GLFW3` | ON | GLFW + OpenGL3 — the cross-platform default |
| `UNIGUI_BACKEND_DX11` | ON (Windows) | Direct3D 11 — Windows production path |
| `UNIGUI_BACKEND_DX12` | OFF | Direct3D 12 (Windows only) |
| `UNIGUI_BACKEND_VULKAN` | OFF | Vulkan renderer via GLFW (cross-platform) |
| `UNIGUI_BACKEND_SDL3` | OFF | SDL3 platform backend (pairs with the Vulkan renderer; needs `sdl3` + `imgui[sdl3-binding]` + Vulkan) |

DX11/DX12 are Windows-only — disable them on Linux/macOS
(`-DUNIGUI_BACKEND_DX11=OFF -DUNIGUI_BACKEND_DX12=OFF`). Metal/WebGPU/Emscripten
renderers are present as **stubs**.

The `BackendType` enum (`<unigui/backend/backend_types.h>`) exposes
`GLFW_GL3`, `SDL3_Vulkan`, `DX11`, `DX12`, `Metal`, `WebGPU`, `Emscripten`, and
`Vulkan`. `AppConfig::backend` (`<unigui/app/app.h>`) defaults to `DX11` on
Windows and `GLFW_GL3` elsewhere.

```cpp
unigui::AppConfig cfg;
cfg.backend = unigui::BackendType::DX11;   // or GLFW_GL3, Vulkan, …
```

---

## Minimal consumer (core widgets only)

To strip the library down to the core widget primitives and nothing else:

```cmake
set(UNIGUI_MODULE_WIDGETS OFF)   # extended widgets off (core primitives remain)
set(UNIGUI_MODULE_DSL      OFF)
set(UNIGUI_MODULE_STYLING  OFF)
set(UNIGUI_MODULE_FONTS    OFF)
set(UNIGUI_MODULE_EVENTS   OFF)
set(UNIGUI_MODULE_PLUGIN   OFF)
set(UNIGUI_BUILD_TESTS     OFF)
set(UNIGUI_BUILD_EXAMPLES  OFF)
add_subdirectory(third_party/TeamkillerUniGUI)
```

The optional dependency-heavy modules (`SQLITE`, `CONFIG`, `IPC`, `NETWORK`,
`TRADING`) are already OFF by default, so a minimal consumer needs no extra
vcpkg packages beyond the base set (`imgui`, `glfw3`, `glad`, `Freetype`,
`implot`, `spdlog`).

## See also

- [`README.md`](../README.md) — API overview and full widget table
- [`WIDGET_API.md`](WIDGET_API.md) — complete widget API reference
- [`TRADING.md`](TRADING.md) — trading toolkit
- [`INTEGRATION.md`](../INTEGRATION.md) — submodule + vcpkg embedding
- [`API_STABILITY.md`](API_STABILITY.md) — the public-API semver contract
