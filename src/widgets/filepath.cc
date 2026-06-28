#include <unigui/widgets/filepath.h>

#include <imgui.h>

#include <algorithm>
#ifdef _WIN32
#ifndef NOMINMAX // also defined project-wide; guard to avoid a /W4 C4005 redefinition
#define NOMINMAX
#endif
#include <windows.h>

#include <commdlg.h>
#endif

namespace unigui {

FilePath::FilePath(std::string name, std::string label, Mode mode)
        : Widget(std::move(name))
        , label_(std::move(label))
        , mode_(mode) {}

std::string FilePath::GetPath() const {
    return path_;
}
void FilePath::SetPath(std::string path) {
    path_ = std::move(path);
    size_t n = std::min(path_.size(), sizeof(buffer_) - 1);
    std::copy_n(path_.data(), n, buffer_);
    buffer_[n] = 0;
}
void FilePath::SetFilter(std::string filter) {
    filter_ = std::move(filter);
}
void FilePath::SetTitle(std::string title) {
    title_ = std::move(title);
}
void FilePath::SetMode(Mode mode) {
    mode_ = mode;
}
void FilePath::SetOnPathChanged(std::function<void(std::string)> cb) {
    on_change_ = std::move(cb);
}

bool FilePath::OpenNativeDialog() {
#ifdef _WIN32
    char file[512] = {};
    std::copy_n(path_.data(), std::min(path_.size(), sizeof(file) - 1), file);
    char filterBuf[512] = {};
    std::copy_n(filter_.data(), std::min(filter_.size(), sizeof(filterBuf) - 1), filterBuf);
    for (char* p = filterBuf; *p; p++)
        if (*p == '|')
            *p = 0;

    OPENFILENAMEA ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter_.empty() ? "All Files\0*.*\0" : filterBuf;
    ofn.lpstrFile = file;
    ofn.nMaxFile = sizeof(file);
    ofn.lpstrTitle = title_.empty() ? (mode_ == Open ? "Open File" : "Save File") : title_.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (mode_ == Save)
        ofn.Flags |= OFN_OVERWRITEPROMPT;

    BOOL ok = (mode_ == Open) ? GetOpenFileNameA(&ofn) : GetSaveFileNameA(&ofn);
    if (ok) {
        path_ = file;
        size_t n = std::min(path_.size(), sizeof(buffer_) - 1);
        std::copy_n(path_.data(), n, buffer_);
        buffer_[n] = 0;
        if (on_change_)
            on_change_(path_);
        return true;
    }
#endif
    return false;
}

bool FilePath::OpenDialog() {
    return OpenNativeDialog();
}

void FilePath::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    if (ImGui::InputText(label_.c_str(), buffer_, sizeof(buffer_))) {
        path_ = buffer_;
        if (on_change_)
            on_change_(path_);
    }
    ImGui::SameLine();
    if (ImGui::Button("Browse...")) {
        OpenNativeDialog();
    }
    ImGui::PopID();
}

} // namespace unigui
