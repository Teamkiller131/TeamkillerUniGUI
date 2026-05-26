#pragma once
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <variant>
#include <imgui.h>

namespace unigui::v2::dsl {

// Forward
struct Node;
using NodePtr = std::shared_ptr<Node>;

/// Widget type enum for leaf nodes
enum class WidgetType { Button, Label, CheckBox, Text, Separator, TextWrapped };

/// A DSL node: either a widget, container, text, if-condition, or for-loop
struct Node {
    // Container: children
    std::vector<NodePtr> children;

    // Widget leaf
    WidgetType widgetType = WidgetType::Label;
    std::string label;
    std::string text;
    std::function<void()> onClick;

    // Window properties
    std::string title;
    bool hasMenuBar = false;

    // If node
    std::function<bool()> condition;

    // For node
    int count = 0;
    std::function<NodePtr(int)> itemBuilder;

    enum class Kind { Widget, VBox, HBox, Window, If, For, Text, Separator, TextWrapped };
    Kind kind = Kind::Widget;
};

// ── Builder functions ──────────────────────────────────────────────────────

NodePtr Window(std::string title, NodePtr child);
NodePtr Window(std::string title, std::vector<NodePtr> children);

NodePtr VBox(std::vector<NodePtr> children);
NodePtr HBox(std::vector<NodePtr> children);

NodePtr Button(std::string label, std::function<void()> onClick = nullptr);
NodePtr Label(std::string text);
NodePtr CheckBox(std::string label, std::function<void(bool)> onChange = nullptr);
NodePtr Text(std::string text);
NodePtr TextWrapped(std::string text);
NodePtr Separator();

NodePtr If(std::function<bool()> condition, NodePtr thenNode);
NodePtr For(int count, std::function<NodePtr(int)> builder);

// ── Render ─────────────────────────────────────────────────────────────────

/// Render a DSL tree. Called each frame.
void Render(NodePtr root);

} // namespace unigui::v2::dsl
