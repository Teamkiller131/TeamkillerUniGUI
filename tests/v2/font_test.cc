#include <unigui/v2/font_manager.h>
#include <imgui.h>
#include <gtest/gtest.h>
using namespace unigui::v2;

class FontTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(FontTest, List_EmptyByDefault) {
    EXPECT_TRUE(FontManager::Instance().List().empty());
}

TEST_F(FontTest, Get_Nonexistent_ReturnsNull) {
    EXPECT_EQ(FontManager::Instance().Get("nonexistent"), nullptr);
}

TEST_F(FontTest, Unload_Nonexistent_ReturnsFalse) {
    EXPECT_FALSE(FontManager::Instance().Unload("nope"));
}

TEST_F(FontTest, SetDefault_DoesNotCrash) {
    FontManager::Instance().SetDefault("nonexistent"); // should not crash
    SUCCEED();
}
