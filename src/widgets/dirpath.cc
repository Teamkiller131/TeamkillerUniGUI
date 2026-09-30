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
    // IFileDialog (Vista+, UTF-16 native): SHBrowseForFolderA is the legacy
    // ANSI dialog — CJK selections garble on non-CJK system codepages.
    CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    bool ok = false;
    IFileDialog* fd = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER,
                                   IID_IFileDialog, (LPVOID*)&fd))) {
        DWORD opts = 0;
        fd->GetOptions(&opts);
        fd->SetOptions(opts | FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM);
        {
            const int tn = MultiByteToWideChar(
                CP_UTF8, 0, title_.empty() ? "Select Folder" : title_.c_str(), -1, nullptr, 0);
            std::wstring wt(tn > 0 ? tn - 1 : 0, L'\0');
            if (tn > 0)
                MultiByteToWideChar(CP_UTF8, 0,
                    title_.empty() ? "Select Folder" : title_.c_str(), -1, wt.data(), tn);
            fd->SetTitle(wt.c_str());
        }
        if (!path_.empty()) {
            const int pn = MultiByteToWideChar(CP_UTF8, 0, path_.c_str(), -1, nullptr, 0);
            std::wstring pp(pn > 0 ? pn - 1 : 0, L'\0');
            if (pn > 0)
                MultiByteToWideChar(CP_UTF8, 0, path_.c_str(), -1, pp.data(), pn);
            IShellItem* start = nullptr;
            if (SUCCEEDED(SHCreateItemFromParsingName(pp.c_str(), nullptr, IID_IShellItem,
                                                      (LPVOID*)&start)) &&
                start) {
                fd->SetFolder(start);
                start->Release();
            }
        }
        if (SUCCEEDED(fd->Show(nullptr))) {
            IShellItem* item = nullptr;
            if (SUCCEEDED(fd->GetResult(&item)) && item) {
                PWSTR wpath = nullptr;
                if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &wpath))) {
                    const int n = WideCharToMultiByte(CP_UTF8, 0, wpath, -1, nullptr, 0,
                                                      nullptr, nullptr);
                    path_.assign(n > 0 ? n - 1 : 0, '\0');
                    if (n > 0)
                        WideCharToMultiByte(CP_UTF8, 0, wpath, -1, path_.data(), n,
                                            nullptr, nullptr);
                    const size_t nz = std::min(path_.size(), sizeof(buffer_) - 1);
                    std::copy_n(path_.data(), nz, buffer_);
                    buffer_[nz] = 0;
                    if (on_change_) on_change_(path_);
                    CoTaskMemFree(wpath);
                    ok = true;
                }
                item->Release();
            }
        }
        fd->Release();
    }
    CoUninitialize();
    return ok;
#endif
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
