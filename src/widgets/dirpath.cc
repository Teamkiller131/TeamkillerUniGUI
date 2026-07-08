#include <unigui/widgets/dirpath.h>

#include <imgui.h>

#include <algorithm>
#ifdef _WIN32
#ifndef NOMINMAX // also defined project-wide; guard to avoid a /W4 C4005 redefinition
#define NOMINMAX
#endif
#include <windows.h>

#include <shlobj.h>
#endif

namespace unigui {

DirPath::DirPath(std::string name, std::string label)
        : FluentWidget<DirPath>(std::move(name))
        , label_(std::move(label)) {}
std::string DirPath::GetPath() const {
    return path_;
}
void DirPath::SetPath(std::string path) {
    path_ = std::move(path);
    size_t n = std::min(path_.size(), sizeof(buffer_) - 1);
    std::copy_n(path_.data(), n, buffer_);
    buffer_[n] = 0;
}
void DirPath::SetTitle(std::string title) {
    title_ = std::move(title);
}
void DirPath::SetOnPathChanged(std::function<void(std::string)> cb) {
    on_change_ = std::move(cb);
}

bool DirPath::OpenNativeDialog() {
#ifdef _WIN32
    BROWSEINFOA bi{};
    bi.lpszTitle = title_.empty() ? "Select Folder" : title_.c_str();
    bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
    LPITEMIDLIST pidl = SHBrowseForFolderA(&bi);
    if (pidl) {
        char folder[MAX_PATH];
        if (SHGetPathFromIDListA(pidl, folder)) {
            path_ = folder;
            size_t n = std::min(path_.size(), sizeof(buffer_) - 1);
            std::copy_n(path_.data(), n, buffer_);
            buffer_[n] = 0;
            if (on_change_)
                on_change_(path_);
        }
        CoTaskMemFree(pidl);
        return true;
    }
#endif
    return false;
}

void DirPath::Render() {
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
