#include <unigui/widgets/filedialog.h>

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <filesystem>
#include <system_error>

namespace fs = std::filesystem;

namespace unigui {

namespace {

// Windows 的 fs::path::string()/path("窄串") 走 ACP(中文系统=GBK), 而本组件与应用
// 约定字符串边界一律 UTF-8(ImGui 渲染也是 UTF-8)。不做转换会出现两种症状:
// 目录名渲染成 U+FFFD 乱码; 确认路径返回 GBK 串被调用方当 UTF-8 用 → 中文路径打不开。
fs::path PathFromUtf8(const std::string& s) {
#ifdef _WIN32
    return fs::path(std::u8string(s.begin(), s.end()));
#else
    return fs::path(s);   // POSIX 原生窄编码即 UTF-8
#endif
}

std::string PathToUtf8(const fs::path& p) {
#ifdef _WIN32
    const std::u8string u = p.u8string();
    return std::string(u.begin(), u.end());
#else
    return p.string();
#endif
}

} // namespace

namespace detail {

namespace {
std::string toLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return s;
}
} // namespace

bool ExtensionMatches(const std::string& filename, const std::vector<std::string>& exts) {
    if (exts.empty())
        return true;
    // 纯 ASCII 层面取扩展名(避免 UTF-8 文件名经 fs::path 的 ACP 误解码;
    // GBK 误读时多字节对可能吞掉相邻的 '.')
    const size_t dot = filename.find_last_of('.');
    const std::string ext = toLower(dot == std::string::npos ? std::string() : filename.substr(dot));
    for (const auto& e : exts) {
        if (toLower(e) == ext)
            return true;
    }
    return false;
}

bool ListDirectory(const std::string& dir, const std::vector<std::string>& exts, bool showHidden,
                   std::vector<DirEntry>& out) {
    out.clear();
    std::error_code ec;
    fs::directory_iterator it(PathFromUtf8(dir), fs::directory_options::skip_permission_denied, ec);
    if (ec)
        return false;
    const fs::directory_iterator end;
    for (; it != end; it.increment(ec)) {
        if (ec)
            break; // stop on iteration error, keep what we have
        const fs::directory_entry& de = *it;
        const std::string name = PathToUtf8(de.path().filename());
        if (name.empty())
            continue;
        if (!showHidden && name[0] == '.')
            continue;

        std::error_code dec;
        const bool isDir = de.is_directory(dec);
        if (dec)
            continue;
        if (!isDir && !ExtensionMatches(name, exts))
            continue;

        DirEntry entry;
        entry.name = name;
        entry.isDir = isDir;
        if (!isDir) {
            std::error_code sec;
            const auto sz = de.file_size(sec);
            entry.size = sec ? 0u : sz;
        }
        out.push_back(std::move(entry));
    }
    // Directories first, then files; each group alphabetical (case-insensitive).
    std::sort(out.begin(), out.end(), [](const DirEntry& a, const DirEntry& b) {
        if (a.isDir != b.isDir)
            return a.isDir; // dirs before files
        return toLower(a.name) < toLower(b.name);
    });
    return true;
}

} // namespace detail

namespace {
std::string currentPathOr(const std::string& fallback) {
    std::error_code ec;
    const fs::path p = fs::current_path(ec);
    return ec ? fallback : PathToUtf8(p);
}
} // namespace

FileDialog::FileDialog(std::string name)
        : FluentWidget<FileDialog>(std::move(name)) {
    dir_ = currentPathOr(".");
}

FileDialog& FileDialog::SetDirectory(const std::string& dir) {
    std::error_code ec;
    const fs::path p = PathFromUtf8(dir);
    if (fs::is_directory(p, ec) && !ec)
        dir_ = PathToUtf8(p.lexically_normal());
    return *this;
}

FileDialog& FileDialog::SetFilename(const std::string& name) {
    std::snprintf(nameBuf_, sizeof(nameBuf_), "%s", name.c_str());
    return *this;
}

void FileDialog::Open() {
    open_ = true;
    openRequested_ = true;
    selected_.clear();
}

void FileDialog::Close() {
    open_ = false;
    openRequested_ = false;
}

std::vector<detail::DirEntry> FileDialog::Entries() const {
    std::vector<detail::DirEntry> out;
    const std::vector<std::string> none;
    detail::ListDirectory(dir_, mode_ == Mode::SelectFolder ? none : filters_, showHidden_, out);
    if (mode_ == Mode::SelectFolder)
        out.erase(std::remove_if(out.begin(), out.end(),
                                 [](const detail::DirEntry& e) { return !e.isDir; }),
                  out.end());
    return out;
}

bool FileDialog::NavigateInto(const std::string& dirName) {
    // dirName 来自 UTF-8 目录列表; 直接 path/窄串 会经 ACP 误解码, 中文目录进不去
    const fs::path target = PathFromUtf8(dir_) / PathFromUtf8(dirName);
    std::error_code ec;
    if (!fs::is_directory(target, ec) || ec)
        return false;
    dir_ = PathToUtf8(target.lexically_normal());
    selected_.clear();
    return true;
}

bool FileDialog::NavigateUp() {
    const fs::path cur = PathFromUtf8(dir_);
    const fs::path parent = cur.parent_path();
    if (parent.empty() || parent == cur)
        return false; // at a root
    dir_ = PathToUtf8(parent.lexically_normal());
    selected_.clear();
    return true;
}

void FileDialog::SelectFile(const std::string& name) {
    selected_ = name;
    if (mode_ == Mode::SaveFile)
        std::snprintf(nameBuf_, sizeof(nameBuf_), "%s", name.c_str());
}

bool FileDialog::MatchesFilter(const std::string& filename) const {
    return detail::ExtensionMatches(filename, filters_);
}

std::string FileDialog::ResolvedPath() const {
    switch (mode_) {
    case Mode::SelectFolder:
        return PathToUtf8(PathFromUtf8(dir_).lexically_normal());
    case Mode::OpenFile:
        if (selected_.empty())
            return {};
        return PathToUtf8((PathFromUtf8(dir_) / PathFromUtf8(selected_)).lexically_normal());
    case Mode::SaveFile: {
        const std::string fn = nameBuf_;
        if (fn.empty())
            return {};
        return PathToUtf8((PathFromUtf8(dir_) / PathFromUtf8(fn)).lexically_normal());
    }
    }
    return {};
}

std::string FileDialog::Confirm() {
    const std::string path = ResolvedPath();
    if (path.empty())
        return {};
    confirmedPath_ = path;
    Close();
    if (onConfirm_)
        onConfirm_(path);
    return path;
}

void FileDialog::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());

    if (openRequested_) {
        ImGui::OpenPopup(title_.c_str());
        openRequested_ = false;
    }

    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(
        ImVec2(vp->WorkPos.x + vp->WorkSize.x * 0.5f, vp->WorkPos.y + vp->WorkSize.y * 0.5f),
        ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(
        ImVec2(std::min(640.0f, vp->WorkSize.x * 0.9f), std::min(460.0f, vp->WorkSize.y * 0.9f)),
        ImGuiCond_Appearing);

    bool stayOpen = true;
    if (ImGui::BeginPopupModal(title_.c_str(), &stayOpen, ImGuiWindowFlags_NoSavedSettings)) {
        // ── Breadcrumb / current dir + up button ────────────────────────────
        if (ImGui::Button("Up"))
            NavigateUp();
        ImGui::SameLine();
        ImGui::TextDisabled("%s", dir_.c_str());
        ImGui::Separator();

        // Defer every mutating action so the ImGui stack stays balanced and the
        // listing is not changed mid-iteration. Exactly one EndPopup() runs.
        std::string enterDir;
        bool doConfirm = false;
        bool doCancel = false;

        // ── Listing ──────────────────────────────────────────────────────────
        const auto entries = Entries();
        ImGui::BeginChild("##fdlist", ImVec2(0, -ImGui::GetFrameHeightWithSpacing() * 2.2f), true);
        for (const auto& e : entries) {
            ImGui::PushID(e.name.c_str());
            const bool isSel = (!e.isDir && e.name == selected_);
            const std::string label = (e.isDir ? "[DIR]  " : "       ") + e.name;
            if (ImGui::Selectable(label.c_str(), isSel, ImGuiSelectableFlags_AllowDoubleClick)) {
                const bool dbl = ImGui::IsMouseDoubleClicked(0);
                // Keyboard nav-activation (Space/Enter on the focused row) reports
                // pressed with the mouse elsewhere — there is no nav double-click, so
                // treat it as "open": descend into a directory / confirm a file.
                // Mouse behavior is unchanged (single-click selects, double opens).
                const bool navActivated = !ImGui::IsItemHovered();
                if (e.isDir) {
                    if (dbl || navActivated)
                        enterDir = e.name;
                } else {
                    SelectFile(e.name);
                    if (dbl || navActivated)
                        doConfirm = true;
                }
            }
            ImGui::PopID();
        }
        ImGui::EndChild();

        // ── Filename field (SaveFile) ─────────────────────────────────────────
        if (mode_ == Mode::SaveFile) {
            ImGui::SetNextItemWidth(-1);
            ImGui::InputTextWithHint("##fdname", "File name", nameBuf_, sizeof(nameBuf_));
        }

        // ── Action buttons ─────────────────────────────────────────────────────
        const char* okLabel = mode_ == Mode::SaveFile       ? "Save"
                              : mode_ == Mode::SelectFolder ? "Select Folder"
                                                            : "Open";
        const bool canOk = !ResolvedPath().empty();
        ImGui::BeginDisabled(!canOk);
        if (ImGui::Button(okLabel))
            doConfirm = true;
        ImGui::EndDisabled();
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
            doCancel = true;

        // ── Apply deferred actions (single exit point) ──────────────────────────
        if (doConfirm && !ResolvedPath().empty()) {
            Confirm(); // records path, fires onConfirm, sets open_ = false
            ImGui::CloseCurrentPopup();
        } else if (doCancel) {
            Close();
            ImGui::CloseCurrentPopup();
            if (onCancel_)
                onCancel_();
        } else if (!enterDir.empty()) {
            NavigateInto(enterDir);
        }
        ImGui::EndPopup();
    }

    if (!stayOpen && open_) {
        // Closed via the window 'x'.
        open_ = false;
        if (onCancel_)
            onCancel_();
    }

    ImGui::PopID();
}

} // namespace unigui
