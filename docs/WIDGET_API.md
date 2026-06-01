# UniGUI Widget API Reference

> **Version**: 0.1.0 (C++23) | **Widgets**: 74+ | **Backend**: Dear ImGui
>
> UniGUI is a modern C++23 wrapper around Dear ImGui. All widgets live in `namespace unigui`.
> Every constructor takes a `std::string name` as its first argument — this becomes the ImGui ID
> and the library handles `PushID`/`PopID` automatically.

---

## Quick Start

```cpp
#include <unigui/unigui.h>

int main() {
    unigui::AppConfig cfg;
    cfg.title = "My App";
    unigui::Init(cfg);

    // Create widgets once (outside the render loop)
    auto btn = std::make_shared<unigui::Button>("btn_ok", "OK");
    btn->SetColorVariant(unigui::Button::Primary);

    unigui::Run([&] {
        // --- Render loop ---
        btn->Render();
        if (btn->WasClicked())
            std::println("Button clicked!");
    });

    unigui::Shutdown();
}
```

Every widget must be `Render()`-ed every frame. The `Run()` callback is your frame loop.

---

## ID Safety: PushID / PopID

Dear ImGui requires **unique IDs** per widget to avoid label collisions. UniGUI handles this
**completely automatically**:

- Every widget constructor takes a `std::string name` that becomes its stable ImGui ID.
- Containers that render children (Window, Panel, Card, GroupBox, TreeView, etc.) manage
  `PushID` / `PopID` internally — user code never touches them.
- Nested widgets with the same label **just work** because each has its own name-based ID.

```cpp
auto panel = std::make_shared<unigui::Panel>("settings", "Settings");
// Create any widgets inside — no ID conflicts, ever.
```

**You never need to call `ImGui::PushID` or `ImGui::PopID` yourself.**

---

## Containers

### Window

Top-level application window. Owns panels.

```cpp
// Constructor
Window(std::string name, std::string title);

// Key methods
void AddPanel(std::shared_ptr<Panel> panel);
void SetSize(float width, float height);
void SetPosition(float x, float y);
void SetMenuBarEnabled(bool enabled);
void SetDropCallback(std::function<void(std::vector<std::string>)> cb);
std::string SaveLayout() const;
void RestoreLayout(const std::string& json);

// Example
auto win = std::make_shared<unigui::Window>("main", "My App");
win->SetSize(800, 600);
auto panel = std::make_shared<unigui::Panel>("content", "Dashboard");
win->AddPanel(panel);
win->Render();
```

> **PushID**: The Window name is the ImGui window ID. Panels are rendered inside via `BeginChild`.

---

### Panel

Collapsible section inside a Window with a content callback.

```cpp
Panel(std::string name, std::string title);

// Key methods
void SetContentCallback(std::function<void()> callback);
void SetTitle(std::string title);
bool IsCollapsed() const;

// Example
auto panel = std::make_shared<unigui::Panel>("props", "Properties");
panel->SetContentCallback([&] {
    // Render child widgets here — IDs are scoped automatically
    unigui::Label{"lbl_name", "Name:"}.Render();
});
panel->Render();
```

---

### GroupBox

Bordered group with a title and content callback.

```cpp
GroupBox(std::string name, std::string title);

void SetContentCallback(std::function<void()> callback);

// Example
auto gb = std::make_shared<unigui::GroupBox>("gb_style", "Style");
gb->SetContentCallback([&] {
    unigui::CheckBox{"cb_bold", "Bold"}.Render();
});
gb->Render();
```

---

### TabWidget

Multi-tab container. Tabs are defined as `TabPage` structs.

```cpp
TabWidget(std::string name);

void AddTab(TabPage page);
void RemoveTab(const std::string& tab_name);
int  GetActiveTab() const;
void SetActiveTab(int index);

// Example
auto tabs = std::make_shared<unigui::TabWidget>("tabs_main");
tabs->AddTab({"general", "General", [&] {
    // Tab content — automatically scoped
    unigui::Label{"lbl1", "General settings"}.Render();
}});
tabs->AddTab({"advanced", "Advanced", [&] {
    unigui::Label{"lbl2", "Advanced settings"}.Render();
}});
tabs->Render();
```

---

### Card

Elevated surface card with title, content, footer, and shadow.

```cpp
Card(const std::string& title = "");

void SetContent(std::function<void()> fn);
void SetFooter(std::function<void()> fn);
void SetVariant(Variant v);  // Elevated, Outlined, Filled
void SetShadow(bool enable);
void SetPadding(float p);

// Example
unigui::Card card{"User Profile"};
card.SetVariant(unigui::Card::Elevated);
card.SetContent([&] {
    unigui::Label{"card_name", "John Doe"}.Render();
});
card.SetFooter([&] {
    unigui::Button{"card_btn", "Edit"}.Render();
});
card.Render();
```

> **Note**: Card is NOT a `Widget` subclass — it's a standalone RAII-style renderer.

---

### HeroSection

Tall gradient banner with title, subtitle, and CTA button.

```cpp
HeroSection(std::string name, std::string title = "", std::string subtitle = "");

void SetBackground(ImU32 topColor, ImU32 bottomColor);
void SetActionButton(std::string label, std::function<void()> callback);
void SetHeight(float h);

// Example
auto hero = std::make_shared<unigui::HeroSection>("hero", "Welcome", "v2.0");
hero->SetBackground(IM_COL32(40, 49, 237, 255), IM_COL32(233, 69, 96, 255));
hero->SetActionButton("Get Started", [] { /* ... */ });
hero->Render();
```

---

### ScrollArea

Scrollable region with a content callback.

```cpp
ScrollArea(std::string name, float width = 0, float height = 200);

void SetContentCallback(std::function<void()> cb);
void SetSize(float w, float h);

// Example
auto scroll = std::make_shared<unigui::ScrollArea>("log_area", 0, 300);
scroll->SetContentCallback([&] {
    for (int i = 0; i < 100; ++i)
        unigui::Label{"log_" + std::to_string(i), "Line " + std::to_string(i)}.Render();
});
scroll->Render();
```

---

### Splitter

Two-panel resizable split (horizontal or vertical).

```cpp
Splitter(std::string name, Orientation orientation = Horizontal, float split = 0.5f);

float GetSplit() const;
void SetContentA(std::function<void()> cb);
void SetContentB(std::function<void()> cb);

// Example
auto split = std::make_shared<unigui::Splitter>("split_main", unigui::Splitter::Horizontal, 0.3f);
split->SetContentA([&] { /* sidebar */ });
split->SetContentB([&] { /* main content */ });
split->Render();
```

---

### MultiSplitter

N-panel resizable layout with drag handles.

```cpp
MultiSplitter(std::string name, Orientation ori = Horizontal);

void AddPanel(float ratio, std::function<void()> content);
std::vector<float> GetRatios() const;

// Example
auto ms = std::make_shared<unigui::MultiSplitter>("ms", unigui::MultiSplitter::Vertical);
ms->AddPanel(0.3f, [&] { /* top */ });
ms->AddPanel(0.4f, [&] { /* middle */ });
ms->AddPanel(0.3f, [&] { /* bottom */ });
ms->Render();
```

---

### DockSpace

Dear ImGui docking space (full-window docking layout).

```cpp
DockSpace(std::string name);

// Example
auto dockspace = std::make_shared<unigui::DockSpace>("main_dock");
dockspace->Render();
// Windows can then dock into this space via ImGui::DockBuilder
```

---

### CollapsingHeader

Expandable/collapsible header section.

```cpp
CollapsingHeader(std::string name, std::string label, bool default_open = false);

bool IsOpen() const;
void SetContentCallback(std::function<void()> cb);

// Example
auto hdr = std::make_shared<unigui::CollapsingHeader>("hdr_details", "Details", true);
hdr->SetContentCallback([&] {
    unigui::Label{"detail", "Additional information..."}.Render();
});
hdr->Render();
```

---

## Inputs

### Button

Clickable button with color variants and sizes.

```cpp
Button(std::string name, std::string label);

bool WasClicked() const;
void SetEnabled(bool enabled);
void SetColorVariant(ColorVariant variant);  // Default, Primary, Danger, Success
void SetSize(Size size);                      // Small, Medium, Large

// Example
auto btn = std::make_shared<unigui::Button>("btn_save", "Save");
btn->SetColorVariant(unigui::Button::Primary);
btn->SetSize(unigui::Button::Large);
btn->Render();
if (btn->WasClicked())
    SaveData();
```

---

### CheckBox

Boolean toggle with label.

```cpp
CheckBox(std::string name, std::string label, bool checked = false);

bool IsChecked() const;
void SetChecked(bool checked);
void SetOnChange(std::function<void(bool)> callback);

// Example
auto cb = std::make_shared<unigui::CheckBox>("cb_vsync", "VSync", true);
cb->SetOnChange([](bool on) { std::println("VSync: {}", on); });
cb->Render();
```

---

### ToggleSwitch

Animated on/off toggle switch.

```cpp
ToggleSwitch(std::string name, std::string label, bool on = false);

bool IsOn() const;
void SetOn(); void SetOff(); void Toggle();
void SetOnChange(std::function<void(bool)> cb);

// Example
auto tog = std::make_shared<unigui::ToggleSwitch>("dark_mode", "Dark Mode", true);
tog->Render();
```

---

### ComboBox

Dropdown selection with optional search and editability.

```cpp
ComboBox(std::string name, std::string label,
         std::vector<std::string> items = {}, int selected = 0);

int  GetSelectedIndex() const;
void SetSelectedIndex(int idx);
void SetItems(std::vector<std::string> items);
void SetEditable(bool on);
void SetSearchable(bool on);
void SetOnChange(std::function<void(int)> callback);

// Example
const std::vector<std::string> themes = {"Dark", "Light", "Solarized"};
auto combo = std::make_shared<unigui::ComboBox>("theme_pick", "Theme", themes, 0);
combo->SetSearchable(true);
combo->SetOnChange([](int idx) { std::println("Selected: {}", idx); });
combo->Render();
```

---

### MultiCombo

Multi-select combo with checkboxes.

```cpp
MultiCombo(std::string name, std::string label, std::vector<std::string> items = {});

bool IsSelected(int index) const;
void SetSelected(int index, bool sel);
std::vector<int> GetSelectedIndices() const;
std::string GetPreview() const;  // "Item1, Item2, +3 more..."

// Example
auto mc = std::make_shared<unigui::MultiCombo>("features", "Features",
    std::vector<std::string>{"Export", "Import", "Sync", "Backup"});
mc->Render();
for (int idx : mc->GetSelectedIndices())
    EnableFeature(idx);
```

---

### RadioGroup

Radio button group (single selection).

```cpp
RadioGroup(std::string name, std::vector<std::string> options, int selected = 0);

int  GetSelected() const;
void SetSelected(int index);
void SetOnChange(std::function<void(int)> callback);

// Example
auto rg = std::make_shared<unigui::RadioGroup>("align",
    std::vector<std::string>{"Left", "Center", "Right"}, 0);
rg->Render();
```

---

### LineEdit

Single-line text input with validation, undo/redo, password mode.

```cpp
LineEdit(std::string name, std::string label, std::string value = "");

std::string GetValue() const;
void SetValue(std::string value);
void SetPlaceholder(std::string text);
void SetValidator(std::function<bool(const std::string&)> fn);
void SetPasswordMode(bool on);
void SetReadOnly(bool on);
void SetMaxLength(int maxLen);
void Undo(); void Redo();

// Example
auto edit = std::make_shared<unigui::LineEdit>("email", "Email", "");
edit->SetPlaceholder("user@example.com");
edit->SetValidator([](const std::string& s) {
    return s.contains('@');  // simple check
});
edit->Render();
if (edit->HasError())
    std::println("Invalid email!");
```

---

### MultiLine

Multi-line text editor with undo/redo.

```cpp
MultiLine(std::string name, std::string text = "", int maxLines = 10);

void SetText(std::string t);
std::string GetText() const;
void SetEditable(bool on);
void Undo(); void Redo();

// Example
auto ml = std::make_shared<unigui::MultiLine>("notes", "", 15);
ml->SetEditable(true);
ml->Render();
SaveNotes(ml->GetText());
```

---

### PasswordInput

Password field with visibility toggle and strength indicator.

```cpp
PasswordInput(std::string name, std::string label, std::string value = "");

std::string GetValue() const;
int GetStrengthScore() const;  // 0=empty, 1=weak, 2=fair, 3=good, 4=strong

// Example
auto pwd = std::make_shared<unigui::PasswordInput>("pwd_new", "New Password");
pwd->Render();
if (pwd->GetStrengthScore() < 3)
    std::println("Password too weak!");
```

---

### SearchBox

Search input with filtered dropdown suggestions.

```cpp
SearchBox(std::string name, std::string hint = "Search...");

void SetItems(std::vector<std::string> items);
const std::string& GetQuery() const;
void SetOnSelect(std::function<void(const std::string&)> fn);

// Example
auto sb = std::make_shared<unigui::SearchBox>("global_search", "Type to search...");
sb->SetItems({"apple", "banana", "cherry", "date"});
sb->SetOnSelect([](const std::string& item) { OpenItem(item); });
sb->Render();
```

---

### InputInt / InputFloat

Numeric input fields with range clamping.

```cpp
InputInt(std::string name, std::string label, int value = 0, int min = 0, int max = 100);
InputFloat(std::string name, std::string label, float value = 0.0f, float min = 0.0f, float max = 100.0f);

int/float GetValue() const;
void SetValue(T v);
void SetRange(T min, T max);
void SetOnChange(std::function<void(T)> cb);

// Example
auto age = std::make_shared<unigui::InputInt>("age", "Age", 25, 0, 120);
auto height = std::make_shared<unigui::InputFloat>("height", "Height (cm)", 170.0f, 50.0f, 250.0f);
age->Render();
height->Render();
```

---

### DragFloat / DragInt

Draggable numeric inputs.

```cpp
DragFloat(std::string name, std::string label, float value = 0, float speed = 1.0f,
          float vmin = 0, float vmax = 0);
DragInt(std::string name, std::string label, int value = 0, float speed = 1.0f,
        int vmin = 0, int vmax = 0);

T GetValue() const;
bool WasChanged() const;

// Example
auto drag = std::make_shared<unigui::DragFloat>("scale", "Scale", 1.0f, 0.01f, 0.1f, 5.0f);
drag->Render();
if (drag->WasChanged())
    ApplyScale(drag->GetValue());
```

---

### Slider

Templated slider (`Slider<float>`, `Slider<int>`).

```cpp
Slider<T>(std::string name, std::string label, T value = T{}, T min = T{}, T max = T{100});

T GetValue() const;
void SetRange(T min, T max);
void SetFormat(const char* fmt);
void SetOnChange(std::function<void(T)> callback);

// Example
auto vol = std::make_shared<unigui::Slider<float>>("volume", "Volume", 0.75f, 0.0f, 1.0f);
vol->SetFormat("%.0f%%");
vol->SetOnChange([](float v) { SetVolume(v); });
vol->Render();
```

---

### MultiHandleSlider

Range slider with multiple draggable tick handles.

```cpp
MultiHandleSlider(std::string name);

void SetTicks(const std::vector<SliderTick>& ticks);
void AddTick(SliderTick tick);
void SetRange(float min, float max);
void SetOnTickChanged(TickChangedFn fn);
void SetCurrentMarker(float pos, ImU32 color);

// Example
auto mhs = std::make_shared<unigui::MultiHandleSlider>("timeline");
mhs->SetRange(0.f, 100.f);
mhs->AddTick({1, 25.f, IM_COL32(233, 69, 96, 255)});
mhs->AddTick({2, 75.f, IM_COL32(34, 197, 94, 255)});
mhs->SetCurrentMarker(50.f, IM_COL32(255, 255, 255, 200));
mhs->Render();
```

---

### SpinBox

Templated spin box (`SpinBox<int>`, `SpinBox<float>`) — numeric input with +/- buttons.

```cpp
SpinBox<T>(std::string name, std::string label, T val = T{}, T mn = T{}, T mx = T{100}, T step = T{1});

T GetValue() const;
void SetRange(T min, T max);
void SetStep(T step);
void SetOnChange(std::function<void(T)> cb);

// Example
auto count = std::make_shared<unigui::SpinBox<int>>("count", "Count", 5, 1, 100, 1);
count->Render();
```

---

### ColorEdit

RGBA color editor (4 floats).

```cpp
ColorEdit(std::string name, std::string label,
          float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f);

ImVec4 GetColor() const;
void SetColor(float r, float g, float b, float a = 1.0f);
bool WasChanged() const;

// Example
auto ce = std::make_shared<unigui::ColorEdit>("bg_color", "Background", 0.1f, 0.1f, 0.1f, 1.0f);
ce->Render();
if (ce->WasChanged()) {
    auto [r, g, b, a] = std::tuple{ce->GetColor().x, ce->GetColor().y,
                                    ce->GetColor().z, ce->GetColor().w};
}
```

---

### ColorPicker

RGB/HSV color picker with optional alpha channel.

```cpp
ColorPicker(std::string name, std::string label,
            std::array<float, 3> color = {0.0f, 0.0f, 0.0f});

std::array<float, 3> GetColor() const;
void SetAlpha(bool on);
void SetOnChange(std::function<void(std::array<float, 3>)> callback);

// Example
auto cp = std::make_shared<unigui::ColorPicker>("accent", "Accent Color");
cp->SetOnChange([](auto color) {
    ApplyAccent(color[0], color[1], color[2]);
});
cp->Render();
```

---

### DatePicker

Date selection with year/month/day.

```cpp
DatePicker(std::string name, std::string label);

std::array<int, 3> GetDate() const;  // {year, month, day}
void SetDate(int y, int m, int d);
void SetOnChange(std::function<void(int, int, int)> cb);

// Example
auto dp = std::make_shared<unigui::DatePicker>("birthday", "Birthday");
dp->SetDate(1990, 6, 15);
dp->SetOnChange([](int y, int m, int d) {
    std::println("Date: {}-{:02}-{:02}", y, m, d);
});
dp->Render();
```

---

### FilePath / DirPath

File/directory path picker with native dialog support.

```cpp
FilePath(std::string name, std::string label, Mode mode = Open);  // Open or Save
DirPath(std::string name, std::string label);

std::string GetPath() const;
void SetFilter(std::string filter);  // FilePath only
void SetOnPathChanged(std::function<void(std::string)> cb);

// Example
auto fp = std::make_shared<unigui::FilePath>("project_file", "Project",
                                              unigui::FilePath::Open);
fp->SetFilter("*.json");
fp->Render();

auto dp = std::make_shared<unigui::DirPath>("workspace_dir", "Workspace");
dp->Render();
```

---

## Display

### Label

Simple text display.

```cpp
Label(std::string name, std::string text = "");

void SetText(std::string text);

// Example
unigui::Label{"status", "Connected"}.Render();
```

---

### Separator

Horizontal separator with optional label.

```cpp
Separator(std::string name, std::string label = "");

// Example
unigui::Separator{"sep1", "Section A"}.Render();
```

---

### Image

Image display with scaling modes.

```cpp
Image(std::string name, void* textureID = nullptr, float w = 0, float h = 0);

void SetTexture(void* tex, float w, float h);
void SetScaleMode(ScaleMode mode);  // Fit, Stretch, Original

// Example
auto img = std::make_shared<unigui::Image>("logo", myTextureID, 128, 128);
img->Render();
```

---

### ImageButton

Button with an image and optional label.

```cpp
ImageButton(std::string name, std::string label = "");

void SetImage(ImTextureID textureID, float width, float height);
bool WasClicked() const;
void SetEnabled(bool enabled);

// Example
auto ibtn = std::make_shared<unigui::ImageButton>("save_icon", "Save");
ibtn->SetImage(myTextureID, 24, 24);
ibtn->Render();
```

---

### IconButton

Button with an icon character/string (use with icon fonts).

```cpp
IconButton(std::string name, std::string icon, std::string label = "");

bool WasClicked() const;
void SetEnabled(bool e);

// Example
auto ib = std::make_shared<unigui::IconButton>("close_icon", "✕", "Close");
ib->Render();
```

---

### Hyperlink

Clickable hyperlink with URL.

```cpp
Hyperlink(std::string name, std::string label, std::string url = "");

bool WasClicked() const;
void SetURL(std::string url);

// Example
unigui::Hyperlink{"docs", "Documentation", "https://example.com/docs"}.Render();
```

---

### RichText

Formatted text with bold/italic/color spans.

```cpp
RichText(std::string name, std::string text = "");

void SetSpans(std::vector<RichTextSpan> spans);
void AddSpan(std::string text, ImVec4 color, bool bold = false, bool italic = false);

// Example
auto rt = std::make_shared<unigui::RichText>("intro");
rt->AddSpan("Hello ", ImVec4{1, 1, 1, 1});
rt->AddSpan("World!", ImVec4{0.2f, 0.7f, 1.0f, 1.0f}, true);
rt->Render();
```

---

### Markdown

Simple Markdown renderer (headers, bold, italic, code, bullets, horizontal rules).

```cpp
Markdown(std::string name, std::string markdown = "");

void SetMarkdown(std::string md);
void SetLinkCallback(std::function<void(const std::string& url)> cb);
void SetMaxWidth(float w);

// Example
constexpr std::string_view helpText = R"md(
# Getting Started
Welcome to **UniGUI**. Here's how to begin:

- Install the library
- Create your first window
- Enjoy!

See [docs](https://example.com) for more.
)md";
auto md = std::make_shared<unigui::Markdown>("help", std::string{helpText});
md->SetLinkCallback([](const std::string& url) { OpenBrowser(url); });
md->Render();
```

---

### ProgressBar

Animated progress bar with gradient and state colors.

```cpp
ProgressBar(std::string name, float fraction = 0.0f);

void SetFraction(float f);
void SetState(State s);  // Normal, Warning, Error
void SetOverlayText(std::string text);

// Example
auto pb = std::make_shared<unigui::ProgressBar>("upload", 0.65f);
pb->SetOverlayText("65%");
pb->SetState(unigui::ProgressBar::Normal);
pb->Render();
```

---

### LoadingIndicator

Animated spinning loading indicator.

```cpp
LoadingIndicator(std::string name, float radius = 16.0f);

void SetActive(bool active);

// Example
auto spinner = std::make_shared<unigui::LoadingIndicator>("loading");
spinner->SetActive(isLoading);
spinner->Render();
```

---

### GradientText

Renders text with a horizontal color gradient.

```cpp
// Static methods
static void Render(const char* text, ImU32 leftColor, ImU32 rightColor);
static void RenderHex(const char* text, unsigned lr, unsigned lg, unsigned lb,
                      unsigned rr, unsigned rg, unsigned rb);

// Example
unigui::GradientText::Render("Hello World",
    IM_COL32(233, 69, 96, 255), IM_COL32(40, 49, 237, 255));
```

---

## Selection & Lists

### Selectable

A selectable item (like a list entry).

```cpp
Selectable(std::string name, std::string label, bool selected = false);

bool IsSelected() const;
bool WasClicked() const;

// Example
auto sel = std::make_shared<unigui::Selectable>("item1", "Item One");
sel->Render();
```

---

### ListView

Simple string list with single or multi-select.

```cpp
ListView(std::string name, std::vector<std::string> items = {});

int  GetSelected() const;
void SetItems(std::vector<std::string> items);
void SetOnSelect(std::function<void(int)> callback);
void SetMultiSelect(bool on);
std::vector<int> GetSelectedItems() const;

// Example
auto lv = std::make_shared<unigui::ListView>("files",
    std::vector<std::string>{"file1.txt", "file2.txt", "file3.txt"});
lv->SetMultiSelect(true);
lv->SetOnSelect([](int idx) { OpenFile(idx); });
lv->Render();
```

---

### ListBox

Labeled list box with change callback.

```cpp
ListBox(std::string name, std::string label,
        std::vector<std::string> items = {}, int selected = -1);

int  GetSelectedIndex() const;
void SetItems(std::vector<std::string>);
void SetOnChange(std::function<void(int)> cb);

// Example
auto lb = std::make_shared<unigui::ListBox>("lang", "Language",
    std::vector<std::string>{"C++", "Python", "Rust"}, 0);
lb->Render();
```

---

### Table

Simple string table with sorting, resizing, and CSV import/export.

```cpp
Table(std::string name, std::vector<std::string> columns);

void AddRow(std::vector<std::string> row);
void ClearRows();
int  GetSelectedRow() const;
void SetSortable(bool on);
void SetResizable(bool on);
std::string ExportCSV() const;
bool ImportCSV(const std::string& csv);

// Example
auto tbl = std::make_shared<unigui::Table>("data",
    std::vector<std::string>{"Name", "Age", "City"});
tbl->SetSortable(true);
tbl->AddRow({"Alice", "30", "NYC"});
tbl->AddRow({"Bob", "25", "SF"});
tbl->Render();
```

---

### DataTable\<T\>

High-performance templated data table with virtual scrolling, sorting, row coloring,
cell formatting, inline editing, text filtering, and group-aware rendering.

```cpp
DataTable<T>(std::string name, std::vector<ColumnDef> columns);

// Data binding (zero-copy pointer)
void SetDataSource(const std::vector<T>* data);

// Cell rendering
void SetCellFormatter(CellFormatter fmt);   // (int row, int col, const T&) -> string
void SetRowColor(RowColorFn fn);            // (int row, const T&) -> ImU32
void SetCellColor(CellColorFn fn);          // per-cell color
void SetCellBold(CellBoldFn fn);

// Sorting
void SetSortCompare(int col, SortCompare cmp);

// Selection
void SetMultiSelect(bool on);
std::vector<int> GetSelectedRows() const;
void SetOnSelect(SelectFn cb);
void SetOnDoubleClick(DoubleClickFn cb);

// Inline editing
void SetCellEditable(int col, bool editable);
void SetOnCellCommit(CellCommitFn fn);  // (row, col, newValue)

// Filtering
void SetFilterText(const std::string& text);
void SetFilterFn(FilterFn fn);

// Groups
void SetGroups(const std::vector<GroupInfo>& groups);

// Scrolling
void SetVirtualScroll(bool on);
void ScrollToRow(int row);

// Example
struct Person { std::string name; int age; std::string city; };
std::vector<Person> people = {{"Alice", 30, "NYC"}, {"Bob", 25, "SF"}};

auto dt = std::make_shared<unigui::DataTable<Person>>("people_table",
    std::vector<unigui::DataTable<Person>::ColumnDef>{
        {"Name", 150.f}, {"Age", 60.f}, {"City", 120.f}
    });
dt->SetDataSource(&people);
dt->SetCellFormatter([](int row, int col, const Person& p) -> std::string {
    switch (col) {
        case 0: return p.name;
        case 1: return std::to_string(p.age);
        case 2: return p.city;
        default: return "";
    }
});
dt->SetMultiSelect(true);
dt->SetOnSelect([](int row) { std::println("Selected row: {}", row); });
dt->Render();
```

---

### TreeView

Hierarchical tree view with multi-select and custom node renderer.

```cpp
TreeView(std::string name);

void SetRoot(TreeNode root);
void SetMultiSelect(bool on);
std::vector<int> GetSelectedNodes() const;
void SetNodeRenderer(std::function<void(int id, int depth, const TreeNode& node)> fn);

// Example
unigui::TreeNode root;
root.label = "Root";
root.children.push_back({"Child 1", {}, false});
root.children.push_back({"Child 2", {}});

auto tv = std::make_shared<unigui::TreeView>("tree");
tv->SetRoot(root);
tv->Render();
```

---

### VirtualList

Virtual scrolling list for 100k+ entries. Uses `ImGuiListClipper` internally.

```cpp
VirtualList(std::string name, int itemCount = 0);

void SetItemCount(int n);
void SetItemGetter(std::function<std::string(int)> fn);
void SetOnSelect(std::function<void(int)> fn);
int  GetSelected() const;

// Example
auto vl = std::make_shared<unigui::VirtualList>("big_list", 100000);
vl->SetItemGetter([](int i) { return "Item " + std::to_string(i); });
vl->SetOnSelect([](int idx) { SelectItem(idx); });
vl->Render();
```

---

### PropertyGrid

Property editor grid (like Visual Studio Properties window). Supports bool/int/float/string/color/combo.

```cpp
PropertyGrid(std::string name);

void AddProperty(PropertyDef prop);
template<typename T> T GetValue(const std::string& name, T defaultVal = T{}) const;
void SetOnChange(std::function<void(const std::string& name, const PropValue& val)> fn);

// Example
auto pg = std::make_shared<unigui::PropertyGrid>("props");
pg->AddProperty({"visible", "Visible", unigui::PropType::Bool, true});
pg->AddProperty({"opacity", "Opacity", unigui::PropType::Float, 0.75f, {}, 0.0f, 1.0f});
pg->AddProperty({"mode", "Mode", unigui::PropType::Combo,
    std::string{"Dark"}, {"Dark", "Light", "Auto"}});
pg->SetOnChange([](const std::string& name, const unigui::PropValue& val) {
    if (name == "visible")
        SetVisible(std::get<bool>(val));
});
pg->Render();
```

---

## Navigation

### MenuBar

Top-level menu bar with nested menus and items.

```cpp
MenuBar(std::string name);

void SetMenus(std::vector<MenuDef> menus);

// Example
auto mb = std::make_shared<unigui::MenuBar>("main_menu");
mb->SetMenus({
    {"File", {
        {"New", [] { NewFile(); }},
        {"Open", [] { OpenFile(); }},
        {"Exit", [] { Quit(); }}
    }},
    {"Edit", {
        {"Undo", [] { Undo(); }},
        {"Redo", [] { Redo(); }}
    }}
});
mb->Render();
```

---

### Breadcrumb

Breadcrumb navigation bar. Click items to navigate.

```cpp
Breadcrumb(std::string name);

void SetItems(std::vector<std::string> items);
int  GetSelected() const;
void SetOnSelect(std::function<void(int)> cb);

// Example
auto bc = std::make_shared<unigui::Breadcrumb>("path");
bc->SetItems({"Home", "Projects", "MyApp", "src"});
bc->SetOnSelect([](int idx) { NavigateTo(idx); });
bc->Render();
```

---

### ContextMenu

Static right-click context menu. No constructor — use static methods.

```cpp
// Show on current item (call inside any widget):
static void Show(const char* id, std::vector<ContextMenuItem> items);

// Show on window background:
static void ShowWindow(const char* id, std::vector<ContextMenuItem> items);

// Example
// Inside any Render callback:
unigui::ContextMenu::Show("ctx", {
    {"Copy",  [] { Copy(); }},
    {"Paste", [] { Paste(); }},
    {"", nullptr, true},  // separator
    {"Delete", [] { Delete(); }}
});
```

---

### ShortcutManager

Global keyboard shortcut manager. Register shortcuts, call `Process()` each frame.

```cpp
void Register(ImGuiKey key, bool ctrl, std::function<void()> action, std::string desc = "");
void Process();

// Example
unigui::ShortcutManager shortcuts;
shortcuts.Register(ImGuiKey_S, true, [] { SaveFile(); }, "Ctrl+S: Save");
shortcuts.Register(ImGuiKey_Z, true, [] { Undo(); }, "Ctrl+Z: Undo");
// In render loop:
shortcuts.Process();
```

---

### Wizard

Multi-step wizard with Next/Previous navigation.

```cpp
Wizard(std::string name, std::string title = "Wizard");

void AddStep(std::string name, std::string title, std::function<void()> renderFn);
int  GetCurrentStep() const;
void Next(); void Previous(); void GoTo(int step);
void SetOnFinish(std::function<void()> fn);

// Example
auto wiz = std::make_shared<unigui::Wizard>("setup", "Setup Wizard");
wiz->AddStep("welcome", "Welcome", [&] {
    unigui::Label{"welcome_lbl", "Welcome to setup!"}.Render();
});
wiz->AddStep("config", "Configuration", [&] {
    unigui::CheckBox{"opt_auto", "Auto-start"}.Render();
});
wiz->SetOnFinish([] { CompleteSetup(); });
wiz->Render();
```

---

### ToolBar

Horizontal toolbar with labeled action buttons.

```cpp
ToolBar(std::string name);

void SetItems(std::vector<ToolBarItem> items);

// Example
auto tb = std::make_shared<unigui::ToolBar>("toolbar");
tb->SetItems({
    {"New",  [] { NewFile(); }, true},
    {"Open", [] { OpenFile(); }, true},
    {"Save", [] { SaveFile(); }, false}  // disabled
});
tb->Render();
```

---

### StatusBar

Bottom status bar with text.

```cpp
StatusBar(std::string name, std::string text = "");

void SetText(std::string text);

// Example
auto sb = std::make_shared<unigui::StatusBar>("status", "Ready");
// Update during render loop:
sb->SetText("Processing...");
sb->Render();
```

---

## Feedback

### Tooltip

Simple tooltip. Static helper — hover over last-rendered widget.

```cpp
static void Show(std::string text);

// Example
unigui::Button{"btn_help", "?"}.Render();
if (ImGui::IsItemHovered())
    unigui::Tooltip::Show("Click for help");
```

> Alternatively, any Widget can have a tooltip via `widget->SetTooltip("text")`.

---

### Notification

In-app notification stack displayed in the top-right corner.

```cpp
Notification(std::string name);

void Show(std::string title, std::string msg, float duration = 3.0f);

// Example
auto notif = std::make_shared<unigui::Notification>("notifications");
notif->Show("Saved", "File saved successfully!", 3.0f);
notif->Render();
```

---

### Toast

Singleton popup notification system. Call static methods from anywhere.

```cpp
static Toast& Instance();
static void Info(std::string msg);
static void Success(std::string msg);
static void Warn(std::string msg);
static void Error(std::string msg);

// Example
// One-time: add Toast to render loop
auto& toast = unigui::Toast::Instance();
toast.Render();

// From anywhere:
unigui::Toast::Success("Export complete!");
unigui::Toast::Error("Connection failed!");
```

---

### Badge

Small notification badge (dot, count, or label). Call `Render()` after the parent widget.

```cpp
Badge(const std::string& label = "");

void SetVariant(Variant v);  // Dot, Count, Label
void SetCount(int n);
void SetColor(ImU32 color);

// Example
unigui::Button{"inbox", "Inbox"}.Render();
unigui::Badge badge{"5"};
badge.SetVariant(unigui::Badge::Count);
badge.SetCount(5);
badge.Render();  // renders overlaid on the button
```

---

### Tag

Colored tag/chip widget, optionally removable.

```cpp
Tag(std::string name, std::string text, std::array<float, 3> color = {0.2f, 0.5f, 1.0f});

void SetRemovable(bool r);
bool RemoveClicked() const;

// Example
auto tag = std::make_shared<unigui::Tag>("tag_cpp", "C++", {0.2f, 0.5f, 1.0f});
tag->SetRemovable(true);
tag->Render();
if (tag->RemoveClicked())
    RemoveTag("C++");
```

---

### Dialog

Modal/popup dialog with OK/Cancel buttons.

```cpp
Dialog(std::string name, std::string title, std::string message);

void Open(); void Close();
void SetButtons(std::string okText, std::string cancelText = "");
void SetOnOk(std::function<void()> callback);
void SetOnCancel(std::function<void()> callback);
bool WasOkClicked() const;

// Example
auto dlg = std::make_shared<unigui::Dialog>("confirm", "Delete", "Are you sure?");
dlg->SetButtons("Delete", "Cancel");
dlg->SetOnOk([] { PerformDelete(); });
dlg->Open();
dlg->Render();
```

---

## Layout & Utilities

### Form

Declarative form with automatic layout and validation.

```cpp
Form(std::string name, std::string title);

void AddTextField(std::string name, std::string label, bool required = false);
void AddCheckbox(std::string name, std::string label);
void AddComboField(std::string name, std::string label, std::vector<std::string> options);
void AddSliderField(std::string name, std::string label, float min = 0, float max = 100);
void AddNumberField(std::string name, std::string label, int min = 0, int max = 100);
std::string GetFieldValue(const std::string& name) const;
std::vector<FormError> Validate() const;
void SetOnSubmit(std::function<void()> callback);
std::string Serialize() const;
bool Deserialize(const std::string& json);

// Example
auto form = std::make_shared<unigui::Form>("login_form", "Login");
form->AddTextField("username", "Username", true);
form->AddTextField("password", "Password", true);
form->SetOnSubmit([&] {
    auto errors = form->Validate();
    if (errors.empty())
        Login(form->GetFieldValue("username"), form->GetFieldValue("password"));
});
form->Render();
```

---

### Layout (namespace)

Declarative layout helpers: `HBox`, `VBox`, `BeginHBox/EndHBox`, `BeginHSplit/NextHSplit/EndHSplit`.

```cpp
namespace unigui::Layout {
    void HBox(std::initializer_list<std::function<void()>> children);
    void VBox(std::initializer_list<std::function<void()>> children);
    void BeginHBox(); void EndHBox();
    void BeginHSplit(float leftRatio = 0.5f);
    void NextHSplit(); void EndHSplit();
}

// Example
unigui::Layout::HBox({
    [] { unigui::Button{"ok", "OK"}.Render(); },
    [] { unigui::Button{"cancel", "Cancel"}.Render(); }
});
```

---

### Animate (namespace)

Simple animation helpers: `FadeIn`, `SlideIn`, `Lerp`, `FadeScope`.

```cpp
namespace unigui::Animate {
    float FadeIn(float duration = 0.3f);
    float SlideIn(float duration = 0.3f, float fromOffset = -50.0f);
    float Lerp(float current, float target, float speed = 0.1f);
    struct FadeScope { /* RAII alpha push/pop */ };
}

// Example
{
    unigui::Animate::FadeScope fade{1.0f, 0.5f};  // fade in over 500ms
    unigui::Label{"fade_text", "Fading in..."}.Render();
}  // alpha restored here
```

---

### Clipboard (namespace)

Wrappers for ImGui clipboard.

```cpp
namespace unigui::Clipboard {
    void Copy(const std::string& text);
    std::string Paste();
}

// Example
unigui::Clipboard::Copy("Hello, world!");
auto text = unigui::Clipboard::Paste();
```

---

### DragDrop (namespace)

Generic drag-and-drop helpers.

```cpp
template<typename T>
bool BeginDragSource(const char* type, const T& data);
template<typename T>
const T* AcceptDragDrop(const char* type);

// Example
struct Item { int id; std::string name; };
Item myItem{42, "Widget"};

if (unigui::BeginDragSource("MY_ITEM", myItem)) {
    // source is being dragged
}
if (auto* dropped = unigui::AcceptDragDrop<Item>("MY_ITEM")) {
    std::println("Received: {}", dropped->name);
}
```

---

## FX

### Shimmer

Animated loading placeholder with gradient sweep (blocks, circles).

```cpp
Shimmer();

void AddBlock(float width, float height, float x = 0, float y = 0);
void AddCircle(float radius, float x = 0, float y = 0);
void Start(); void Stop();
void SetSpeed(float s);
void Render();

// Example
unigui::Shimmer shimmer;
shimmer.AddBlock(200, 16);
shimmer.AddBlock(150, 16, 0, 24);
shimmer.Start();
shimmer.Render();
```

---

### SkeletonScreen

Static loading skeleton with optional built-in shimmer animation.

```cpp
SkeletonScreen();

void AddBlock(float width, float height, float x = 0, float y = 0);
void AddLine(float width, float x = 0, float y = 0);
void AddCircle(float radius, float x = 0, float y = 0);
void SetShimmer(bool enable, float speed = 1.2f);
static SkeletonScreen FromSize(float w, float h, int lineCount = 4);
void Render();

// Example
auto skel = unigui::SkeletonScreen::FromSize(300, 100, 3);
skel.SetShimmer(true);
skel.Render();
```

---

### TrayIcon

Windows system tray icon with context menu and notifications.

```cpp
TrayIcon(std::string name, std::string title = "UniGUI", int iconId = 0);

bool Show(); void Hide();
void SetMenu(std::vector<TrayMenuItem> items);
void ShowNotification(std::string title, std::string msg, NotifyType type = Info);

// Example
#ifdef _WIN32
auto tray = std::make_shared<unigui::TrayIcon>("app_tray", "MyApp", 101);
tray->SetMenu({
    {"Show", [] { ShowWindow(); }},
    {"", nullptr, true},  // separator
    {"Exit", [] { QuitApp(); }}
});
tray->Show();
#endif
```

---

### TimeSeriesChart

Real-time time-series chart using implot. Supports multiple series, sliding window,
crosshair, legend, pan/zoom.

```cpp
TimeSeriesChart(std::string name);

int  AddSeries(TimeSeriesDef def);
void AppendPoint(int seriesId, float value, double timestamp = -1.0);
void SetSlidingWindow(int maxPoints);
void SetYAxisAutoFit(bool on);
void SetCrosshairEnabled(bool on);
void SetLegendEnabled(bool on);
void SetPanEnabled(bool on);
void SetZoomEnabled(bool on);

// Example
auto chart = std::make_shared<unigui::TimeSeriesChart>("perf_chart");
int seriesId = chart->AddSeries({"FPS", IM_COL32(34, 197, 94, 255)});
chart->SetSlidingWindow(300);
chart->SetCrosshairEnabled(true);

// In render loop:
chart->AppendPoint(seriesId, currentFPS);
chart->Render();
```

---

## Widget Base Class

All widgets (except a few standalone types like Card, Badge, Shimmer, SkeletonScreen,
GradientText) inherit from `unigui::Widget`:

```cpp
class Widget {
public:
    explicit Widget(std::string name);
    virtual void Render() = 0;

    // Visibility
    void Show(); void Hide();
    bool IsVisible() const;

    // Identity
    const std::string& GetName() const;
    ImGuiID GetID() const;

    // Tooltip
    void SetTooltip(std::string t);

    // Focus
    void SetFocused();
    bool IsFocused() const;
    static void SetNextFocused();

    // Size constraints
    void SetMinSize(float w, float h);
    void SetMaxSize(float w, float h);

    // Shadow (v3.0)
    void SetShadow(bool enable, float radius = 4.f,
                   float offX = 2.f, float offY = 2.f);

    // Accessibility
    void SetAccessibleName(std::string n);
    void SetAccessibleDescription(std::string d);
};
```

---

## Complete Application Template

```cpp
#include <unigui/unigui.h>
#include <memory>
#include <print>

int main() {
    using namespace unigui;

    AppConfig cfg;
    cfg.title  = "UniGUI Demo";
    cfg.width  = 1280;
    cfg.height = 720;

    Init(cfg);

    // ── Create widgets ─────────────────────────────────────────────
    auto window   = std::make_shared<Window>("main", "UniGUI Demo");
    auto panel    = std::make_shared<Panel>("main_panel", "Dashboard");
    auto split    = std::make_shared<Splitter>("layout", Splitter::Horizontal, 0.3f);
    auto btn      = std::make_shared<Button>("btn_action", "Click Me");
    auto lbl      = std::make_shared<Label>("lbl_status", "Ready");
    auto progress = std::make_shared<ProgressBar>("pbar", 0.0f);
    auto& toast   = Toast::Instance();

    btn->SetColorVariant(Button::Primary);
    split->SetContentA([&] {
        // Sidebar
        lbl->Render();
        btn->Render();
        if (btn->WasClicked()) {
            Toast::Success("Button clicked!");
            progress->SetFraction(0.75f);
        }
        progress->Render();
    });
    split->SetContentB([&] {
        // Main area
        static auto cb = std::make_shared<CheckBox>("opt_debug", "Debug Mode", false);
        cb->Render();
    });

    panel->SetContentCallback([&] { split->Render(); });
    window->AddPanel(panel);

    // Shortcuts
    ShortcutManager shortcuts;
    shortcuts.Register(ImGuiKey_Q, true, [] { /* quit */ }, "Ctrl+Q: Quit");

    // ── Main loop ──────────────────────────────────────────────────
    Run([&] {
        shortcuts.Process();
        window->Render();
        toast.Render();
    });

    Shutdown();
    return 0;
}
```
