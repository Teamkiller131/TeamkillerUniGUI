#include <unigui/unigui.h>
#include <unigui/widgets/richtext.h>

#include <imgui.h>

#include <gtest/gtest.h>
class RichTextTest : public ::testing::Test {
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
TEST_F(RichTextTest, Render_DoesNotCrash) {
    unigui::RichText rt("rt", "Hello");
    rt.Render();
}
TEST_F(RichTextTest, SetSpans_Renders) {
    unigui::RichText rt("rt");
    rt.SetSpans(
        {{"Bold", true, false, ImVec4(1, 1, 1, 1)}, {" Italic", false, true, ImVec4(1, 0, 0, 1)}});
    rt.Render();
}
