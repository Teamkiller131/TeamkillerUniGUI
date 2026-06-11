#include <unigui/unigui.h>
#include <unigui/widgets/radiogroup.h>

#include <imgui.h>

#include <gtest/gtest.h>
class RadioGroupTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};
TEST_F(RadioGroupTest, DefaultsToFirstOption) {
    unigui::RadioGroup rg("rg", {"A", "B", "C"});
    EXPECT_EQ(rg.GetSelected(), 0);
}
TEST_F(RadioGroupTest, SetSelected_Works) {
    unigui::RadioGroup rg("rg", {"A", "B", "C"});
    rg.SetSelected(2);
    EXPECT_EQ(rg.GetSelected(), 2);
}
TEST_F(RadioGroupTest, GetOptions_ReturnsOptions) {
    unigui::RadioGroup rg("rg", {"X", "Y"});
    EXPECT_EQ(rg.GetOptions().size(), 2u);
}
TEST_F(RadioGroupTest, Render_DoesNotCrash) {
    unigui::RadioGroup rg("rg", {"A", "B"});
    rg.Render();
}
