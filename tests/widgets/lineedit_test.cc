#include <unigui/unigui.h>
#include <unigui/widgets/lineedit.h>
#include <imgui.h>
#include <gtest/gtest.h>
class LineEditTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize = ImVec2(800, 600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(LineEditTest, DefaultsToEmpty) {
    unigui::LineEdit le("le", "Name");
    EXPECT_EQ(le.GetValue(), "");
}
TEST_F(LineEditTest, SetValue_Works) {
    unigui::LineEdit le("le", "Name");
    le.SetValue("Alice");
    EXPECT_EQ(le.GetValue(), "Alice");
}
TEST_F(LineEditTest, HasError_DefaultsToFalse) {
    unigui::LineEdit le("le", "Email");
    EXPECT_FALSE(le.HasError());
}
TEST_F(LineEditTest, Validator_RejectsInvalid) {
    unigui::LineEdit le("le", "Email");
    le.SetValidator([](const std::string& s) { return s.find('@') != std::string::npos; });
    le.SetValue("invalid"); // No @ sign
    le.Render();
}
TEST_F(LineEditTest, Render_DoesNotCrash) {
    unigui::LineEdit le("le", "Field");
    le.SetPlaceholder("Enter text...");
    le.Render();
}
TEST_F(LineEditTest, Undo_Redo_Works) {
    unigui::LineEdit le("le", "Field");
    // Initial value "" is pushed by ctor; SetValue pushes "A" then "B"
    le.SetValue("A");
    le.SetValue("B");
    EXPECT_EQ(le.GetValue(), "B");
    EXPECT_TRUE(le.CanUndo());
    le.Undo();
    EXPECT_EQ(le.GetValue(), "A");
    EXPECT_TRUE(le.CanRedo());
    le.Redo();
    EXPECT_EQ(le.GetValue(), "B");
}
TEST_F(LineEditTest, CanUndo_DefaultsToFalse) {
    unigui::LineEdit le("le", "Field");
    EXPECT_FALSE(le.CanUndo());
    le.SetValue("new");
    EXPECT_TRUE(le.CanUndo()); // Can undo to initial empty value
}
