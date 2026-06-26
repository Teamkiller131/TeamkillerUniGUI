#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// UniGUI Declarative DSL  (namespace unigui::dsl)
//
// Describe a UI as a tree of value-type builder calls, then `Render()` the tree
// each frame. The DSL is the most concise of the UniGUI "ease-of-use" layers —
// it complements (and is rendered through) the themed immediate-mode layer
// `unigui::im`, so DSL output matches the rest of the toolkit's look and feel.
//
//     auto ui = Window("Demo", VBox({
//         Text("Welcome!"),
//         Separator(),
//         HBox({ Button("Save", ButtonVariant::Primary, []{ save(); }),
//                Button("Exit", []{ std::exit(0); }) }),
//         CheckBox("Enabled", &enabled),
//         SliderFloat("Gain", &gain, 0.f, 1.f),
//         For(3, [](int i){ return Label("Row " + std::to_string(i)); }),
//     }));
//     // each frame:
//     Render(ui);
//
// Stateful controls (CheckBox / SliderFloat / InputText) either bind to an
// external variable via pointer, or keep their state inside the retained node
// itself — so simply re-`Render()`-ing the same tree preserves user input.
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/core/flex_layout.h>
#include <unigui/im/im.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace unigui::dsl {

// Forward
struct Node;
using NodePtr = std::shared_ptr<Node>;

/// Themed button color variant — mirrors `unigui::im::ButtonVariant`.
using ButtonVariant = im::ButtonVariant;

/// Main-axis distribution for a `Flex` row — mirrors `unigui::layout::FlexJustify`.
using FlexJustify = layout::FlexJustify;

/// A DSL node. Every builder function below produces one of these `Kind`s.
struct Node {
    enum class Kind {
        Window,
        VBox,
        HBox,
        Flex,
        Button,
        Label,
        Text,
        TextWrapped,
        TextDisabled,
        BulletText,
        CheckBox,
        SliderFloat,
        InputText,
        Separator,
        Spacing,
        If,
        For,
        Custom,
    };
    Kind kind = Kind::Label;

    // Container children (also: If `then`=children[0], `else`=children[1]).
    std::vector<NodePtr> children;

    // Text-bearing nodes.
    std::string text;
    std::string label;

    // Window.
    std::string title;

    // Button.
    ButtonVariant buttonVariant = ButtonVariant::Default;
    std::function<void()> onClick;

    // CheckBox — bound to `boolBinding` when non-null, else to `boolState`.
    bool* boolBinding = nullptr;
    bool boolState = false;
    std::function<void(bool)> onToggle;

    // SliderFloat — bound to `floatBinding` when non-null, else to `floatState`.
    float* floatBinding = nullptr;
    float floatState = 0.0f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    std::function<void(float)> onChangeFloat;

    // InputText — bound to `strBinding` when non-null, else to `strState`.
    std::string* strBinding = nullptr;
    std::string strState;
    std::function<void(const std::string&)> onChangeText;

    // If / IfElse.
    std::function<bool()> condition;

    // For.
    int count = 0;
    std::function<NodePtr(int)> itemBuilder;

    // Flex — one horizontal flex line of `children`. Each child grows by its
    // weight in `flexGrow` (parallel to `children`; defaults to 1 = equal split).
    // `flexGap` is the pixel gap between children; `flexJustify` distributes any
    // leftover main-axis space (see `unigui::layout::FlexJustify`).
    std::vector<float> flexGrow;
    float flexGap = 0.0f;
    FlexJustify flexJustify = FlexJustify::Start;

    // Custom — an escape hatch: `customDraw` is invoked verbatim each frame, so any
    // immediate-mode (`unigui::im`) drawing — or a hosted Component — can live
    // inside a DSL tree.
    std::function<void()> customDraw;
};

// ── Containers ───────────────────────────────────────────────────────────────
NodePtr Window(std::string title, NodePtr child);
NodePtr Window(std::string title, std::vector<NodePtr> children);
NodePtr VBox(std::vector<NodePtr> children);
NodePtr HBox(std::vector<NodePtr> children);

/// Horizontal flexbox row. Children are laid out side-by-side and share the
/// available width by their flex-grow weight; with the default weight (1) every
/// child gets an equal slice. Rendered through `unigui::Layout::FlexRow` at a
/// fixed one-line height (`ImGui::GetFrameHeightWithSpacing()`).
///
/// Limitations (v1): single line only (no wrap), one fixed row height, and
/// cross-axis alignment is not exposed. For per-child weights use the
/// `weights` overload (parallel to `children`; a shorter/empty vector falls
/// back to weight 1 for the missing entries).
NodePtr Flex(std::vector<NodePtr> children, float gap = 0.0f,
             FlexJustify justify = FlexJustify::Start);
NodePtr Flex(std::vector<NodePtr> children, std::vector<float> weights, float gap = 0.0f,
             FlexJustify justify = FlexJustify::Start);

/// Escape hatch: embed arbitrary immediate-mode drawing (or a hosted Component)
/// in a DSL tree. `draw` runs verbatim each frame at the current cursor.
NodePtr Custom(std::function<void()> draw);

// ── Text ─────────────────────────────────────────────────────────────────────
NodePtr Label(std::string text);
NodePtr Text(std::string text);
NodePtr TextWrapped(std::string text);
NodePtr TextDisabled(std::string text);
NodePtr BulletText(std::string text);

// ── Buttons ──────────────────────────────────────────────────────────────────
NodePtr Button(std::string label, std::function<void()> onClick = nullptr);
NodePtr Button(std::string label, ButtonVariant variant, std::function<void()> onClick = nullptr);

// ── Stateful inputs ──────────────────────────────────────────────────────────
/// Checkbox holding its own state inside the node (persists across frames).
NodePtr CheckBox(std::string label, std::function<void(bool)> onChange = nullptr);
/// Checkbox bound to an external `bool` (caller owns the storage).
NodePtr CheckBox(std::string label, bool* bound, std::function<void(bool)> onChange = nullptr);

/// Slider holding its own value inside the node.
NodePtr SliderFloat(std::string label, float min, float max,
                    std::function<void(float)> onChange = nullptr);
/// Slider bound to an external `float`.
NodePtr SliderFloat(std::string label, float* bound, float min, float max,
                    std::function<void(float)> onChange = nullptr);

/// Single-line text input holding its own value inside the node.
NodePtr InputText(std::string label, std::function<void(const std::string&)> onChange = nullptr);
/// Single-line text input bound to an external `std::string`.
NodePtr InputText(std::string label, std::string* bound,
                  std::function<void(const std::string&)> onChange = nullptr);

// ── Spacers ──────────────────────────────────────────────────────────────────
NodePtr Separator();
NodePtr Spacing();

// ── Control flow ─────────────────────────────────────────────────────────────
NodePtr If(std::function<bool()> condition, NodePtr thenNode);
NodePtr IfElse(std::function<bool()> condition, NodePtr thenNode, NodePtr elseNode);
NodePtr For(int count, std::function<NodePtr(int)> builder);

// ── Render ───────────────────────────────────────────────────────────────────
/// Render a DSL tree. Call once per frame.
void Render(NodePtr root);

} // namespace unigui::dsl
