#include <unigui/widgets/filepath.h>

#include <imgui.h>

#include <algorithm>
#include <cwchar>
#include <iterator>
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

#ifdef _WIN32
namespace {

// UTF-8 <-> UTF-16 boundary conversion. The widget API (path_/title_/filter_) is
// UTF-8 like the rest of the library; the ANSI dialog APIs interpret bytes in the
// local code page (GBK on zh-CN), which mangles non-ASCII paths/labels — so the
// dialog must go through the W variants with explicit conversion at this boundary.
std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty())
        return {};
    const int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int) s.size(), nullptr, 0);
    std::wstring w(n > 0 ? (size_t) n : 0, L'\0');
    if (n > 0)
        MultiByteToWideChar(CP_UTF8, 0, s.data(), (int) s.size(), w.data(), n);
    return w;
}

std::string WideToUtf8(const wchar_t* s, size_t len) {
    if (len == 0)
        return {};
    const int n = WideCharToMultiByte(CP_UTF8, 0, s, (int) len, nullptr, 0, nullptr, nullptr);
    std::string out(n > 0 ? (size_t) n : 0, '\0');
    if (n > 0)
        WideCharToMultiByte(CP_UTF8, 0, s, (int) len, out.data(), n, nullptr, nullptr);
    return out;
}

} // namespace
#endif

bool FilePath::OpenNativeDialog() {
#ifdef _WIN32
    wchar_t file[512] = {};
    const std::wstring widePath = Utf8ToWide(path_);
    std::copy_n(widePath.data(), std::min(widePath.size(), std::size(file) - 1), file);

    // filter_ uses '|' separators ("Label|*.ext|…"); the dialog wants embedded NULs.
    std::wstring wideFilter = Utf8ToWide(filter_);
    for (wchar_t& c : wideFilter)
        if (c == L'|')
            c = L'\0';
    wideFilter.push_back(L'\0'); // double-NUL terminator after the last entry

    const std::wstring wideTitle = Utf8ToWide(title_);

    OPENFILENAMEW ofn{};
    ofn.lStructSize = sizeof(ofn);
    ofn.lpstrFilter = filter_.empty() ? L"All Files\0*.*\0" : wideFilter.c_str();
    ofn.lpstrFile = file;
    ofn.nMaxFile = (DWORD) std::size(file);
    ofn.lpstrTitle =
        title_.empty() ? (mode_ == Open ? L"Open File" : L"Save File") : wideTitle.c_str();
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
    if (mode_ == Save)
        ofn.Flags |= OFN_OVERWRITEPROMPT;

    BOOL ok = (mode_ == Open) ? GetOpenFileNameW(&ofn) : GetSaveFileNameW(&ofn);
    if (ok) {
        path_ = WideToUtf8(file, wcsnlen(file, std::size(file)));
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
