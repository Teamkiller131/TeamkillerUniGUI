#include <unigui/dsl/dsl.h>
#include <unigui/widgets/layout.h>

#include <imgui.h>

#include <format>
#include <string>
#include <utility>
#include <vector>

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

NodePtr Flex(std::vector<NodePtr> children, float gap, FlexJustify justify) {
    auto n = makeNode(Node::Kind::Flex);
    n->children = std::move(children);
    n->flexGap = gap;
    n->flexJustify = justify;
    return n;
}
NodePtr Flex(std::vector<NodePtr> children, std::vector<float> weights, float gap,
             FlexJustify justify) {
    auto n = Flex(std::move(children), gap, justify);
    n->flexGrow = std::move(weights);
    return n;
}

NodePtr Custom(std::function<void()> draw) {
    auto n = makeNode(Node::Kind::Custom);
    n->customDraw = std::move(draw);
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

// Per-frame counter that hands each rendered Flex row a stable, unique id (for
// ID safety). Reset at the start of every Render() so the ids a given tree
// produces are identical frame-to-frame (the tree structure is retained), yet
// distinct between sibling Flex rows in one window.
static int g_flexCounter = 0;

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

    case Node::Kind::Flex: {
        // Build one FlexChild per DSL child. Each child grows by its weight in
        // `flexGrow` (default 1 = equal split); its render callback re-enters the
        // SAME per-node dispatch, so any node Kind can live inside a Flex row.
        std::vector<unigui::Layout::FlexChild> kids;
        kids.reserve(node->children.size());
        for (size_t i = 0; i < node->children.size(); ++i) {
            const float grow = i < node->flexGrow.size() ? node->flexGrow[i] : 1.0f;
            unigui::Layout::FlexChild fc;
            fc.item.grow = grow;
            // Capture the child shared_ptr by value (cheap) so the callback stays
            // valid for the FlexRow call; it re-enters the SAME per-node dispatch.
            fc.render = [child = node->children[i]] { renderImpl(child); };
            kids.push_back(std::move(fc));
        }
        // Stable, unique id for ID safety: a per-render counter (reset each
        // frame in Render()), so the same tree yields the same ids every frame
        // while sibling Flex rows stay distinct.
        const std::string id = "##dslflex" + std::to_string(g_flexCounter++);
        // One fixed line of height so following content flows underneath; pass
        // through the requested gap and justify. (v1: single line, fixed height.)
        unigui::Layout::FlexRowOptions opt;
        opt.height = ImGui::GetFrameHeightWithSpacing();
        opt.gap = node->flexGap;
        opt.justify = node->flexJustify;
        unigui::Layout::FlexRow(id.c_str(), kids, opt);
        break;
    }

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

    case Node::Kind::Custom:
        if (node->customDraw)
            node->customDraw();
        break;
    }
}

void Render(NodePtr root) {
    g_flexCounter = 0;
    renderImpl(root);
}

// ── Code emission (designer tool) ─────────────────────────────────────────────
namespace {

std::string Escape(std::string_view s) {
    std::string out;
    out.reserve(s.size() + 4);
    for (const char c : s) {
        switch (c) {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

std::string Str(std::string_view s) {
    return "\"" + Escape(s) + "\"";
}

// 0.5 -> "0.5f", 2 -> "2.0f", 1e-06 -> "1e-06f" (the 'f' suffix keeps the
// literal a float wherever the builder takes one).
std::string F(float v) {
    std::string s = std::format("{:g}", v);
    if (s.find('.') == std::string::npos && s.find('e') == std::string::npos)
        s += ".0";
    return s + "f";
}

const char* JustifyName(layout::FlexJustify j) {
    switch (j) {
    case layout::FlexJustify::Start:
        return "Start";
    case layout::FlexJustify::End:
        return "End";
    case layout::FlexJustify::Center:
        return "Center";
    case layout::FlexJustify::SpaceBetween:
        return "SpaceBetween";
    case layout::FlexJustify::SpaceAround:
        return "SpaceAround";
    case layout::FlexJustify::SpaceEvenly:
        return "SpaceEvenly";
    }
    return "Start";
}

const char* VariantName(im::ButtonVariant v) {
    switch (v) {
    case im::ButtonVariant::Default:
        return "Default";
    case im::ButtonVariant::Primary:
        return "Primary";
    case im::ButtonVariant::Danger:
        return "Danger";
    case im::ButtonVariant::Success:
        return "Success";
    case im::ButtonVariant::Warning:
        return "Warning";
    }
    return "Default";
}

// Emit one node as a builder expression. Containers lay their children out
// one per line; leaves stay inline. Returns lines (indented by `pad`).
std::vector<std::string> Emit(const NodePtr& node, int pad) {
    std::vector<std::string> lines;
    if (!node)
        return lines;
    const std::string ind(pad, ' ');

    auto emitChildren = [&](const std::vector<NodePtr>& children) {
        // Child expressions, each rendered at depth pad+2.
        std::vector<std::string> flat;
        for (const auto& c : children)
            for (auto& l : Emit(c, pad + 2))
                flat.push_back(std::move(l));
        return flat;
    };

    switch (node->kind) {
    case Node::Kind::Window: {
        const std::string t = Str(node->title);
        if (node->children.size() == 1) {
            const auto child = Emit(node->children[0], pad);
            if (child.empty()) {
                lines.push_back(ind + "Window(" + t + ", nullptr),");
            } else if (child.size() == 1) {
                // Leaf child: the whole expression fits on one line. The child
                // line carries a trailing comma and its own indent; drop both
                // before closing the window's own paren.
                std::string leaf = child[0].substr(pad);
                if (leaf.ends_with(','))
                    leaf.pop_back();
                lines.push_back(ind + "Window(" + t + ", " + leaf + "),");
            } else {
                // Container child: collapse its opening line onto the Window
                // line (de-indented — it now continues the Window's indent),
                // keep the middle, and REPLACE the child's closer with the
                // window's own so the parens stay balanced.
                lines.push_back(ind + "Window(" + t + ", " + child[0].substr(pad));
                for (std::size_t i = 1; i + 1 < child.size(); ++i)
                    lines.push_back(child[i]);
                lines.push_back(ind + "}),");
            }
        } else {
            lines.push_back(ind + "Window(" + t + ", {");
            for (auto& l : emitChildren(node->children))
                lines.push_back(std::move(l));
            lines.push_back(ind + "}),");
        }
        break;
    }
    case Node::Kind::VBox:
    case Node::Kind::HBox: {
        const char* name = node->kind == Node::Kind::VBox ? "VBox" : "HBox";
        lines.push_back(ind + std::string(name) + "({");
        for (auto& l : emitChildren(node->children))
            lines.push_back(std::move(l));
        lines.push_back(ind + "}),");
        break;
    }
    case Node::Kind::Flex: {
        lines.push_back(ind + "Flex({");
        for (auto& l : emitChildren(node->children))
            lines.push_back(std::move(l));
        if (!node->flexGrow.empty()) {
            std::string w = "{";
            for (std::size_t i = 0; i < node->flexGrow.size(); ++i) {
                if (i)
                    w += ", ";
                w += F(node->flexGrow[i]);
            }
            w += "}";
            lines.push_back(ind + "}, " + w + ", " + F(node->flexGap) +
                            ", FlexJustify::" + JustifyName(node->flexJustify) + "),");
        } else {
            lines.push_back(ind + "}, " + F(node->flexGap) +
                            ", FlexJustify::" + JustifyName(node->flexJustify) + "),");
        }
        break;
    }
    case Node::Kind::Button: {
        if (node->buttonVariant == im::ButtonVariant::Default) {
            lines.push_back(ind + "Button(" + Str(node->label) + "),");
        } else {
            lines.push_back(ind + "Button(" + Str(node->label) +
                            ", ButtonVariant::" + VariantName(node->buttonVariant) + "),");
        }
        break;
    }
    case Node::Kind::Label:
    case Node::Kind::Text:
    case Node::Kind::TextWrapped:
    case Node::Kind::TextDisabled:
    case Node::Kind::BulletText: {
        static const char* kNames[] = {"Label", "Text", "TextWrapped", "TextDisabled",
                                       "BulletText"};
        const char* name =
            kNames[static_cast<int>(node->kind) - static_cast<int>(Node::Kind::Label)];
        lines.push_back(ind + std::string(name) + "(" + Str(node->text) + "),");
        break;
    }
    case Node::Kind::CheckBox: {
        lines.push_back(ind + "CheckBox(" + Str(node->label) + ")," +
                        (node->boolBinding ? " // bound to an external bool" : ""));
        break;
    }
    case Node::Kind::SliderFloat: {
        lines.push_back(ind + "SliderFloat(" + Str(node->label) + ", " + F(node->minValue) + ", " +
                        F(node->maxValue) + ")," +
                        (node->floatBinding ? " // bound to an external float" : ""));
        break;
    }
    case Node::Kind::InputText: {
        lines.push_back(ind + "InputText(" + Str(node->label) + ")," +
                        (node->strBinding ? " // bound to an external string" : ""));
        break;
    }
    case Node::Kind::Separator:
        lines.push_back(ind + "Separator(),");
        break;
    case Node::Kind::Spacing:
        lines.push_back(ind + "Spacing(),");
        break;
    case Node::Kind::If: {
        const bool hasElse = node->children.size() > 1;
        const char* name = hasElse ? "IfElse" : "If";
        // The condition cannot be recovered from a std::function, so emit a
        // compilable placeholder (`[] { return true; }`) with a comment.
        if (hasElse) {
            auto thenLines = Emit(node->children[0], pad + 2);
            auto elseLines = Emit(node->children[1], pad + 2);
            lines.push_back(ind + std::string(name) + "([] { return true; } /* condition */,");
            for (auto& l : thenLines)
                lines.push_back(std::move(l));
            for (auto& l : elseLines)
                lines.push_back(std::move(l));
            lines.push_back(ind + "),");
        } else if (!node->children.empty()) {
            lines.push_back(ind + std::string(name) + "([] { return true; } /* condition */,");
            for (auto& l : Emit(node->children[0], pad + 2))
                lines.push_back(std::move(l));
            lines.push_back(ind + "),");
        } else {
            lines.push_back(ind + std::string(name) +
                            "([] { return true; } /* condition */, nullptr),");
        }
        break;
    }
    case Node::Kind::For: {
        lines.push_back(ind + "For(" + std::to_string(node->count) +
                        ", [](int i) { /* item builder */ return Label(std::to_string(i)); }),");
        break;
    }
    case Node::Kind::Custom:
        lines.push_back(ind + "Custom([] { /* draw lambda */ }),");
        break;
    }
    return lines;
}

} // namespace

std::string ToSource(const NodePtr& root) {
    if (!root)
        return {};
    std::string out = "using namespace unigui::dsl;\n\nNodePtr ui =\n";
    auto lines = Emit(root, 4);
    for (const auto& l : lines)
        out += l + "\n";
    // Drop the trailing comma the expression emitter added, then terminate.
    if (!out.empty() && out.back() == '\n')
        out.pop_back();
    if (!out.empty() && out.back() == ',')
        out.pop_back();
    out += ";\n";
    return out;
}

} // namespace unigui::dsl
