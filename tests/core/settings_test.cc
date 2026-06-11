#include <unigui/unigui.h>
#include <unigui/core/settings.h>
#include <gtest/gtest.h>
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
    EXPECT_NO_THROW({ (void)s.GetInt("n", 7); });
    EXPECT_EQ(s.GetInt("n", 7), 7);
}

TEST_F(SettingsTest, EmptyFloat_ReturnsDefault) {
    auto& s = unigui::Settings::Instance();
    s.Set("f", "");
    EXPECT_NO_THROW({ (void)s.GetFloat("f", 1.5f); });
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
