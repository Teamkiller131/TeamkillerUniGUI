#include <unigui/unigui.h>
#include <unigui/widgets/combobox.h>
#include <imgui.h>
#include <gtest/gtest.h>
class ComboBoxTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize = ImVec2(800, 600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(ComboBoxTest, DefaultsToFirstItem) {
    unigui::ComboBox cb("cb", "Select", {"A", "B", "C"});
    EXPECT_EQ(cb.GetSelectedIndex(), 0);
    EXPECT_EQ(cb.GetSelectedValue(), "A");
}
TEST_F(ComboBoxTest, SetSelectedIndex_Works) {
    unigui::ComboBox cb("cb", "Select", {"X", "Y", "Z"});
    cb.SetSelectedIndex(2);
    EXPECT_EQ(cb.GetSelectedIndex(), 2);
    EXPECT_EQ(cb.GetSelectedValue(), "Z");
}
TEST_F(ComboBoxTest, GetItems_ReturnsItems) {
    unigui::ComboBox cb("cb", "Items", {"One", "Two"});
    EXPECT_EQ(cb.GetItems().size(), 2u);
}
TEST_F(ComboBoxTest, Render_DoesNotCrash) {
    unigui::ComboBox cb("cb", "Choose", {"A", "B"});
    cb.Render();
}
TEST_F(ComboBoxTest, SetItemIcon_DoesNotCrash) {
    unigui::ComboBox cb("cb", "Icons", {"A", "B"});
    cb.SetItemIcon(0, (ImTextureID)(uintptr_t)1);
    EXPECT_NE(cb.GetItemIcon(0), (ImTextureID)0);
    EXPECT_EQ(cb.GetItemIcon(1), (ImTextureID)0);
    cb.Render();
}
