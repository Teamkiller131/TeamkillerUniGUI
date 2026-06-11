#include <unigui/unigui.h>
#include <unigui/widgets/separator.h>

#include <imgui.h>

#include <gtest/gtest.h>
class SeparatorTest : public ::testing::Test {
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
TEST_F(SeparatorTest, Render_DoesNotCrash) {
    unigui::Separator sep("sep", "Section");
    sep.Render();
}
