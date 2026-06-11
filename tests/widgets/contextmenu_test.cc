#include <unigui/unigui.h>
#include <unigui/widgets/contextmenu.h>

#include <imgui.h>

#include <gtest/gtest.h>
class ContextMenuTest : public ::testing::Test {
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
TEST_F(ContextMenuTest, Ctor_DoesNotCrash) {
    unigui::ContextMenu::Show("id", {{"Item", [] {}}});
    SUCCEED();
}
