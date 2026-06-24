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
    ASSERT_TRUE(ListDirectory(root_.string(), {}, /*showHidden=*/false, out));
    ASSERT_FALSE(out.empty());
    EXPECT_TRUE(out.front().isDir); // "sub" directory comes first
    EXPECT_EQ(out.front().name, "sub");
    // No dotfile when showHidden=false.
    for (const auto& e : out)
        EXPECT_NE(e.name, ".hidden");
}

TEST_F(FileDialogFsTest, ListDirectoryShowHidden) {
    std::vector<DirEntry> out;
    ASSERT_TRUE(ListDirectory(root_.string(), {}, /*showHidden=*/true, out));
    bool sawHidden = false;
    for (const auto& e : out)
        if (e.name == ".hidden")
            sawHidden = true;
    EXPECT_TRUE(sawHidden);
}

TEST_F(FileDialogFsTest, ListDirectoryExtensionFilterKeepsDirs) {
    std::vector<DirEntry> out;
    ASSERT_TRUE(ListDirectory(root_.string(), {".csv"}, false, out));
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
    EXPECT_FALSE(ListDirectory((root_ / "does_not_exist").string(), {}, false, out));
    EXPECT_TRUE(out.empty()); // cleared on failure
}

TEST_F(FileDialogFsTest, EntriesRespectMode) {
    FileDialog fd;
    fd.SetDirectory(root_.string()).SetFilters({".csv"});

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
    fd.SetDirectory(root_.string());
    EXPECT_EQ(fs::path(fd.GetDirectory()).filename(), "unigui_filedialog_test");

    EXPECT_TRUE(fd.NavigateInto("sub"));
    EXPECT_EQ(fs::path(fd.GetDirectory()).filename(), "sub");
    EXPECT_FALSE(fd.NavigateInto("nope")); // not a directory

    EXPECT_TRUE(fd.NavigateUp());
    EXPECT_EQ(fs::path(fd.GetDirectory()).filename(), "unigui_filedialog_test");
}

TEST_F(FileDialogFsTest, ResolvedPathPerMode) {
    FileDialog fd;
    fd.SetDirectory(root_.string());

    // OpenFile: empty until a file is selected.
    fd.SetMode(FileDialog::Mode::OpenFile);
    EXPECT_TRUE(fd.ResolvedPath().empty());
    fd.SelectFile("a.csv");
    EXPECT_EQ(fs::path(fd.ResolvedPath()).filename(), "a.csv");

    // SaveFile: derives from the filename field.
    fd.SetMode(FileDialog::Mode::SaveFile);
    fd.SetFilename("");
    EXPECT_TRUE(fd.ResolvedPath().empty());
    fd.SetFilename("out.csv");
    EXPECT_EQ(fs::path(fd.ResolvedPath()).filename(), "out.csv");

    // SelectFolder: the current directory itself.
    fd.SetMode(FileDialog::Mode::SelectFolder);
    EXPECT_EQ(fs::path(fd.ResolvedPath()).filename(), "unigui_filedialog_test");
}

TEST_F(FileDialogFsTest, ConfirmFiresCallbackAndCloses) {
    FileDialog fd;
    fd.SetDirectory(root_.string()).SetMode(FileDialog::Mode::OpenFile);
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
    fd.SetDirectory(root_.string()).SetMode(FileDialog::Mode::SaveFile).SetFilename("x.csv");
    fd.Open();

    for (int frame = 0; frame < 3; ++frame) {
        ImGui::NewFrame();
        EXPECT_NO_THROW(fd.Render());
        ImGui::Render();
    }
    ImGui::DestroyContext();
}
