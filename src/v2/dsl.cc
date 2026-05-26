#include <unigui/v2/dsl.h>

namespace unigui::v2::dsl {

// ── Helpers ────────────────────────────────────────────────────────────────

static NodePtr makeNode(Node::Kind k) { auto n = std::make_shared<Node>(); n->kind = k; return n; }

// ── Builder implementations ────────────────────────────────────────────────

NodePtr Window(std::string title, NodePtr child) {
    auto n = makeNode(Node::Kind::Window); n->title = std::move(title);
    n->children.push_back(std::move(child)); return n;
}
NodePtr Window(std::string title, std::vector<NodePtr> children) {
    auto n = makeNode(Node::Kind::Window); n->title = std::move(title);
    n->children = std::move(children); return n;
}

NodePtr VBox(std::vector<NodePtr> children) {
    auto n = makeNode(Node::Kind::VBox); n->children = std::move(children); return n;
}
NodePtr HBox(std::vector<NodePtr> children) {
    auto n = makeNode(Node::Kind::HBox); n->children = std::move(children); return n;
}

NodePtr Button(std::string label, std::function<void()> onClick) {
    auto n = makeNode(Node::Kind::Widget); n->widgetType = WidgetType::Button;
    n->label = std::move(label); n->onClick = std::move(onClick); return n;
}
NodePtr Label(std::string text) {
    auto n = makeNode(Node::Kind::Widget); n->widgetType = WidgetType::Label;
    n->text = std::move(text); return n;
}
NodePtr CheckBox(std::string label, std::function<void(bool)> onChange) {
    auto n = makeNode(Node::Kind::Widget); n->widgetType = WidgetType::CheckBox;
    n->label = std::move(label);
    if (onChange) n->onClick = [onChange]() { /* state managed by render */ };
    return n;
}
NodePtr Text(std::string text) {
    auto n = makeNode(Node::Kind::Text); n->text = std::move(text); return n;
}
NodePtr TextWrapped(std::string text) {
    auto n = makeNode(Node::Kind::TextWrapped); n->text = std::move(text); return n;
}
NodePtr Separator() { return makeNode(Node::Kind::Separator); }

NodePtr If(std::function<bool()> condition, NodePtr thenNode) {
    auto n = makeNode(Node::Kind::If); n->condition = std::move(condition);
    n->children.push_back(std::move(thenNode)); return n;
}
NodePtr For(int count, std::function<NodePtr(int)> builder) {
    auto n = makeNode(Node::Kind::For); n->count = count;
    n->itemBuilder = std::move(builder); return n;
}

// ── Render ─────────────────────────────────────────────────────────────────

static void renderImpl(NodePtr node) {
    if (!node) return;

    switch (node->kind) {
    case Node::Kind::Window:
        if (ImGui::Begin(node->title.c_str())) {
            for (auto& c : node->children) renderImpl(c);
        }
        ImGui::End();
        break;
    case Node::Kind::VBox:
        for (auto& c : node->children) { renderImpl(c); }
        break;
    case Node::Kind::HBox:
        for (size_t i = 0; i < node->children.size(); i++) {
            renderImpl(node->children[i]);
            if (i + 1 < node->children.size()) ImGui::SameLine();
        }
        break;
    case Node::Kind::Widget:
        switch (node->widgetType) {
        case WidgetType::Button:
            if (ImGui::Button(node->label.c_str()) && node->onClick) node->onClick();
            break;
        case WidgetType::Label:
            ImGui::TextUnformatted(node->text.c_str());
            break;
        case WidgetType::CheckBox: {
            static std::unordered_map<std::string,bool> states;
            bool& v = states[node->label];
            if (ImGui::Checkbox(node->label.c_str(), &v) && node->onClick) node->onClick();
            break;
        }
        case WidgetType::Text: case WidgetType::TextWrapped:
        default: break;
        }
        break;
    case Node::Kind::Text:
        ImGui::TextUnformatted(node->text.c_str()); break;
    case Node::Kind::TextWrapped:
        ImGui::TextWrapped("%s", node->text.c_str()); break;
    case Node::Kind::Separator:
        ImGui::Separator(); break;
    case Node::Kind::If:
        if (node->condition && node->condition() && !node->children.empty())
            renderImpl(node->children[0]);
        break;
    case Node::Kind::For:
        for (int i = 0; i < node->count && node->itemBuilder; i++) {
            auto child = node->itemBuilder(i);
            if (child) renderImpl(child);
        }
        break;
    }
}

void Render(NodePtr root) { renderImpl(root); }

} // namespace unigui::v2::dsl
