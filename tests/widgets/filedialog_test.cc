#include <unigui/widgets/filedialog.h>

#include <imgui.h>

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using unigui::FileDialog;
using unigui::detail::DirEntry;
using unigui::detail::ExtensionMatches;
using unigui::detail::ListDirectory;

namespace {
void touch(const fs::path& p) {
    std::ofstream(p) << "x";
}

// 组件契约: 字符串边界一律 UTF-8。Windows 下 fs::path::string() 是 ACP(GBK),
// 中文用户名的临时目录会喂进错误编码 — 测试侧同样显式转 UTF-8。
std::string Utf8(const fs::path& p) {
#ifdef _WIN32
    const std::u8string u = p.u8string();
    return std::string(u.begin(), u.end());
#else
    return p.string();
#endif
}
fs::path FromUtf8(const std::string& s) {
#ifdef _WIN32
    return fs::path(std::u8string(s.begin(), s.end()));
#else
    return fs::path(s);
#endif
}
} // namespace

// ── Pure extension filter ────────────────────────────────────────────────────

TEST(FileDialogExt, EmptyFiltersAcceptsAll) {
    EXPECT_TRUE(ExtensionMatches("anything.xyz", {}));
    EXPECT_TRUE(ExtensionMatches("no_extension", {}));
}

TEST(FileDialogExt, CaseInsensitiveMatch) {
    const std::vector<std::string> exts = {".csv", ".txt"};
    EXPECT_TRUE(ExtensionMatches("data.csv", exts));
    EXPECT_TRUE(ExtensionMatches("DATA.CSV", exts));
    EXPECT_TRUE(ExtensionMatches("notes.TXT", exts));
    EXPECT_FALSE(ExtensionMatches("image.png", exts));
    EXPECT_FALSE(ExtensionMatches("noext", exts));
}

// ── Listing + the widget, against a real temp tree ───────────────────────────

class FileDialogFsTest : public ::testing::Test {
protected:
    fs::path root_;
    void SetUp() override {
        root_ = fs::temp_directory_path() / "unigui_filedialog_test";
        std::error_code ec;
        fs::remove_all(root_, ec);
        fs::create_directories(root_ / "sub", ec);
        touch(root_ / "a.csv");
        touch(root_ / "b.txt");
        touch(root_ / "c.png");
        touch(root_ / ".hidden");
    }
    void TearDown() override {
        std::error_code ec;
        fs::remove_all(root_, ec);
    }
};

TEST_F(FileDialogFsTest, ListDirectorySortsDirsFirstAndFiltersHidden) {
    std::vector<DirEntry> out;
    ASSERT_TRUE(ListDirectory(Utf8(root_), {}, /*showHidden=*/false, out));
    ASSERT_FALSE(out.empty());
    EXPECT_TRUE(out.front().isDir); // "sub" directory comes first
    EXPECT_EQ(out.front().name, "sub");
    // No dotfile when showHidden=false.
    for (const auto& e : out)
        EXPECT_NE(e.name, ".hidden");
}

TEST_F(FileDialogFsTest, ListDirectoryShowHidden) {
    std::vector<DirEntry> out;
    ASSERT_TRUE(ListDirectory(Utf8(root_), {}, /*showHidden=*/true, out));
    bool sawHidden = false;
    for (const auto& e : out)
        if (e.name == ".hidden")
            sawHidden = true;
    EXPECT_TRUE(sawHidden);
}

TEST_F(FileDialogFsTest, ListDirectoryExtensionFilterKeepsDirs) {
    std::vector<DirEntry> out;
    ASSERT_TRUE(ListDirectory(Utf8(root_), {".csv"}, false, out));
    int files = 0, dirs = 0;
    for (const auto& e : out) {
        if (e.isDir)
            ++dirs;
        else {
            ++files;
            EXPECT_EQ(e.name, "a.csv"); // only the .csv file survives
        }
    }
    EXPECT_EQ(files, 1);
    EXPECT_GE(dirs, 1); // directories are always listed
}

TEST_F(FileDialogFsTest, ListDirectoryMissingReturnsFalse) {
    std::vector<DirEntry> out{{"stale", false, 0}};
    EXPECT_FALSE(ListDirectory(Utf8(root_ / "does_not_exist"), {}, false, out));
    EXPECT_TRUE(out.empty()); // cleared on failure
}

TEST_F(FileDialogFsTest, EntriesRespectMode) {
    FileDialog fd;
    fd.SetDirectory(Utf8(root_)).SetFilters({".csv"});

    fd.SetMode(FileDialog::Mode::OpenFile);
    bool sawCsv = false, sawPng = false;
    for (const auto& e : fd.Entries()) {
        if (e.name == "a.csv")
            sawCsv = true;
        if (e.name == "c.png")
            sawPng = true;
    }
    EXPECT_TRUE(sawCsv);
    EXPECT_FALSE(sawPng); // filtered out

    // SelectFolder ignores file filters and shows only directories.
    fd.SetMode(FileDialog::Mode::SelectFolder);
    for (const auto& e : fd.Entries())
        EXPECT_TRUE(e.isDir);
}

TEST_F(FileDialogFsTest, NavigateIntoAndUp) {
    FileDialog fd;
    fd.SetDirectory(Utf8(root_));
    EXPECT_EQ(FromUtf8(fd.GetDirectory()).filename(), "unigui_filedialog_test");

    EXPECT_TRUE(fd.NavigateInto("sub"));
    EXPECT_EQ(FromUtf8(fd.GetDirectory()).filename(), "sub");
    EXPECT_FALSE(fd.NavigateInto("nope")); // not a directory

    EXPECT_TRUE(fd.NavigateUp());
    EXPECT_EQ(FromUtf8(fd.GetDirectory()).filename(), "unigui_filedialog_test");
}

TEST_F(FileDialogFsTest, ResolvedPathPerMode) {
    FileDialog fd;
    fd.SetDirectory(Utf8(root_));

    // OpenFile: empty until a file is selected.
    fd.SetMode(FileDialog::Mode::OpenFile);
    EXPECT_TRUE(fd.ResolvedPath().empty());
    fd.SelectFile("a.csv");
    EXPECT_EQ(FromUtf8(fd.ResolvedPath()).filename(), "a.csv");

    // SaveFile: derives from the filename field.
    fd.SetMode(FileDialog::Mode::SaveFile);
    fd.SetFilename("");
    EXPECT_TRUE(fd.ResolvedPath().empty());
    fd.SetFilename("out.csv");
    EXPECT_EQ(FromUtf8(fd.ResolvedPath()).filename(), "out.csv");

    // SelectFolder: the current directory itself.
    fd.SetMode(FileDialog::Mode::SelectFolder);
    EXPECT_EQ(FromUtf8(fd.ResolvedPath()).filename(), "unigui_filedialog_test");
}

TEST_F(FileDialogFsTest, ConfirmFiresCallbackAndCloses) {
    FileDialog fd;
    fd.SetDirectory(Utf8(root_)).SetMode(FileDialog::Mode::OpenFile);
    std::string got;
    fd.SetOnConfirm([&](const std::string& p) { got = p; });
    fd.Open();
    EXPECT_TRUE(fd.IsOpen());

    EXPECT_TRUE(fd.Confirm().empty()); // nothing selected yet → no-op
    EXPECT_TRUE(fd.IsOpen());
    EXPECT_TRUE(got.empty());

    fd.SelectFile("b.txt");
    const std::string path = fd.Confirm();
    EXPECT_FALSE(path.empty());
    EXPECT_EQ(got, path);
    EXPECT_EQ(fd.GetSelectedPath(), path);
    EXPECT_FALSE(fd.IsOpen()); // confirm closes
}

TEST_F(FileDialogFsTest, RenderDoesNotCrash) {
    ImGui::CreateContext();
    ImGui::GetIO().DisplaySize = ImVec2(1024, 768);
    ImGui::GetIO().Fonts->Build();

    FileDialog fd;
    fd.SetDirectory(Utf8(root_)).SetMode(FileDialog::Mode::SaveFile).SetFilename("x.csv");
    fd.Open();

    for (int frame = 0; frame < 3; ++frame) {
        ImGui::NewFrame();
        EXPECT_NO_THROW(fd.Render());
        ImGui::Render();
    }
    ImGui::DestroyContext();
}

// ── 中文路径回归(2026-11-13: ListDirectory/NavigateInto/ResolvedPath 的 ACP 乱码修复) ──

TEST_F(FileDialogFsTest, ChineseNamesRoundTripUtf8) {
    // 建中文目录 + 中文文件名(经宽字符路径, 与编码无关地创建)
    const fs::path cdir = root_ / fs::path(std::u8string(u8"中文目录"));
    std::error_code ec;
    fs::create_directories(cdir, ec);
    ASSERT_FALSE(ec) << "create 中文目录 failed";
    touch(cdir / fs::path(std::u8string(u8"策略.cpp")));

    // 列表: 条目名必须是合法 UTF-8 的中文名(不能是 GBK 字节或 U+FFFD)
    std::vector<DirEntry> out;
    ASSERT_TRUE(ListDirectory(Utf8(root_), {".cpp"}, false, out));
    ASSERT_FALSE(out.empty());
    ASSERT_EQ(out.front().name, std::string(u8"中文目录")) << "列表第一项应为中文目录";
    // 能进入中文目录并列出其中的中文文件
    FileDialog fd;
    fd.SetDirectory(Utf8(root_)).SetMode(FileDialog::Mode::OpenFile).SetFilters({".cpp"});
    ASSERT_TRUE(fd.NavigateInto(std::string(u8"中文目录")));
    std::vector<DirEntry> inner;
    ASSERT_TRUE(ListDirectory(fd.GetDirectory(), {".cpp"}, false, inner));
    ASSERT_EQ(inner.size(), 1u);
    EXPECT_EQ(inner[0].name, std::string(u8"策略.cpp"));
    // 选中后确认路径逐字节回环: UTF-8 串 → fs::path → 与真实路径相等
    fd.SelectFile(std::string(u8"策略.cpp"));
    const std::string resolved = fd.ResolvedPath();
    EXPECT_TRUE(fs::equivalent(FromUtf8(resolved), cdir / fs::path(std::u8string(u8"策略.cpp"))));
}
