#include <unigui/core/settings.h>
#include <unigui/unigui.h>

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>

namespace {
std::string TempCfgPath(const char* tag) {
    auto p =
        std::filesystem::temp_directory_path() / (std::string("unigui_settings_") + tag + ".ini");
    return p.string();
}
} // namespace

class SettingsTest : public ::testing::Test {
protected:
    void TearDown() override { unigui::Settings::Instance().Clear(); }
};
TEST_F(SettingsTest, SetAndGet_String) {
    auto& s = unigui::Settings::Instance();
    s.Set("key", "value");
    EXPECT_EQ(s.Get("key"), "value");
}
TEST_F(SettingsTest, SetAndGet_IntFloat) {
    auto& s = unigui::Settings::Instance();
    s.SetInt("count", 42);
    s.SetFloat("pi", 3.14f);
    s.SetBool("flag", true);
    EXPECT_EQ(s.GetInt("count"), 42);
    EXPECT_NEAR(s.GetFloat("pi"), 3.14f, 0.01f);
    EXPECT_TRUE(s.GetBool("flag"));
}

// ── Error-path: malformed stored values must fall back to the default,
//    not throw (regression for the previous unguarded std::stoi/std::stof). ──
TEST_F(SettingsTest, GarbageInt_ReturnsDefault) {
    auto& s = unigui::Settings::Instance();
    s.Set("n", "not-a-number");
    EXPECT_NO_THROW({ (void) s.GetInt("n", 7); });
    EXPECT_EQ(s.GetInt("n", 7), 7);
}

TEST_F(SettingsTest, EmptyFloat_ReturnsDefault) {
    auto& s = unigui::Settings::Instance();
    s.Set("f", "");
    EXPECT_NO_THROW({ (void) s.GetFloat("f", 1.5f); });
    EXPECT_NEAR(s.GetFloat("f", 1.5f), 1.5f, 0.001f);
}

TEST_F(SettingsTest, MissingKey_ReturnsDefault) {
    EXPECT_EQ(unigui::Settings::Instance().GetInt("nope", -1), -1);
    EXPECT_NEAR(unigui::Settings::Instance().GetFloat("nope", 2.5f), 2.5f, 0.001f);
}

TEST_F(SettingsTest, IntWithTrailingText_ParsesLeadingNumber) {
    auto& s = unigui::Settings::Instance();
    s.Set("n", "42abc");
    EXPECT_EQ(s.GetInt("n", 0), 42);
}

// ── Save/Load round-trip and escaping ────────────────────────────────────────

TEST_F(SettingsTest, SaveLoad_RoundTrip_BasicValues) {
    auto& s = unigui::Settings::Instance();
    s.Set("name", "Alice");
    s.SetInt("count", 7);
    auto path = TempCfgPath("basic");
    ASSERT_TRUE(s.Save(path));
    s.Clear();
    ASSERT_TRUE(s.Load(path));
    EXPECT_EQ(s.Get("name"), "Alice");
    EXPECT_EQ(s.GetInt("count"), 7);
    std::filesystem::remove(path);
}

// Values containing '=', backslashes, and newlines must survive a round-trip.
TEST_F(SettingsTest, SaveLoad_PreservesSpecialCharsInValue) {
    auto& s = unigui::Settings::Instance();
    s.Set("expr", "a=b+c");
    s.Set("winpath", "C:\\x\\y");
    s.Set("multi", "line1\nline2");
    auto path = TempCfgPath("special");
    ASSERT_TRUE(s.Save(path));
    s.Clear();
    ASSERT_TRUE(s.Load(path));
    EXPECT_EQ(s.Get("expr"), "a=b+c");
    EXPECT_EQ(s.Get("winpath"), "C:\\x\\y");
    EXPECT_EQ(s.Get("multi"), "line1\nline2");
    std::filesystem::remove(path);
}

// Regression: a key containing '=' used to corrupt on load (the separator was
// the first raw '=' rather than the first UNescaped one).
TEST_F(SettingsTest, SaveLoad_KeyContainingEquals) {
    auto& s = unigui::Settings::Instance();
    s.Set("a=b", "v");
    auto path = TempCfgPath("keyeq");
    ASSERT_TRUE(s.Save(path));
    s.Clear();
    ASSERT_TRUE(s.Load(path));
    EXPECT_TRUE(s.Has("a=b"));
    EXPECT_EQ(s.Get("a=b"), "v");
    EXPECT_FALSE(s.Has("a\\")); // must not leave a stray-backslash key
    std::filesystem::remove(path);
}

TEST_F(SettingsTest, Load_SkipsCommentsAndBlankLines) {
    auto path = TempCfgPath("comments");
    {
        std::ofstream o(path);
        o << "# a comment\n; another comment\n\nreal=42\n";
    }
    auto& s = unigui::Settings::Instance();
    ASSERT_TRUE(s.Load(path));
    EXPECT_EQ(s.GetInt("real"), 42);
    EXPECT_FALSE(s.Has("# a comment"));
    std::filesystem::remove(path);
}

TEST_F(SettingsTest, Load_ReplacesExistingData) {
    auto& s = unigui::Settings::Instance();
    s.Set("fresh", "1");
    auto path = TempCfgPath("replace");
    ASSERT_TRUE(s.Save(path));
    s.Set("stale", "x"); // present in memory before load
    ASSERT_TRUE(s.Load(path));
    EXPECT_TRUE(s.Has("fresh"));
    EXPECT_FALSE(s.Has("stale")); // Load() replaces rather than merges
    std::filesystem::remove(path);
}

// A second Save must fully replace the file (no stale keys) and leave no .tmp.
TEST_F(SettingsTest, Save_OverwritesExistingFile_NoTempLeftover) {
    auto& s = unigui::Settings::Instance();
    auto path = TempCfgPath("overwrite");
    s.Set("a", "1");
    s.Set("b", "2");
    ASSERT_TRUE(s.Save(path));
    s.Clear();
    s.Set("c", "3");
    ASSERT_TRUE(s.Save(path));
    s.Clear();
    ASSERT_TRUE(s.Load(path));
    EXPECT_EQ(s.Get("c"), "3");
    EXPECT_FALSE(s.Has("a"));
    EXPECT_FALSE(s.Has("b"));
    EXPECT_FALSE(std::filesystem::exists(path + ".tmp"));
    std::filesystem::remove(path);
}

// ── Erase / Keys / ClearRecentFiles gap tolerance ───────────────────────────

TEST_F(SettingsTest, Erase_RemovesKey) {
    auto& s = unigui::Settings::Instance();
    s.Set("x", "1");
    EXPECT_TRUE(s.Has("x"));
    s.Erase("x");
    EXPECT_FALSE(s.Has("x"));
}

TEST_F(SettingsTest, Erase_NonexistentKey_IsNoOp) {
    auto& s = unigui::Settings::Instance();
    EXPECT_NO_THROW(s.Erase("does_not_exist"));
}

TEST_F(SettingsTest, Keys_ReturnsAllKeys) {
    auto& s = unigui::Settings::Instance();
    s.Set("a", "1");
    s.Set("b", "2");
    auto ks = s.Keys();
    EXPECT_GE(ks.size(), 2u);
}

TEST_F(SettingsTest, Keys_PrefixFilter_Works) {
    auto& s = unigui::Settings::Instance();
    s.Set("recent.0", "/a");
    s.Set("recent.1", "/b");
    s.Set("other", "x");
    auto ks = s.Keys("recent.");
    EXPECT_EQ(ks.size(), 2u);
    for (auto& k : ks)
        EXPECT_EQ(k.compare(0, 7, "recent."), 0);
}

TEST_F(SettingsTest, ClearRecentFiles_HandlesGaps) {
    auto& s = unigui::Settings::Instance();
    s.Set("recent.0", "/a");
    // gap: no recent.1
    s.Set("recent.2", "/c");
    s.Set("recent.5", "/f");
    s.ClearRecentFiles();
    EXPECT_FALSE(s.Has("recent.0"));
    EXPECT_FALSE(s.Has("recent.2"));
    EXPECT_FALSE(s.Has("recent.5"));
}

TEST_F(SettingsTest, ClearRecentFiles_NoRecent_IsNoOp) {
    auto& s = unigui::Settings::Instance();
    s.Set("other", "x");
    EXPECT_NO_THROW(s.ClearRecentFiles());
    EXPECT_TRUE(s.Has("other"));
}

// ── Non-ASCII path round-trip (Windows UTF-8) ──────────────────────────────
TEST_F(SettingsTest, SaveLoad_NonAsciiPath_RoundTrip) {
    auto& s = unigui::Settings::Instance();
    s.Set("key", "中文值");
    // Build a UTF-8 path string directly. "配置" = \xE9\x85\x8D\xE7\xBD\xAE
    // std::filesystem::path on POSIX treats char* as UTF-8 natively.
    // On Windows, the path is handed to Utf8ToWide (MultiByteToWideChar CP_UTF8).
    auto utf8Dir = std::string("unigui_\xE9\x85\x8D\xE7\xBD\xAE_test");
    auto base = std::filesystem::temp_directory_path() / utf8Dir;
    std::filesystem::create_directories(base);
    auto filePath = (base / "app.ini").string();
    ASSERT_TRUE(s.Save(filePath));
    s.Clear();
    ASSERT_TRUE(s.Load(filePath));
    EXPECT_EQ(s.Get("key"), "中文值");
    std::filesystem::remove_all(base);
}
