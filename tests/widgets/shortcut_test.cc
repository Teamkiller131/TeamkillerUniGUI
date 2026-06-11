#include <unigui/unigui.h>
#include <unigui/widgets/shortcut.h>

#include <imgui.h>

#include <gtest/gtest.h>
class ShortcutManagerTest : public ::testing::Test {
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
TEST_F(ShortcutManagerTest, Register_DoesNotCrash) {
    unigui::ShortcutManager sm;
    sm.Register(ImGuiKey_A, true, [] {}, "");
    sm.Process();
    SUCCEED();
}
