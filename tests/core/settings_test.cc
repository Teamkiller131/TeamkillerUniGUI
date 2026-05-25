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
