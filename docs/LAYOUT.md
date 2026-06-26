# Layout

The complete reference for UniGUI's layout system: a CSS-flexbox-style solver,
the ImGui-facing helpers that apply it, the simpler box/split helpers, and the
on-disk layout store.

UniGUI splits layout into two cleanly separated halves, distinguished by
**capitalization**:

| Namespace | Header | What it is | ImGui? | Allocates? |
|-----------|--------|------------|--------|-----------|
| `unigui::layout` (lowercase) | `<unigui/core/flex_layout.h>` | The **pure math**: a headless flexbox solver (`SolveFlex`, `SolveFlexWrap`) | No | Only the result vector |
| `unigui::Layout` (Capital) | `<unigui/widgets/layout.h>` | The **ImGui-facing helpers**: `FlexRow`, `HBox`/`VBox`, `BeginHSplit`/… that *apply* the math through ImGui child regions | Yes | — |

> **Why the split matters.** `unigui::layout::SolveFlex` does no drawing, touches
> no ImGui global state, and allocates nothing beyond the vector it returns. That
> makes it fully unit-testable in a headless process (the test suite exercises it
> with no GL context). The capital-`Layout` helpers are the thin adapter that
> feeds real ImGui geometry (`GetContentRegionAvail`) into the solver and renders
> each resolved span inside a child region.

There is also `unigui::LayoutStore` (`<unigui/core/layout_store.h>`), an
unrelated tiny key/value file store for **persisting** layout state (and other
simple preferences) across runs.

---

## Part 1 — `unigui::layout`: the pure flexbox solver

`#include <unigui/core/flex_layout.h>`

This header is the computational core. Given a container length and a list of
flex items (basis + grow/shrink + min/max clamps), it resolves each item's
main-axis size and position so a UI can reflow without hand-computed widths.
Cross-axis placement (`align-items`) and line wrapping (`flex-wrap`) are layered
on top.

### Data types

#### `FlexItem` — one participant in a flex line

All sizes are in pixels.

```cpp
struct FlexItem {
    float basis = 0.0f;      // preferred main-axis size (flex-basis)
    float grow = 0.0f;       // share of leftover space to absorb (flex-grow)
    float shrink = 1.0f;     // share of overflow to give up (flex-shrink)
    float minSize = 0.0f;    // lower clamp on the resolved main-axis size
    float maxSize = FLT_MAX; // upper clamp on the resolved main-axis size
    float crossSize = 0.0f;  // preferred cross-axis size (used by align != Stretch)
};
```

Notes that bite if ignored:

- **`grow` defaults to `0`** (an item does *not* grow unless asked), matching CSS
  `flex-grow: 0`.
- **`shrink` defaults to `1`** (an item *does* give up space under overflow),
  matching CSS `flex-shrink: 1`. Set `shrink = 0` to pin an item at its basis even
  when the container overflows.
- `crossSize` is the item's preferred cross-axis (height in a row) extent. It is
  consumed by `align != Stretch`; under `Stretch` the container's cross size wins
  (when one is supplied).

#### `FlexParams` — container-level inputs

```cpp
struct FlexParams {
    float containerSize = 0.0f;               // available main-axis length
    float crossSize = 0.0f;                   // available cross-axis length (0 = unknown)
    float gap = 0.0f;                         // fixed gap between adjacent items
    FlexJustify justify = FlexJustify::Start; // leftover main-axis distribution
    FlexAlign align = FlexAlign::Start;       // cross-axis placement
};
```

`crossSize = 0` means "cross extent unknown"; in that case the cross fields of the
output fall back to each item's own `crossSize` (and `Stretch` likewise uses the
item's `crossSize`, since there is no container cross extent to stretch into).

#### `FlexSpan` — resolved geometry for one item

```cpp
struct FlexSpan {
    float offset = 0.0f;      // main-axis position
    float size = 0.0f;        // main-axis size
    float crossOffset = 0.0f; // cross-axis position (per align mode)
    float crossSize = 0.0f;   // cross-axis size (per align mode)
};
```

`offset`/`size` are always filled. `crossOffset`/`crossSize` are filled per the
`align` mode. Under `Start`/`Center`/`End` the span's `crossSize` is the item's own
`crossSize` (so it is `0` only when the item itself requested none); under
`Stretch` it is the container's `crossSize` when positive, else the item's own.
`crossOffset` clamps to `0` whenever no container `crossSize` is supplied.

#### `FlexJustify` — main-axis distribution (CSS `justify-content`)

```cpp
enum class FlexJustify { Start, End, Center, SpaceBetween, SpaceAround, SpaceEvenly };
```

#### `FlexAlign` — cross-axis placement (CSS `align-items`)

```cpp
enum class FlexAlign { Start, Center, End, Stretch };
```

### `SolveFlex` — resolve one line along the main axis

```cpp
std::vector<FlexSpan> SolveFlex(const std::vector<FlexItem>& items,
                                const FlexParams& params);
```

Returns one `FlexSpan` per item, **in input order**. Empty input yields an empty
vector. The algorithm runs in four phases.

**1. Seed.** Each item starts at its basis, clamped to `[minSize, maxSize]`.

**2. Grow or shrink (whichever the free space demands).** Free space is
`containerSize − (Σ sizes + totalGap)`, where `totalGap = gap × (n − 1)`.

- **`free > 0` → GROW.** Leftover space is handed out in proportion to each item's
  `grow` weight. This uses a **freeze-and-redistribute** pass: an item that would
  reach or exceed its `maxSize` is set to `maxSize`, *frozen*, and its unabsorbed
  surplus is redistributed among the still-growable items on the next iteration.
  The loop is bounded (`guard <= n`) and stops early once the free space is fully
  absorbed or no item froze. Items with `grow <= 0` never grow.
- **`free < 0` → SHRINK.** Overflow is removed in proportion to the **scaled
  shrink factor** `shrink × basis` (CSS's scaled-flex-shrink, so wider items give
  up more). The same freeze-and-redistribute applies, but freezing at `minSize`:
  an item floored at its min stops shrinking and its remaining overflow is taken
  from the others. Items with `shrink × basis <= 0` never shrink.

**3. Justify.** Whatever main-axis space remains after sizing (`leftover`, clamped
to `>= 0`) is distributed per `justify`:

| `FlexJustify` | Leading offset (`cursor`) | Spacing between items (`between`) |
|---------------|---------------------------|-----------------------------------|
| `Start` | `0` | `gap` |
| `End` | `leftover` | `gap` |
| `Center` | `leftover / 2` | `gap` |
| `SpaceBetween` | `0` | `gap + leftover / (n − 1)` |
| `SpaceAround` | `leftover / (2n)` | `gap + leftover / n` |
| `SpaceEvenly` | `leftover / (n + 1)` | `gap + leftover / (n + 1)` |

The `gap` is always preserved *in addition to* the distributed space (the table's
`between` adds to `gap`, it doesn't replace it). When items already fill the
container (`leftover == 0`), every mode degenerates to start-packing with the gap.
For `SpaceBetween` with a single item, the `between` term is not applied (the
`n > 1` guard), so it falls back to start-packing.

**4. Cross-axis placement.** For each item, per `align`:

| `FlexAlign` | `crossSize` of span | `crossOffset` of span |
|-------------|---------------------|------------------------|
| `Start` | item's `crossSize` | `0` |
| `Center` | item's `crossSize` | `max(0, (params.crossSize − cs) / 2)` |
| `End` | item's `crossSize` | `max(0, params.crossSize − cs)` |
| `Stretch` | `params.crossSize` if `> 0`, else item's `crossSize` | `0` |

#### Worked example — grow with weighted split

```cpp
#include <unigui/core/flex_layout.h>
using namespace unigui::layout;

// 200px of free space (400 − 100 − 100), split 1:2 between two items.
auto spans = SolveFlex({{.basis = 100.0f, .grow = 1.0f},
                        {.basis = 100.0f, .grow = 2.0f}},
                       {.containerSize = 400.0f});

// spans[0] = { offset 0.0,    size 166.667 }   // 100 + 1/3 of 200
// spans[1] = { offset 166.667, size 233.333 }  // 100 + 2/3 of 200
// The two sizes sum to exactly 400 (the container is fully filled).
```

#### Worked example — grow with `maxSize` redistribution

```cpp
// item0 wants to grow to 200 but is capped at 120; its 80px surplus flows to item1.
auto spans = SolveFlex(
    {{.basis = 100.0f, .grow = 1.0f, .maxSize = 120.0f},
     {.basis = 100.0f, .grow = 1.0f}},
    {.containerSize = 400.0f});
// spans[0].size == 120.0   (frozen at its max)
// spans[1].size == 280.0   (absorbed the rest)
```

#### Worked example — shrink by scaled factor

```cpp
// 40px overflow (200 content into a 160 container), split by shrink*basis = 100:300.
auto spans = SolveFlex(
    {{.basis = 100.0f, .shrink = 1.0f},
     {.basis = 100.0f, .shrink = 3.0f}},
    {.containerSize = 160.0f});
// spans[0].size == 90.0   (gave up 10 = 1/4 of overflow)
// spans[1].size == 70.0   (gave up 30 = 3/4 of overflow)
```

#### Worked example — gap and justify

```cpp
// No grow: two 100px items in 300px leave 100px leftover, centered.
auto spans = SolveFlex({{.basis = 100.0f}, {.basis = 100.0f}},
                       {.containerSize = 300.0f, .justify = FlexJustify::Center});
// spans[0].offset == 50.0, spans[1].offset == 150.0

// SpaceBetween pushes the whole 100px between the two children:
auto sb = SolveFlex({{.basis = 100.0f}, {.basis = 100.0f}},
                    {.containerSize = 300.0f, .justify = FlexJustify::SpaceBetween});
// sb[0].offset == 0.0, sb[1].offset == 200.0
```

#### Worked example — cross-axis align

```cpp
// A 40px-tall item centered within a 100px-tall line:
auto spans = SolveFlex(
    {{.basis = 50.0f, .crossSize = 40.0f}},
    {.containerSize = 50.0f, .crossSize = 100.0f, .align = FlexAlign::Center});
// spans[0].crossOffset == 30.0   // (100 - 40) / 2
// spans[0].crossSize   == 40.0

// Under Stretch the item fills the line height:
auto st = SolveFlex(
    {{.basis = 50.0f, .crossSize = 40.0f}},
    {.containerSize = 50.0f, .crossSize = 100.0f, .align = FlexAlign::Stretch});
// st[0].crossOffset == 0.0, st[0].crossSize == 100.0
```

### `SolveFlexWrap` — break into lines and stack on the cross axis

```cpp
std::vector<std::vector<FlexSpan>> SolveFlexWrap(const std::vector<FlexItem>& items,
                                                 const FlexParams& params,
                                                 float lineHeight = 0.0f);
```

This implements CSS `flex-wrap: wrap`. It returns one inner vector per line (item
order preserved within each line), with the outer vector ordered top-to-bottom.
Empty input yields an empty outer vector.

**1. Greedy line-breaking.** Items are walked in order, accumulating each item's
`basis` plus the inter-item `gap`. When adding the next item would push the running
main-axis total past `containerSize`, that item starts a **fresh line**. An item
whose own `basis` already exceeds `containerSize` occupies a line **by itself**
(and neighbours wrap around it).

**2. Per-line solve.** Each line's sub-vector is handed to `SolveFlex` with the
**same** `containerSize` / `gap` / `justify` / `align`. Grow/shrink/justify are
therefore resolved *within that line only* — space never flows across the wrap
boundary, exactly as in CSS.

**3. Cross-axis stacking.** Lines stack top-to-bottom: each line's `crossOffset` is
the sum of the **effective heights** of the lines above it. The effective line
height is:

- the caller-supplied `lineHeight` when it is `> 0` (the common case — a fixed row
  height), otherwise
- the tallest `FlexItem::crossSize` among that line's *own* items (auto-sized rows
  pack against their own content, like CSS `align-content` with auto-sized lines).

> **Cumulative stacking semantics.** Because each line accumulates the height of the
> lines *above* it (a running `crossCursor`) rather than multiplying a line index by
> a single height, lines of *differing* heights stack correctly. Mixing tall and
> short rows produces the right cumulative offsets; using `l × thisLineHeight` would
> be wrong, which is precisely why the implementation carries a running cursor.

#### Worked example — wrapping with a fixed line height

```cpp
// Four 100px items in a 250px container, 10px gap.
// Greedy packing fits two per line: 100 + 10 + 100 = 210 <= 250;
// a third would be 320 > 250, so it wraps.
const std::vector<FlexItem> items = {
    {.basis = 100.0f, .crossSize = 30.0f},
    {.basis = 100.0f, .crossSize = 30.0f},
    {.basis = 100.0f, .crossSize = 30.0f},
    {.basis = 100.0f, .crossSize = 30.0f},
};
auto lines = SolveFlexWrap(items, {.containerSize = 250.0f, .gap = 10.0f},
                           /*lineHeight=*/50.0f);
// lines.size() == 2, each line has 2 items.
// line 0: offsets 0 and 110, crossOffset 0.
// line 1: offsets 0 and 110, crossOffset 50  (shifted down by one lineHeight).
```

#### Worked example — auto line height (per-line max crossSize)

```cpp
// No explicit lineHeight: each line is as tall as its tallest item.
// Container 250, no gap. Items 0 and 1 pack (200 <= 250); item 2 wraps.
const std::vector<FlexItem> items = {
    {.basis = 100.0f, .crossSize = 20.0f},
    {.basis = 100.0f, .crossSize = 40.0f},
    {.basis = 100.0f, .crossSize = 25.0f},
};
auto lines = SolveFlexWrap(items, {.containerSize = 250.0f});
// lines.size() == 2; line 0 has 2 items, line 1 has 1 item.
// lines[1][0].crossOffset == 40.0  // line 0's tallest crossSize (40), not item-2's 25
```

---

## Part 2 — `unigui::Layout`: the ImGui-facing helpers

`#include <unigui/widgets/layout.h>`

These free functions and small classes live in the capital-`Layout` namespace and
*apply* the solver (or simpler ImGui flow) inside a live frame. Everything in this
header is `inline` (the helpers are defined in the header itself).

### `FlexRow` — a flexbox row through ImGui child regions

```cpp
struct FlexChild {
    layout::FlexItem item;       // its flex sizing (basis/grow/shrink/min/max[/crossSize])
    std::function<void()> render; // drawn inside a child region of the resolved width
};

struct FlexRowOptions {
    float width  = 0.0f; // main-axis length; <= 0 uses the available content width
    float height = 0.0f; // child height;     <= 0 fills the remaining vertical space
    float gap    = 0.0f; // fixed gap between adjacent children
    layout::FlexJustify justify = layout::FlexJustify::Start;
    layout::FlexAlign   align   = layout::FlexAlign::Start;
};

void FlexRow(const char* id, const std::vector<FlexChild>& children,
             const FlexRowOptions& opt = {});
```

`FlexRow` lays its children out in a horizontal flex line, sizing each via
`SolveFlex` and drawing it inside an ImGui child region of its resolved width.
`FlexRowOptions` is designated-initializer friendly:

```cpp
FlexRow("toolbar", kids, {.gap = 8.f, .justify = layout::FlexJustify::SpaceBetween});
```

#### Behaviour and guard rails

The implementation has several deliberate guards — understanding them prevents
surprises:

- **Empty children → no-op.** `FlexRow` with an empty vector returns immediately
  (and never throws).
- **Input sanitization.** `gap`, `height`, and `width` are coerced with `x > 0 ? x
  : 0` ternaries, which also reject `NaN` (since `NaN > 0` is false).
- **`width <= 0` uses the available content width** (`GetContentRegionAvail().x`).
  **If there is no horizontal room** (the resolved container is not `> 0`), the row
  renders **nothing** — the no-room guard.
- **Zero-width omission.** A child the solver collapses to `~0` width (`size <=
  0.5f`) is **skipped entirely** — its `render` is not even called. This guards the
  ImGui trap where `BeginChild(width == 0)` means "fill the remaining space," which
  would make a collapsed child balloon over its neighbours.
- **ID safety.** The per-child region IDs are scoped under `PushID(id)` /
  `PopID()`, so multiple `FlexRow`s can coexist in one window without ID
  collisions. Each child additionally gets a unique `##fc<i>` region name. (This is
  the project-wide ID-safety rule from `CLAUDE.md`.) The child regions themselves
  are created with `ImGuiChildFlags_None` (no border).
- **Reserve-footprint behaviour.** When `height > 0`, the row first reserves its
  full rect with one invisible `ImGui::Dummy(width, height)`, then positions each
  child absolutely on top, then resumes the cursor exactly at the row's bottom
  (`start.y + height`) so following content flows underneath with no extra spacing.
  The Dummy is what keeps that resume **in-bounds** — without it the absolute
  `SetCursorPos` would over-extend the window and trip ImGui's "SetCursorPos
  extended bounds without an item" assertion. Callers wanting a gap below the row
  add their own `Spacing()`.
- **`height <= 0` fills the remaining vertical space.** In that mode every child
  fills the rest of the window, so a `FlexRow` with `height <= 0` **must be the last
  element in its container** — it forfeits the cursor flow for anything after it.
  Pass a positive `height` to lay further content out below the row.

#### Cross-axis alignment

`opt.align` (CSS `align-items`) positions each child vertically within the row.
The row's cross-axis extent is `opt.height` (or the remaining content height,
`GetContentRegionAvail().y`, when `opt.height <= 0`). A child's effective height is
its **solved cross size** when it sets `FlexChild::item.crossSize` (or under
`Stretch`), otherwise it falls back to `opt.height`.

The **default** — `align = Start` with no per-child `crossSize` — preserves the
legacy uniform-height behaviour: every child simply gets `opt.height`.

#### Worked example — a responsive toolbar

A toolbar with a fixed-width left button group, a title that grows to absorb slack,
and a fixed-width right button — all reflowing as the window resizes:

```cpp
#include <unigui/widgets/layout.h>
#include <unigui/im/im.h>   // unigui::im immediate-mode controls
using namespace unigui;

void DrawToolbar() {
    Layout::FlexRow(
        "main_toolbar",
        {
            // Left: fixed 120px button cluster, never grows or shrinks.
            {{.basis = 120.0f, .grow = 0.0f, .shrink = 0.0f},
             [] {
                 im::Button("New");
                 ImGui::SameLine();
                 im::Button("Open");
             }},
            // Center: title field that absorbs all leftover width, but won't
            // shrink below 80px.
            {{.basis = 0.0f, .grow = 1.0f, .minSize = 80.0f},
             [] { im::Text("Untitled document"); }},
            // Right: fixed 90px action, pinned to its size.
            {{.basis = 90.0f, .grow = 0.0f, .shrink = 0.0f},
             [] { im::Button("Share"); }},
        },
        {.height = ImGui::GetFrameHeight(), .gap = 8.0f});
}
```

Because `width` is omitted it uses the live content width, so the center child
grows and shrinks as the window resizes. With `.height` positive, content drawn
after the call flows directly below the toolbar.

#### Worked example — space-between footer

```cpp
Layout::FlexRow(
    "footer",
    {
        {{.basis = 100.0f}, [] { im::Text("Ready"); }},
        {{.basis = 100.0f}, [] { im::Button("Cancel"); }},
    },
    {.height = ImGui::GetFrameHeight(),
     .justify = layout::FlexJustify::SpaceBetween});
// The two children are pushed to the row's left and right edges.
```

### `HBox` / `VBox` free functions

```cpp
void HBox(std::initializer_list<std::function<void()>> children);
void VBox(std::initializer_list<std::function<void()>> children);
```

The simplest declarative helpers. `HBox` renders its child callbacks side by side,
inserting `ImGui::SameLine()` between adjacent children (but not after the last).
`VBox` simply renders the children in order (ImGui's natural top-to-bottom flow).

```cpp
Layout::HBox({
    [] { im::Button("A"); },
    [] { im::Button("B"); },
    [] { im::Button("C"); },
});  // A B C on one line
```

### `BeginHBox` / `EndHBox`

```cpp
void BeginHBox();  // ImGui::BeginGroup()
void EndHBox();    // ImGui::EndGroup()
```

Thin wrappers around an ImGui group, useful for treating a span of widgets as a
single layout unit (e.g. for `SameLine` after the group).

### `BeginHSplit` / `NextHSplit` / `EndHSplit` — a two-pane ratio split

```cpp
void BeginHSplit(float leftRatio = 0.5f); // ratio = 0.5 → 50/50
void NextHSplit();
void EndHSplit();
```

A bordered two-pane horizontal split. `BeginHSplit` opens a left child sized to
`leftRatio` of the available width; `NextHSplit` closes it, emits a `SameLine()`,
and opens a right child filling the remainder; `EndHSplit` closes that. Both panes
get `ImGuiChildFlags_Borders`.

```cpp
Layout::BeginHSplit(0.3f);      // left pane = 30% width
    im::Text("Sidebar");
Layout::NextHSplit();           // right pane = remaining 70%
    im::Text("Main content");
Layout::EndHSplit();
```

> For a *resizable*, *serializable* multi-pane splitter, use the retained
> `MultiSplitter` widget (`<unigui/widgets/multisplitter.h>`) instead — it provides
> draggable handles and `SerializeLayout()`/`RestoreLayout()`, which pair with
> `LayoutStore` (Part 3). `BeginHSplit` is for a quick fixed-ratio split.

### RAII `HBox` / `VBox` classes

In addition to the free functions, `unigui::HBox` and `unigui::VBox` (note:
**outside** the `Layout` namespace, directly in `unigui`) are RAII guards that push
an `ItemSpacing` style var for their lifetime.

```cpp
class HBox {
public:
    HBox(float spacing = -1.0f);   // spacing < 0 → leave ItemSpacing unchanged
    ~HBox();                        // pops the style var if it was pushed
    static void VSeparator();       // SameLine() then a "|" glyph
};

class VBox {
public:
    VBox(float spacing = -1.0f);
    ~VBox();
};
```

- `unigui::HBox(spacing)` pushes `ImGuiStyleVar_ItemSpacing = (spacing, 0)` while in
  scope (only when `spacing >= 0`); `~HBox` pops it. `VSeparator()` is a static
  helper that draws a vertical `|` divider after a `SameLine()`.
- `unigui::VBox(spacing)` pushes `ImGuiStyleVar_ItemSpacing = (0, spacing)` for the
  vertical gap between stacked children (again only when `spacing >= 0`).

```cpp
{
    unigui::HBox h(4.0f);          // 4px horizontal item spacing in this scope
    im::Button("Save");
    unigui::HBox::VSeparator();    // " | "
    im::Button("Discard");
}                                  // spacing restored here
```

> **Disambiguation.** `unigui::Layout::HBox(...)` (a *function* taking child
> callbacks) and `unigui::HBox` (a *class* RAII guard) are different APIs in
> different scopes. Pick by intent: the function for a quick inline row of
> callbacks, the class to override item spacing across a block of normal ImGui
> calls. The same split applies to `unigui::Layout::VBox` vs `unigui::VBox`.

### Relationship to the DSL `Flex` node

The declarative DSL (`<unigui/dsl/dsl.h>`) exposes a `Flex` container node that is
**rendered through `Layout::FlexRow`** at a fixed one-line height
(`ImGui::GetFrameHeightWithSpacing()`):

```cpp
namespace unigui::dsl {
using FlexJustify = layout::FlexJustify;   // re-export of the same enum

NodePtr Flex(std::vector<NodePtr> children, float gap = 0.0f,
             FlexJustify justify = FlexJustify::Start);
NodePtr Flex(std::vector<NodePtr> children, std::vector<float> weights,
             float gap = 0.0f, FlexJustify justify = FlexJustify::Start);
}
```

Children share the available width by their flex-grow weight (default `1` =
equal split). The `weights` overload sets per-child grow weights (a shorter or
empty vector falls back to weight `1` for the missing entries). The DSL `Flex` is
single-line only (no wrap), uses one fixed row height, and does not expose
cross-axis alignment — for those, drop to `Layout::FlexRow` directly. The DSL is
documented separately; it is mentioned here only to show that the same solver
underlies every layer.

---

## Part 3 — `unigui::LayoutStore`: persisting layout state

`#include <unigui/core/layout_store.h>`

`LayoutStore` is a tiny named string-value store for persisting UI layout state
(and other simple preferences) across runs. It is **header-only** and
**dependency-free** (just `<fstream>`/`<map>`/`<string>`), so it is unit-testable
against a temp file with no GUI/ImGui context.

Keys map to opaque values — typically the output of a widget's serializer such as
`MultiSplitter::SerializeLayout()`, a theme-preset name, or a locale tag. It
persists as plain `name=value` lines and, in keeping with the project's
"no throwing parsers" rule, **never throws on malformed input** (it simply skips
bad lines).

### API

```cpp
class LayoutStore {
public:
    void Set(const std::string& name, const std::string& value);
    std::string Get(const std::string& name) const;   // "" if absent
    bool Has(const std::string& name) const;
    void Remove(const std::string& name);
    void Clear();
    std::size_t Size() const;
    const std::map<std::string, std::string>& Entries() const;

    bool Save(const std::string& path) const;  // false if the file can't be opened
    bool Load(const std::string& path);        // false only if it can't be opened
};
```

Semantics worth knowing:

- **`Set`** replaces any `\n`/`\r` in the value (each becomes a space) so the
  one-line-per-entry file format stays intact. Setting an existing key replaces it.
- **`Get`** returns `""` for an absent key (use `Has` to distinguish "absent" from
  "empty").
- **`Save`** writes every entry as a `name=value` line, opening the file binary and
  truncating. Returns `false` if the file can't be opened (or the stream goes bad).
- **`Load`** replaces the current contents. It strips a trailing `\r` (CRLF
  tolerance) and **skips** any line without a `=` or with an empty key (the `=` at
  index 0). A *missing* file is a benign `false` (the normal first-run case); it
  returns `false` **only** when the file can't be opened.

### Worked example — persist a `MultiSplitter` across runs

```cpp
#include <unigui/core/layout_store.h>
#include <unigui/widgets/multisplitter.h>
using namespace unigui;

LayoutStore store;
MultiSplitter splitter("workspace");

// On startup: load the file, then restore any saved split ratios.
store.Load("layout.ini");                       // benign false on first run
if (store.Has("main_split"))
    splitter.RestoreLayout(store.Get("main_split"));

// ... each frame, the splitter renders with draggable handles ...

// On shutdown: capture the current ratios and write them back.
store.Set("main_split", splitter.SerializeLayout());
store.Set("theme", "dark");                      // any simple preference fits too
store.Save("layout.ini");
```

`SerializeLayout()` returns an opaque, compact string (e.g. `"0.30,0.44,0.26"`);
`LayoutStore` treats it as a value and round-trips it verbatim (minus any embedded
newlines). `RestoreLayout()` applies it only when the value count matches the
current panel count — a stale or short layout for a different splitter is ignored
(it returns `false`) and is non-throwing on malformed input. This is the canonical
pairing: the widget owns the *format*, the store owns the *persistence*.

---

## Choosing the right tool

| You want to… | Use |
|--------------|-----|
| Compute sizes/positions with no ImGui (testable, embeddable) | `unigui::layout::SolveFlex` |
| Same, but wrap items onto multiple lines | `unigui::layout::SolveFlexWrap` |
| A responsive horizontal row of widgets that reflows | `unigui::Layout::FlexRow` |
| A quick inline row of callbacks (`A B C`) | `unigui::Layout::HBox` (function) |
| Stack callbacks vertically | `unigui::Layout::VBox` (function) |
| A fixed-ratio two-pane bordered split | `unigui::Layout::BeginHSplit` / `NextHSplit` / `EndHSplit` |
| A *resizable, serializable* multi-pane split | `MultiSplitter` widget |
| Override item spacing across a block of widgets | `unigui::HBox` / `unigui::VBox` (RAII classes) |
| A declarative flex row in a DSL tree | `unigui::dsl::Flex` |
| Persist layout/preferences to disk across runs | `unigui::LayoutStore` |

## See also

- `include/unigui/core/flex_layout.h` — the pure solver (this is the source of
  truth for the math).
- `include/unigui/widgets/layout.h` — the ImGui-facing helpers.
- `include/unigui/core/layout_store.h` — the persistence store.
- `tests/core/flex_layout_test.cc`, `tests/widgets/flexrow_test.cc` — exhaustive
  worked cases for every mode documented above.
- `include/unigui/widgets/multisplitter.h` — the resizable splitter whose
  `SerializeLayout()` output `LayoutStore` is designed to persist.
- `include/unigui/dsl/dsl.h` — the DSL `Flex` node built on `FlexRow`.
