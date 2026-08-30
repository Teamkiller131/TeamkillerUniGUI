// Scene-text parser for the designer tool (include/unigui/dsl/dsl_scene.h).
//
// A tiny indentation-based format that describes a DSL tree in text, so scenes
// can be typed/pasted/edited in-app and hot-reloaded from disk. Deliberately a
// SUBSET of the builder API: everything stateful/structural round-trips, while
// things a text file cannot express (callbacks, conditions, custom bodies) are
// rejected with a clear error instead of silently degrading.

#include <unigui/core/strutil.h>
#include <unigui/dsl/dsl_scene.h>

#include <string_view>
#include <vector>

namespace unigui::dsl {

namespace {

// ── Line tokenizer ────────────────────────────────────────────────────────────

enum class ArgKind { Str, Number, Ident };

struct Arg {
    ArgKind kind = ArgKind::Ident;
    std::string text;
    double number = 0.0;
};

// Tokenize the argument part of a line: double-quoted strings (with \" and \\
// escapes), numbers, and bare identifiers, separated by whitespace.
bool Tokenize(std::string_view rest, std::vector<Arg>& out, std::string& err) {
    out.clear();
    std::size_t i = 0;
    while (i < rest.size()) {
        while (i < rest.size() && (rest[i] == ' ' || rest[i] == '\t'))
            ++i;
        if (i >= rest.size())
            break;
        if (rest[i] == '"') {
            ++i;
            std::string s;
            bool closed = false;
            while (i < rest.size()) {
                const char c = rest[i++];
                if (c == '"') {
                    closed = true;
                    break;
                }
                if (c == '\\' && i < rest.size()) {
                    const char e = rest[i++];
                    switch (e) {
                    case 'n':
                        s += '\n';
                        break;
                    case 't':
                        s += '\t';
                        break;
                    case '"':
                        s += '"';
                        break;
                    case '\\':
                        s += '\\';
                        break;
                    default:
                        s += '\\';
                        s += e;
                        break;
                    }
                } else {
                    s += c;
                }
            }
            if (!closed) {
                err = "unterminated string literal";
                return false;
            }
            out.push_back({ArgKind::Str, std::move(s), 0.0});
            continue;
        }
        // Number or identifier: consume until whitespace.
        std::size_t start = i;
        while (i < rest.size() && rest[i] != ' ' && rest[i] != '\t')
            ++i;
        const std::string tok(rest.substr(start, i - start));
        double num = 0.0;
        if (unigui::TryToDouble(tok, num))
            out.push_back({ArgKind::Number, tok, num});
        else
            out.push_back({ArgKind::Ident, tok, 0.0});
    }
    return true;
}

bool ExpectStr(const std::vector<Arg>& args, std::size_t idx, std::string& out) {
    if (idx >= args.size() || args[idx].kind != ArgKind::Str)
        return false;
    out = args[idx].text;
    return true;
}

bool ExpectNum(const std::vector<Arg>& args, std::size_t idx, float& out) {
    if (idx >= args.size() || args[idx].kind != ArgKind::Number)
        return false;
    out = static_cast<float>(args[idx].number);
    return true;
}

// ── Node helpers ──────────────────────────────────────────────────────────────

NodePtr Make(Node::Kind kind) {
    auto n = std::make_shared<Node>();
    n->kind = kind;
    return n;
}

// Deep copy — `for` clones its template once per iteration so stateful
// template children get fresh instances.
NodePtr Clone(const NodePtr& n) {
    if (!n)
        return nullptr;
    auto c = std::make_shared<Node>(*n);
    c->children.clear();
    c->children.reserve(n->children.size());
    for (const auto& child : n->children)
        c->children.push_back(Clone(child));
    return c;
}

// ── Line model ────────────────────────────────────────────────────────────────

struct Line {
    int number = 0;
    int indent = 0;
    std::string_view keyword;
    std::vector<Arg> args;
};

// Build one DSL node from a line; children are attached afterwards.
NodePtr BuildNode(const Line& line, std::string& err) {
    const auto& kw = line.keyword;
    const auto& a = line.args;

    if (kw == "window") {
        std::string title;
        if (!ExpectStr(a, 0, title) || a.size() != 1) {
            err = "window expects exactly one quoted title";
            return nullptr;
        }
        auto n = Make(Node::Kind::Window);
        n->title = std::move(title);
        return n;
    }
    if (kw == "vbox" || kw == "hbox" || kw == "flex") {
        if (!a.empty()) {
            err = std::string(kw) + " takes no arguments";
            return nullptr;
        }
        return Make(kw == "vbox"   ? Node::Kind::VBox
                    : kw == "hbox" ? Node::Kind::HBox
                                   : Node::Kind::Flex);
    }
    if (kw == "label" || kw == "text" || kw == "text_wrapped" || kw == "text_disabled" ||
        kw == "bullet_text") {
        std::string s;
        if (!ExpectStr(a, 0, s) || a.size() != 1) {
            err = std::string(kw) + " expects exactly one quoted string";
            return nullptr;
        }
        Node::Kind kind = Node::Kind::Label;
        if (kw == "text")
            kind = Node::Kind::Text;
        else if (kw == "text_wrapped")
            kind = Node::Kind::TextWrapped;
        else if (kw == "text_disabled")
            kind = Node::Kind::TextDisabled;
        else if (kw == "bullet_text")
            kind = Node::Kind::BulletText;
        auto n = Make(kind);
        n->text = std::move(s);
        return n;
    }
    if (kw == "button") {
        std::string label;
        if (!ExpectStr(a, 0, label) || a.size() > 2) {
            err = "button expects \"label\" [variant]";
            return nullptr;
        }
        auto n = Make(Node::Kind::Button);
        n->label = std::move(label);
        if (a.size() == 2) {
            if (a[1].kind != ArgKind::Ident) {
                err = "button variant must be an identifier";
                return nullptr;
            }
            const std::string& v = a[1].text;
            if (v == "default")
                n->buttonVariant = ButtonVariant::Default;
            else if (v == "primary")
                n->buttonVariant = ButtonVariant::Primary;
            else if (v == "danger")
                n->buttonVariant = ButtonVariant::Danger;
            else if (v == "success")
                n->buttonVariant = ButtonVariant::Success;
            else if (v == "warning")
                n->buttonVariant = ButtonVariant::Warning;
            else {
                err = "unknown button variant '" + v + "'";
                return nullptr;
            }
        }
        return n;
    }
    if (kw == "checkbox") {
        std::string label;
        if (!ExpectStr(a, 0, label) || a.size() != 1) {
            err = "checkbox expects exactly one quoted label";
            return nullptr;
        }
        auto n = Make(Node::Kind::CheckBox);
        n->label = std::move(label);
        return n;
    }
    if (kw == "slider_float") {
        std::string label;
        float lo = 0.0f, hi = 0.0f;
        if (!ExpectStr(a, 0, label) || !ExpectNum(a, 1, lo) || !ExpectNum(a, 2, hi) ||
            a.size() != 3) {
            err = "slider_float expects \"label\" min max";
            return nullptr;
        }
        auto n = Make(Node::Kind::SliderFloat);
        n->label = std::move(label);
        n->minValue = lo;
        n->maxValue = hi;
        return n;
    }
    if (kw == "input_text") {
        std::string label;
        if (!ExpectStr(a, 0, label) || a.size() != 1) {
            err = "input_text expects exactly one quoted label";
            return nullptr;
        }
        auto n = Make(Node::Kind::InputText);
        n->label = std::move(label);
        return n;
    }
    if (kw == "separator" || kw == "spacing") {
        if (!a.empty()) {
            err = std::string(kw) + " takes no arguments";
            return nullptr;
        }
        return Make(kw == "separator" ? Node::Kind::Separator : Node::Kind::Spacing);
    }
    if (kw == "for") {
        if (a.size() != 1 || a[0].kind != ArgKind::Number ||
            a[0].number != static_cast<double>(static_cast<int>(a[0].number)) ||
            static_cast<int>(a[0].number) < 0) {
            err = "for expects one non-negative integer count";
            return nullptr;
        }
        auto n = Make(Node::Kind::For);
        n->count = static_cast<int>(a[0].number);
        return n;
    }
    if (kw == "if" || kw == "if_else" || kw == "custom") {
        err = std::string(kw) + " needs code (callbacks/conditions) and cannot be expressed "
                                "in scene text";
        return nullptr;
    }
    err = "unknown keyword '" + std::string(kw) + "'";
    return nullptr;
}

} // namespace

SceneParseResult ParseScene(std::string_view text) {
    SceneParseResult result;

    // ── Tokenize lines ────────────────────────────────────────────────────────
    std::vector<Line> lines;
    int lineNumber = 0;
    for (std::size_t pos = 0; pos <= text.size(); ++lineNumber) {
        const std::size_t eol = text.find('\n', pos);
        const bool last = eol == std::string_view::npos;
        std::string_view raw = text.substr(pos, last ? text.size() - pos : eol - pos);
        pos = last ? text.size() : eol + 1;

        if (!raw.empty() && raw.back() == '\r')
            raw.remove_suffix(1);

        std::size_t indent = 0;
        while (indent < raw.size() && raw[indent] == ' ')
            ++indent;
        const std::string_view body = raw.substr(indent);
        if (!(body.empty() || body[0] == '#')) { // skip blank / comment lines
            Line line;
            line.number = lineNumber + 1; // 1-based for humans
            line.indent = static_cast<int>(indent);
            const std::size_t sp = body.find_first_of(" \t");
            line.keyword = sp == std::string_view::npos ? body : body.substr(0, sp);
            std::string err;
            if (!Tokenize(sp == std::string_view::npos ? std::string_view{} : body.substr(sp),
                          line.args, err)) {
                result.error = std::to_string(line.number) + ": " + err;
                return result;
            }
            lines.push_back(std::move(line));
        }
        if (last)
            break;
    }

    if (lines.empty()) {
        result.error = "empty scene (start with `window \"title\"`)";
        return result;
    }
    if (lines[0].keyword != "window") {
        result.error = std::to_string(lines[0].number) + ": the root of a scene must be "
                                                         "`window`";
        return result;
    }

    // ── Build the tree with an indent stack ───────────────────────────────────
    struct Frame {
        int indent = 0;
        NodePtr node;
    };
    std::vector<Frame> stack;
    for (const Line& line : lines) {
        std::string err;
        NodePtr node = BuildNode(line, err);
        if (!node) {
            result.error = std::to_string(line.number) + ": " + err;
            return result;
        }
        if (stack.empty()) {
            stack.push_back({line.indent, node});
            continue;
        }
        while (!stack.empty() && line.indent <= stack.back().indent)
            stack.pop_back();
        if (stack.empty()) {
            result.error = std::to_string(line.number) + ": indentation does not attach to any "
                                                         "parent";
            return result;
        }
        stack.back().node->children.push_back(node);
        stack.push_back({line.indent, node});
    }

    // ── Resolve `for` templates into cloning item builders ────────────────────
    // Post-pass: a `for`'s attached children ARE its template. Children are
    // pushed for processing BEFORE the template is moved out, so a nested
    // `for` inside the template still gets its own item builder.
    std::vector<NodePtr> pending = {stack.front().node};
    while (!pending.empty()) {
        NodePtr n = pending.back();
        pending.pop_back();
        for (const auto& c : n->children)
            pending.push_back(c);
        if (n->kind == Node::Kind::For && !n->children.empty()) {
            const std::vector<NodePtr> tmpl = std::move(n->children);
            n->children.clear();
            n->itemBuilder = [tmpl](int) {
                if (tmpl.size() == 1)
                    return Clone(tmpl[0]);
                auto box = Make(Node::Kind::VBox);
                for (const auto& t : tmpl)
                    box->children.push_back(Clone(t));
                return box;
            };
        }
    }

    result.tree = stack.front().node;
    return result;
}

} // namespace unigui::dsl
