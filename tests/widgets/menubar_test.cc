#include <unigui/unigui.h>
#include <unigui/widgets/menubar.h>

#include <imgui.h>

#include <gtest/gtest.h>
class MenuBarTest : public ::testing::Test {
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
TEST_F(MenuBarTest, Render_DoesNotCrash) {
    unigui::MenuBar mb("mb");
    mb.SetMenus({{"File", {{"Exit", [] {}}}}});
    mb.Render();
}
