#include <unigui/unigui.h>
#include <unigui/widgets/breadcrumb.h>

#include <imgui.h>

#include <gtest/gtest.h>
class BreadcrumbTest : public ::testing::Test {
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
TEST_F(BreadcrumbTest, Render_DoesNotCrash) {
    unigui::Breadcrumb bc("bc");
    bc.SetItems({"Home", "Settings"});
    bc.Render();
}
TEST_F(BreadcrumbTest, GetSelected_DefaultsNegativeOne) {
    unigui::Breadcrumb bc("bc");
    EXPECT_EQ(bc.GetSelected(), -1);
}
