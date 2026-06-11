#include <unigui/dsl/dsl.h>

#include <imgui.h>

#include <utility>

namespace unigui::dsl {

// ── Helpers ────────────────────────────────────────────────────────────────

static NodePtr makeNode(Node::Kind k) {
    auto n = std::make_shared<Node>();
    n->kind = k;
    return n;
}

// ── Containers ───────────────────────────────────────────────────────────────

NodePtr Window(std::string title, NodePtr child) {
    auto n = makeNode(Node::Kind::Window);
    n->title = std::move(title);
    n->children.push_back(std::move(child));
    return n;
}
NodePtr Window(std::string title, std::vector<NodePtr> children) {
    auto n = makeNode(Node::Kind::Window);
    n->title = std::move(title);
    n->children = std::move(children);
    return n;
}

NodePtr VBox(std::vector<NodePtr> children) {
    auto n = makeNode(Node::Kind::VBox);
    n->children = std::move(children);
    return n;
}
NodePtr HBox(std::vector<NodePtr> children) {
    auto n = makeNode(Node::Kind::HBox);
    n->children = std::move(children);
    return n;
}

// ── Text ─────────────────────────────────────────────────────────────────────

static NodePtr makeText(Node::Kind k, std::string text) {
    auto n = makeNode(k);
    n->text = std::move(text);
    return n;
}

NodePtr Label(std::string text) {
    return makeText(Node::Kind::Label, std::move(text));
}
NodePtr Text(std::string text) {
    return makeText(Node::Kind::Text, std::move(text));
}
NodePtr TextWrapped(std::string text) {
    return makeText(Node::Kind::TextWrapped, std::move(text));
}
NodePtr TextDisabled(std::string text) {
    return makeText(Node::Kind::TextDisabled, std::move(text));
}
NodePtr BulletText(std::string text) {
    return makeText(Node::Kind::BulletText, std::move(text));
}

// ── Buttons ──────────────────────────────────────────────────────────────────

NodePtr Button(std::string label, std::function<void()> onClick) {
    return Button(std::move(label), ButtonVariant::Default, std::move(onClick));
}
NodePtr Button(std::string label, ButtonVariant variant, std::function<void()> onClick) {
    auto n = makeNode(Node::Kind::Button);
    n->label = std::move(label);
    n->buttonVariant = variant;
    n->onClick = std::move(onClick);
    return n;
}

// ── Stateful inputs ──────────────────────────────────────────────────────────

NodePtr CheckBox(std::string label, std::function<void(bool)> onChange) {
    auto n = makeNode(Node::Kind::CheckBox);
    n->label = std::move(label);
    n->onToggle = std::move(onChange);
    return n;
}
NodePtr CheckBox(std::string label, bool* bound, std::function<void(bool)> onChange) {
    auto n = CheckBox(std::move(label), std::move(onChange));
    n->boolBinding = bound;
    return n;
}

NodePtr SliderFloat(std::string label, float min, float max, std::function<void(float)> onChange) {
    auto n = makeNode(Node::Kind::SliderFloat);
    n->label = std::move(label);
    n->minValue = min;
    n->maxValue = max;
    n->floatState = min;
    n->onChangeFloat = std::move(onChange);
    return n;
}
NodePtr SliderFloat(std::string label, float* bound, float min, float max,
                    std::function<void(float)> onChange) {
    auto n = SliderFloat(std::move(label), min, max, std::move(onChange));
    n->floatBinding = bound;
    return n;
}

NodePtr InputText(std::string label, std::function<void(const std::string&)> onChange) {
    auto n = makeNode(Node::Kind::InputText);
    n->label = std::move(label);
    n->onChangeText = std::move(onChange);
    return n;
}
NodePtr InputText(std::string label, std::string* bound,
                  std::function<void(const std::string&)> onChange) {
    auto n = InputText(std::move(label), std::move(onChange));
    n->strBinding = bound;
    return n;
}

// ── Spacers ──────────────────────────────────────────────────────────────────

NodePtr Separator() {
    return makeNode(Node::Kind::Separator);
}
NodePtr Spacing() {
    return makeNode(Node::Kind::Spacing);
}

// ── Control flow ─────────────────────────────────────────────────────────────

NodePtr If(std::function<bool()> condition, NodePtr thenNode) {
    auto n = makeNode(Node::Kind::If);
    n->condition = std::move(condition);
    n->children.push_back(std::move(thenNode));
    return n;
}
NodePtr IfElse(std::function<bool()> condition, NodePtr thenNode, NodePtr elseNode) {
    auto n = If(std::move(condition), std::move(thenNode));
    n->children.push_back(std::move(elseNode));
    return n;
}
NodePtr For(int count, std::function<NodePtr(int)> builder) {
    auto n = makeNode(Node::Kind::For);
    n->count = count;
    n->itemBuilder = std::move(builder);
    return n;
}

// ── Render ─────────────────────────────────────────────────────────────────

static void renderImpl(const NodePtr& node) {
    if (!node)
        return;

    switch (node->kind) {
    case Node::Kind::Window:
        if (ImGui::Begin(node->title.c_str())) {
            for (auto& c : node->children)
                renderImpl(c);
        }
        ImGui::End();
        break;

    case Node::Kind::VBox:
        for (auto& c : node->children)
            renderImpl(c);
        break;

    case Node::Kind::HBox:
        for (size_t i = 0; i < node->children.size(); ++i) {
            renderImpl(node->children[i]);
            if (i + 1 < node->children.size())
                im::SameLine();
        }
        break;

    case Node::Kind::Button:
        if (im::Button(node->label, node->buttonVariant) && node->onClick)
            node->onClick();
        break;

    case Node::Kind::Label:
    case Node::Kind::Text:
        im::Text(node->text);
        break;
    case Node::Kind::TextWrapped:
        im::TextWrapped(node->text);
        break;
    case Node::Kind::TextDisabled:
        im::TextDisabled(node->text);
        break;
    case Node::Kind::BulletText:
        im::BulletText(node->text);
        break;

    case Node::Kind::CheckBox: {
        bool* value = node->boolBinding ? node->boolBinding : &node->boolState;
        if (im::Checkbox(node->label, value) && node->onToggle)
            node->onToggle(*value);
        break;
    }
    case Node::Kind::SliderFloat: {
        float* value = node->floatBinding ? node->floatBinding : &node->floatState;
        if (im::SliderFloat(node->label, value, node->minValue, node->maxValue) &&
            node->onChangeFloat)
            node->onChangeFloat(*value);
        break;
    }
    case Node::Kind::InputText: {
        std::string* value = node->strBinding ? node->strBinding : &node->strState;
        if (im::InputText(node->label, value) && node->onChangeText)
            node->onChangeText(*value);
        break;
    }

    case Node::Kind::Separator:
        im::Separator();
        break;
    case Node::Kind::Spacing:
        im::Spacing();
        break;

    case Node::Kind::If:
        if (node->condition && node->condition()) {
            if (!node->children.empty())
                renderImpl(node->children[0]);
        } else if (node->children.size() > 1) {
            renderImpl(node->children[1]);
        }
        break;

    case Node::Kind::For:
        for (int i = 0; i < node->count && node->itemBuilder; ++i)
            renderImpl(node->itemBuilder(i));
        break;
    }
}

void Render(NodePtr root) {
    renderImpl(root);
}

} // namespace unigui::dsl
