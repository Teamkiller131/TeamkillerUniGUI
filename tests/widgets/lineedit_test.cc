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
