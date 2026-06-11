#include <unigui/fonts/font_manager.h>

#include <imgui.h>

#include <gtest/gtest.h>
using namespace unigui::fonts;

class FontTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(FontTest, List_EmptyByDefault) {
    EXPECT_TRUE(Manager::Instance().List().empty());
}

TEST_F(FontTest, Get_Nonexistent_ReturnsNull) {
    EXPECT_EQ(Manager::Instance().Get("nonexistent"), nullptr);
}

TEST_F(FontTest, Unload_Nonexistent_ReturnsFalse) {
    EXPECT_FALSE(Manager::Instance().Unload("nope"));
}

TEST_F(FontTest, SetDefault_DoesNotCrash) {
    Manager::Instance().SetDefault("nonexistent"); // should not crash
    SUCCEED();
}
