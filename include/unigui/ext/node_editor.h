#pragma once
#include <imgui.h>
#include <string>
#include <functional>
#include <memory>
#include <cstdint>

// imgui-node-editor forward declarations
namespace ax::NodeEditor {
struct EditorContext;
using NodeId = uintptr_t;
using PinId  = uintptr_t;
using LinkId = uintptr_t;
enum class PinKind { Input, Output };
}

namespace unigui {

/// RAII wrapper for ax::NodeEditor context and Begin/End.
class NodeEditor {
public:
    NodeEditor(std::string id);
    ~NodeEditor();
    NodeEditor(const NodeEditor&) = delete;
    NodeEditor& operator=(const NodeEditor&) = delete;

    /// Begin the node editor canvas.
    void Begin(const ImVec2& size = ImVec2(0, 0));
    /// End the node editor canvas.
    void End();

    /// Begin a node. Call EndNode() after rendering node content.
    void BeginNode(ax::NodeEditor::NodeId id, const char* title, const ImVec4& color = ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    /// End current node.
    void EndNode();

    /// Begin an input pin on current node.
    void BeginInputPin(ax::NodeEditor::PinId id);
    /// Begin an output pin on current node.
    void BeginOutputPin(ax::NodeEditor::PinId id);
    /// End current pin.
    void EndPin();

    /// Create a link between two pins.
    bool Link(ax::NodeEditor::LinkId id, ax::NodeEditor::PinId startPin, ax::NodeEditor::PinId endPin,
        const ImVec4& color = ImVec4(1, 1, 1, 1), float thickness = 1.0f);

    /// Center the view on the content.
    void NavigateToContent();

private:
    ax::NodeEditor::EditorContext* ctx_ = nullptr;
    std::string id_;
    bool begun_ = false;
};

/// Node builder helper: NodeEditor::BeginNode/EndNode via callback.
template<typename F>
void RenderNode(NodeEditor& editor, ax::NodeEditor::NodeId id, const char* title, F&& callback,
    const ImVec4& color = ImVec4(0.2f, 0.2f, 0.2f, 1.0f)) {
    editor.BeginNode(id, title, color);
    callback();
    editor.EndNode();
}

} // namespace unigui
