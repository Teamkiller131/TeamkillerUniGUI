#include <unigui/unigui.h>
#include <unigui/widgets/toast.h>

#include <imgui.h>

#include <gtest/gtest.h>
class ToastTest : public ::testing::Test {
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
TEST_F(ToastTest, Show_DoesNotCrash) {
    unigui::Toast::Info("Test");
}
TEST_F(ToastTest, Render_DoesNotCrash) {
    unigui::Toast::Info("Hello");
    unigui::Toast::Instance().Render();
}
