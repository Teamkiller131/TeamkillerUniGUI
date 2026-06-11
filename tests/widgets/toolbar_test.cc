#include <unigui/unigui.h>
#include <unigui/widgets/toolbar.h>

#include <imgui.h>

#include <gtest/gtest.h>
class ToolBarTest : public ::testing::Test {
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
TEST_F(ToolBarTest, Render_DoesNotCrash) {
    unigui::ToolBar tb("tb");
    tb.SetItems({{"Save", [] {}}});
    tb.Render();
}
