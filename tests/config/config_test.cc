#include <unigui/config/config.h>

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <utility>
#include <vector>
using namespace unigui::config;
class StoreTest : public ::testing::Test {
protected:
    void TearDown() override { Store::Instance().Clear(); }
};
TEST_F(StoreTest, SetGet_String) {
    Store::Instance().SetString("k", "v");
    EXPECT_EQ(Store::Instance().GetString("k"), "v");
}
TEST_F(StoreTest, SetGet_Int) {
    Store::Instance().SetInt("n", 42);
    EXPECT_EQ(Store::Instance().GetInt("n"), 42);
}
TEST_F(StoreTest, Has_Works) {
    Store::Instance().SetInt("a", 1);
    EXPECT_TRUE(Store::Instance().Has("a"));
}

// ── Error-path: malformed stored values must fall back to defaults ──────────
TEST_F(StoreTest, GetInt_Garbage_ReturnsDefault) {
    Store::Instance().SetString("n", "not-a-number");
    EXPECT_EQ(Store::Instance().GetInt("n", 7), 7);
}
TEST_F(StoreTest, GetInt_Empty_ReturnsDefault) {
    Store::Instance().SetString("n", "");
    EXPECT_EQ(Store::Instance().GetInt("n", -1), -1);
}
TEST_F(StoreTest, GetInt_Whitespace_ReturnsDefault) {
    Store::Instance().SetString("n", "   ");
    EXPECT_EQ(Store::Instance().GetInt("n", 5), 5);
}
TEST_F(StoreTest, GetDouble_Garbage_ReturnsDefault) {
    Store::Instance().SetString("d", "abc");
    EXPECT_DOUBLE_EQ(Store::Instance().GetDouble("d", 3.14), 3.14);
}
TEST_F(StoreTest, GetDouble_Empty_ReturnsDefault) {
    Store::Instance().SetString("d", "");
    EXPECT_DOUBLE_EQ(Store::Instance().GetDouble("d", 2.71), 2.71);
}
TEST_F(StoreTest, GetInt_LargeNumber_Parses) {
    Store::Instance().SetString("n", "999999");
    EXPECT_EQ(Store::Instance().GetInt("n"), 999999);
}

// ── Load* now return Result<void> (4.0): distinguish missing file from bad parse ──
TEST_F(StoreTest, LoadJSON_Missing_ReturnsFileNotFound) {
    const auto r = Store::Instance().LoadJSON("/no_such_dir_unigui/missing.json");
    ASSERT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), unigui::ErrorCode::FileNotFound);
}

TEST_F(StoreTest, LoadJSON_Malformed_ReturnsParseFailed) {
    const auto p = std::filesystem::temp_directory_path() / "unigui_cfg_bad.json";
    std::ofstream(p) << "{ this is not valid json ";
    const auto r = Store::Instance().LoadJSON(p.string());
    EXPECT_FALSE(r.has_value());
    EXPECT_EQ(r.error(), unigui::ErrorCode::ParseFailed);
    std::filesystem::remove(p);
}

TEST_F(StoreTest, LoadJSON_Valid_Succeeds) {
    const auto p = std::filesystem::temp_directory_path() / "unigui_cfg_ok.json";
    std::ofstream(p) << R"({"city":"NYC","n":5})";
    const auto r = Store::Instance().LoadJSON(p.string());
    ASSERT_TRUE(r.has_value());
    EXPECT_EQ(Store::Instance().GetString("city"), "NYC");
    std::filesystem::remove(p);
}

// ── Save round-trips (regressions for the module review) ────────────────────

// An empty value used to serialize as `key = ` (invalid TOML), which made the WHOLE
// file fail to re-parse — one empty value lost all config.
TEST_F(StoreTest, SaveTOML_EmptyValue_RoundTrips) {
    auto& s = Store::Instance();
    s.SetString("empty", "");
    s.SetString("name", "hello");
    const auto p = std::filesystem::temp_directory_path() / "unigui_cfg_empty.toml";
    ASSERT_TRUE(s.SaveTOML(p.string()));
    s.Clear();
    ASSERT_TRUE(Store::Instance().LoadTOML(p.string()).has_value()); // must still parse
    EXPECT_EQ(Store::Instance().GetString("name"), "hello");
    EXPECT_EQ(Store::Instance().GetString("empty"), "");
    std::filesystem::remove(p);
}

// Keys with TOML-significant characters (spaces, '=', quotes) reach SaveTOML via LoadINI
// and must be quoted+escaped so the file still round-trips.
TEST_F(StoreTest, SaveTOML_SpecialCharKey_RoundTrips) {
    auto& s = Store::Instance();
    s.SetString("a key with spaces", "v1");
    s.SetString("has\"quote", "v2");
    const auto p = std::filesystem::temp_directory_path() / "unigui_cfg_keys.toml";
    ASSERT_TRUE(s.SaveTOML(p.string()));
    s.Clear();
    ASSERT_TRUE(Store::Instance().LoadTOML(p.string()).has_value());
    EXPECT_EQ(Store::Instance().GetString("a key with spaces"), "v1");
    EXPECT_EQ(Store::Instance().GetString("has\"quote"), "v2");
    std::filesystem::remove(p);
}

// A value above 32-bit INT range was truncated by strtol+static_cast<int> on save
// (Windows `long` is 32-bit). It must survive a JSON round-trip intact.
TEST_F(StoreTest, SaveJSON_LargeInt_RoundTrips) {
    auto& s = Store::Instance();
    s.SetString("big", "3000000000"); // > INT_MAX, fits in int64
    const auto p = std::filesystem::temp_directory_path() / "unigui_cfg_big.json";
    ASSERT_TRUE(s.SaveJSON(p.string()));
    s.Clear();
    ASSERT_TRUE(Store::Instance().LoadJSON(p.string()).has_value());
    EXPECT_EQ(Store::Instance().GetString("big"), "3000000000");
    std::filesystem::remove(p);
}

// SaveJSON used to return true even when the file could not be opened.
TEST_F(StoreTest, SaveJSON_BadPath_ReturnsFalse) {
    Store::Instance().SetString("k", "v");
    EXPECT_FALSE(Store::Instance().SaveJSON("/no_such_dir_unigui/nested/out.json"));
}

// strtod accepts these, but cpptoml's bare-number grammar does not (or changes them) —
// emitting them bare made the WHOLE file fail to re-parse, or altered the value. They must
// be quoted so the file still loads and each value survives verbatim.
TEST_F(StoreTest, SaveTOML_NonCanonicalNumbers_RoundTripAsStrings) {
    auto& s = Store::Instance();
    const std::vector<std::pair<std::string, std::string>> cases = {
        {"zip", "007"},  {"half", ".5"}, {"huge", "1e9999"},
        {"hex", "0x10"}, {"plus", "+5"}, {"pi", "3.14"},
    };
    for (auto& [k, v] : cases)
        s.SetString(k, v);
    const auto p = std::filesystem::temp_directory_path() / "unigui_cfg_nums.toml";
    ASSERT_TRUE(s.SaveTOML(p.string()));
    s.Clear();
    ASSERT_TRUE(Store::Instance().LoadTOML(p.string()).has_value()); // must still parse
    for (auto& [k, v] : cases)
        EXPECT_EQ(Store::Instance().GetString(k), v) << "key " << k;
    std::filesystem::remove(p);
}

// Canonical integers are still emitted bare and must round-trip to the identical text.
TEST_F(StoreTest, SaveTOML_CanonicalInt_RoundTrips) {
    auto& s = Store::Instance();
    s.SetString("answer", "42");
    s.SetString("neg", "-7");
    s.SetString("zero", "0");
    const auto p = std::filesystem::temp_directory_path() / "unigui_cfg_int.toml";
    ASSERT_TRUE(s.SaveTOML(p.string()));
    s.Clear();
    ASSERT_TRUE(Store::Instance().LoadTOML(p.string()).has_value());
    EXPECT_EQ(Store::Instance().GetString("answer"), "42");
    EXPECT_EQ(Store::Instance().GetString("neg"), "-7");
    EXPECT_EQ(Store::Instance().GetString("zero"), "0");
    std::filesystem::remove(p);
}
