#include <unigui/unigui.h>
#include <unigui/widgets/confirmdialog.h>

#include <imgui.h>

#include <gtest/gtest.h>

class ConfirmDialogTest : public ::testing::Test {
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

TEST_F(ConfirmDialogTest, Defaults_NotOpen) {
    unigui::ConfirmDialog dlg("dlg");
    EXPECT_FALSE(dlg.IsOpen());
}

TEST_F(ConfirmDialogTest, Open_SetsOpen) {
    unigui::ConfirmDialog dlg("dlg");
    dlg.Open();
    EXPECT_TRUE(dlg.IsOpen());
}

TEST_F(ConfirmDialogTest, Render_DoesNotCrash) {
    unigui::ConfirmDialog dlg("dlg");
    dlg.Render();
}

TEST_F(ConfirmDialogTest, SetTitle_Works) {
    unigui::ConfirmDialog dlg("dlg");
    dlg.SetTitle("Warning");
    SUCCEED();
}
