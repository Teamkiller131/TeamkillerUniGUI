#include <unigui/widgets/lineedit.h>
#include <imgui.h>
#include <algorithm>
namespace unigui {
LineEdit::LineEdit(std::string name, std::string label, std::string value)
    : Widget(std::move(name)), label_(std::move(label)), value_(std::move(value)) {
    SetValue(value_);
}
void LineEdit::PushUndo() {
    // Truncate redo tail
    if (undoIndex_ + 1 < (int)undoStack_.size()) undoStack_.resize(undoIndex_ + 1);
    undoStack_.push_back(value_);
    undoIndex_ = (int)undoStack_.size() - 1;
    // Limit undo depth to 50
    if ((int)undoStack_.size() > 50) { undoStack_.erase(undoStack_.begin()); undoIndex_--; }
}
void LineEdit::Render() {
    if (!IsVisible()) return;
    const char* hint = placeholder_.empty() ? nullptr : placeholder_.c_str();
    ImGuiInputTextFlags flags = 0;
    if (password_) flags |= ImGuiInputTextFlags_Password;
    if (multiline_) flags |= ImGuiInputTextFlags_None;
    if (read_only_) flags |= ImGuiInputTextFlags_ReadOnly;
    if (has_error_) ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.4f, 0.1f, 0.1f, 1.0f));
    ImGui::PushID(GetName().c_str());
    bool changed = false;
    if (multiline_) {
        changed = ImGui::InputTextMultiline(label_.c_str(), buffer_, max_length_, ImVec2(-1, 80), flags);
    } else {
        changed = ImGui::InputTextWithHint(label_.c_str(), hint ? hint : "", buffer_, max_length_, flags);
    }
    if (changed) {
        value_ = buffer_; has_error_ = false;
        if (validator_ && !validator_(value_)) has_error_ = true;
        PushUndo();
    }
    ImGui::PopID();
    if (has_error_) ImGui::PopStyleColor();
}
void LineEdit::Undo() {
    if (undoIndex_ > 0) { undoIndex_--; value_ = undoStack_[undoIndex_]; std::copy_n(value_.data(), std::min(value_.size(), sizeof(buffer_)-1), buffer_); buffer_[std::min(value_.size(), sizeof(buffer_)-1)] = 0; }
}
void LineEdit::Redo() {
    if (undoIndex_ + 1 < (int)undoStack_.size()) { undoIndex_++; value_ = undoStack_[undoIndex_]; std::copy_n(value_.data(), std::min(value_.size(), sizeof(buffer_)-1), buffer_); buffer_[std::min(value_.size(), sizeof(buffer_)-1)] = 0; }
}
bool LineEdit::CanUndo() const { return undoIndex_ > 0; }
bool LineEdit::CanRedo() const { return undoIndex_ + 1 < (int)undoStack_.size(); }
std::string LineEdit::GetValue() const { return value_; }
void LineEdit::SetValue(std::string value) {
    value_ = std::move(value);
    size_t n = std::min(value_.size(), sizeof(buffer_) - 1);
    std::copy_n(value_.data(), n, buffer_); buffer_[n] = 0;
    PushUndo();
}
void LineEdit::SetPlaceholder(std::string text) { placeholder_ = std::move(text); }
void LineEdit::SetValidator(std::function<bool(const std::string&)> fn) { validator_ = std::move(fn); }
bool LineEdit::HasError() const { return has_error_; }
void LineEdit::SetPasswordMode(bool on) { password_ = on; }
void LineEdit::SetMultiline(bool on) { multiline_ = on; }
void LineEdit::SetReadOnly(bool on) { read_only_ = on; }
void LineEdit::SetMaxLength(int maxLen) { max_length_ = maxLen; }
}
