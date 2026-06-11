#include <unigui/widgets/multiline.h>

#include <imgui.h>

#include <algorithm>
namespace unigui {
MultiLine::MultiLine(std::string n, std::string t, int ml)
        : Widget(std::move(n))
        , text_(std::move(t))
        , maxLines_(ml) {
    size_t sz = std::min(text_.size(), sizeof(buf_) - 1);
    std::copy_n(text_.data(), sz, buf_);
    buf_[sz] = 0;
    PushUndo();
}
void MultiLine::PushUndo() {
    if (undoIndex_ + 1 < (int) undoStack_.size())
        undoStack_.resize(undoIndex_ + 1);
    undoStack_.push_back(text_);
    undoIndex_ = (int) undoStack_.size() - 1;
    if ((int) undoStack_.size() > 50) {
        undoStack_.erase(undoStack_.begin());
        undoIndex_--;
    }
}
void MultiLine::SetText(std::string t) {
    text_ = std::move(t);
    size_t sz = std::min(text_.size(), sizeof(buf_) - 1);
    std::copy_n(text_.data(), sz, buf_);
    buf_[sz] = 0;
    PushUndo();
}
std::string MultiLine::GetText() const {
    return text_;
}
void MultiLine::SetMaxLines(int n) {
    maxLines_ = n;
}
void MultiLine::SetEditable(bool on) {
    editable_ = on;
}
void MultiLine::Undo() {
    if (undoIndex_ > 0) {
        undoIndex_--;
        text_ = undoStack_[undoIndex_];
        size_t sz = std::min(text_.size(), sizeof(buf_) - 1);
        std::copy_n(text_.data(), sz, buf_);
        buf_[sz] = 0;
    }
}
void MultiLine::Redo() {
    if (undoIndex_ + 1 < (int) undoStack_.size()) {
        undoIndex_++;
        text_ = undoStack_[undoIndex_];
        size_t sz = std::min(text_.size(), sizeof(buf_) - 1);
        std::copy_n(text_.data(), sz, buf_);
        buf_[sz] = 0;
    }
}
bool MultiLine::CanUndo() const {
    return undoIndex_ > 0;
}
bool MultiLine::CanRedo() const {
    return undoIndex_ + 1 < (int) undoStack_.size();
}
void MultiLine::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    float h = ImGui::GetTextLineHeight() * maxLines_;
    ImGuiInputTextFlags flags = editable_ ? 0 : ImGuiInputTextFlags_ReadOnly;
    if (ImGui::InputTextMultiline(GetName().c_str(), buf_, sizeof(buf_), ImVec2(-1, h), flags)) {
        text_ = buf_;
        PushUndo();
    }
    ImGui::PopID();
}
} // namespace unigui
