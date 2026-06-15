# Examples & Cookbook

Runnable demo: [`examples/hello_unigui/main.cc`](../examples/hello_unigui/main.cc).

All retained widgets: construct **once**, call **`Render()` every frame**.

**Per-widget minimal snippets (all 93 entries):** [WIDGET_EXAMPLES.md](WIDGET_EXAMPLES.md)  
**Full API + merged TreeView / CascadingCombo:** [WIDGET_API.md](WIDGET_API.md)  
**Alphabetical index:** [API_INDEX.md](API_INDEX.md)

---

## 1. Minimal window (`RunApp`)

```cpp
#include <unigui/unigui.h>

int main() {
    unigui::AppConfig cfg;
    cfg.title = "Demo";
    return unigui::RunApp(cfg, [] {
        if (ImGui::Begin("Panel")) {
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }, /*maxFrames=*/0);
}
```

---

## 2. Retained `Button` + fluent API

```cpp
static auto save = std::make_shared<unigui::Button>("save", "Save");
static bool dirty = true;

void ui() {
    save->WithTooltip("Ctrl+S")
        .WithEnabled(dirty)
        .WithPrimary()
        .Render();
    if (save->WasClicked()) { /* persist */ dirty = false; }
}
```

---

## 3. Immediate mode (`unigui::im`)

```cpp
#include <unigui/im/im.h>
namespace im = unigui::im;

static bool enabled = true;
static float gain = 0.5f;

void ui() {
    im::Checkbox("Enabled", &enabled);
    if (im::Button("Apply", im::ButtonVariant::Primary)) apply();
    im::SliderFloat("Gain", &gain, 0.f, 1.f);
    im::SameLine();
    im::Text("ok");
}
```

---

## 4. RAII scopes

```cpp
#include <unigui/core/scope.h>

void settings(bool readOnly) {
    if (unigui::WindowScope w{"Settings"}) {
        unigui::IDScope row{"row1"};
        unigui::DisabledScope d{readOnly};
        unigui::im::Button("Apply");
    }
}
```

---

## 5. `TabWidget`

```cpp
static unigui::TabWidget tabs("main_tabs");
static bool init = false;
if (!init) {
    tabs.AddTab({"home", "Home", []{ ImGui::Text("Home"); }});
    tabs.AddTab({"settings", "Settings", []{ ImGui::Text("Settings"); }, /*closable=*/true});
    init = true;
}
tabs.Render();
```

---

## 6. `MultiSplitter` (multi-column layout)

```cpp
static unigui::MultiSplitter split("cols", unigui::MultiSplitter::Vertical);
static bool once = false;
if (!once) {
    split.AddPanel(0.25f, []{ /* left */ });
    split.AddPanel(0.50f, []{ /* center */ });
    split.AddPanel(0.25f, []{ /* right */ });
    once = true;
}
split.Render();
```

---

## 7. `DataTable<T>` (large lists)

```cpp
struct Row { std::string name; double value; };
static std::vector<Row> rows = {{"A", 1.2}, {"B", 3.4}};
static unigui::DataTable<Row> table("tbl", {
    {"Name", 120}, {"Value", 80, /*sortable*/ true}
});

void ui() {
    table.SetDataSource(&rows);
    table.SetColumnStretch(1, 1.f);
    table.SetCellFormatter([](int r, int c, const Row& row) {
        if (c == 1) return std::format("{:.2f}", row.value);
        return row.name;
    });
    table.SetRowColor([](int r, const Row&) {
        return r % 2 ? IM_COL32(255,255,255,8) : 0;
    });
    table.Render();
}
```

See also [WIDGET_API — DataTable](WIDGET_API.md#datatablet).

---

## 8. `TimeSeriesChart` (real-time plot)

```cpp
static unigui::TimeSeriesChart chart("live");
static int series = -1;
static bool init = false;
if (!init) {
    chart.SetSlidingWindow(500);
    chart.SetYAxisAutoFit(true);
    chart.SetYRangeFit(true);
    chart.SetLegendEnabled(true);
    series = chart.AddSeries({.label = "Spread", .color = IM_COL32(14,165,233,255)});
    init = true;
}
chart.AppendPoint(series, value, timestampSec);
chart.Render();
```

---

## 9. `Toast` notifications

```cpp
#include <unigui/widgets/toast.h>

void onSuccess() { unigui::Toast::Success("Saved"); }
void onError()   { unigui::Toast::Error("Network error"); }

// Each frame, after other UI:
unigui::Toast::Instance().Render();
```

---

## 10. Theme + surface material

```cpp
unigui::AppConfig cfg;
cfg.theme.preset = unigui::ThemePreset::Dark;
cfg.theme.surface = unigui::theme::SurfaceStyle::Glass;
cfg.theme.font_size = 18.f;
unigui::Init(cfg);

// Runtime preset switch:
unigui::theme::ThemeRegistry::Apply("nord");
```

---

## 11. Background thread → UI thread

```cpp
#include <unigui/core/main_thread.h>

void onNetworkMessage(const Msg& m) {
    unigui::InvokeOnMainThread([m]() {
        queue.push_back(m);
    });
}

// Main loop:
unigui::NewFrame();
unigui::ProcessMainThreadTasks();
// ... render ...
```

---

## 12. Declarative DSL (optional)

Enable `UNIGUI_MODULE_DSL` in CMake, then:

```cpp
#include <unigui/dsl/dsl.h>
using namespace unigui::dsl;

bool ok = true;
auto ui = Window("DSL", VBox({
    Text("Hello"),
    CheckBox("OK", &ok),
    Button("Quit", ButtonVariant::Danger, []{ std::exit(0); })
}));
// each frame:
Render(ui);
```

Details: [MODULES.md](MODULES.md), [WIDGET_API — DSL](WIDGET_API.md#declarative-dsl-uniguidsl).

---

## 13. `TreeView` — `TextSpan` + custom row

```cpp
#include <unigui/widgets/treeview.h>

unigui::TreeNode root;
root.label = "Portfolio";
unigui::TreeNode leaf;
leaf.label = "IF2506";
leaf.spans = {
    {"IF2506 ", 0},
    {"多", IM_COL32(220, 60, 60, 255)},
    {" 12手", 0},
};
root.children.push_back(std::move(leaf));

auto tv = std::make_shared<unigui::TreeView>("pos");
tv->SetRoot(std::move(root));
tv->SetHideRoot(true);
tv->SetRowRenderer([](int, int, const unigui::TreeNode& n, bool) {
    if (!n.spans.empty()) {
        for (const auto& s : n.spans)
            ImGui::TextColored(ImGui::ColorConvertU32ToFloat4(s.color), "%s", s.text.c_str());
    } else {
        ImGui::TextUnformatted(n.label.c_str());
    }
});
tv->Render();
```

Full API: [WIDGET_API — TreeView](WIDGET_API.md#treeview).

---

## 14. `CascadingCombo` — horizontal + linkage

```cpp
auto cc = std::make_shared<unigui::CascadingCombo>("region");
cc->SetLevels({
    {"省", {"江苏", "浙江"}},
    {"市", {"南京", "苏州"}},
    {"区", {"玄武区", "鼓楼区"}},
});
cc->WithLayout(unigui::CascadingCombo::Layout::Horizontal)
   .WithItemWidth(120.f)
   .WithShowLabels(true);
cc->SetOnChanged([&](int level, int index) {
    if (level == 0) cc->SetOptions(1, CitiesForProvince(index));
});
cc->Render();
```

Full API: [WIDGET_API — CascadingCombo](WIDGET_API.md#cascadingcombo).

---

## 15. Custom + raw ImGui

UniGUI does not block raw ImGui. Mix freely inside callbacks:

```cpp
ImGui::Begin("Debug");
ImGui::Checkbox("ImGui demo", &show);
if (show) ImGui::ShowDemoWindow(&show);
ImGui::End();
```

Theme colors apply to styled widgets; demo window uses ImGui defaults unless themed.
