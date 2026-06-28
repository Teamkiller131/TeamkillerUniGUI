#include <unigui/config/config.h>

#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
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
