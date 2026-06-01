#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {
class LineEdit : public ValueWidget<std::string> {
public:
    LineEdit(std::string name, std::string label, std::string value = "");
    void Render() override;
    void SetValue(std::string value);
    void SetPlaceholder(std::string text);
    void SetValidator(std::function<bool(const std::string&)> fn);
    bool HasError() const;
    void SetPasswordMode(bool on);
    void SetMultiline(bool on);
    void SetReadOnly(bool on);
    void SetMaxLength(int maxLen);
    // Undo/redo
    void Undo();
    void Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    int GetUndoDepth() const { return undoIndex_ + 1; }
    int GetRedoDepth() const { return (int)undoStack_.size() - undoIndex_ - 1; }
private:
    void PushUndo();
    std::string label_;
    std::string placeholder_;
    std::function<bool(const std::string&)> validator_;
    bool has_error_ = false;
    bool password_ = false;
    bool multiline_ = false;
    bool read_only_ = false;
    int max_length_ = 256;
    char buffer_[1024] = {};
    std::vector<std::string> undoStack_;
    int undoIndex_ = -1;
};
}
