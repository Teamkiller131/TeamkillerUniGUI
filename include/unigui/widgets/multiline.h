#pragma once
#include <unigui/widgets/widget_base.h>

#include <string>
#include <vector>
namespace unigui {
class MultiLine : public FluentWidget<MultiLine> {
public:
    MultiLine(std::string name, std::string text = "", int maxLines = 10);
    void Render() override;
    void SetText(std::string t);
    std::string GetText() const;
    void SetMaxLines(int n);
    void SetEditable(bool on);

    // ── Fluent (chainable) helpers — return MultiLine& via CRTP base ──────────
    MultiLine& WithText(std::string t) {
        SetText(std::move(t));
        return *this;
    }
    MultiLine& WithMaxLines(int n) {
        SetMaxLines(n);
        return *this;
    }
    MultiLine& WithEditable(bool on) {
        SetEditable(on);
        return *this;
    }

    // Undo/redo
    void Undo();
    void Redo();
    bool CanUndo() const;
    bool CanRedo() const;

private:
    void PushUndo();
    std::string text_;
    int maxLines_;
    bool editable_ = false;
    char buf_[4096] = {};
    std::vector<std::string> undoStack_;
    int undoIndex_ = -1;
};
} // namespace unigui
