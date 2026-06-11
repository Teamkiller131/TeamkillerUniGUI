#include <unigui/unigui.h>
#include <unigui/widgets/tabwidget.h>

#include <imgui.h>

#include <gtest/gtest.h>
class TabWidgetTest : public ::testing::Test {
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
TEST_F(TabWidgetTest, Render_DoesNotCrash) {
    unigui::TabWidget tw("tw");
    tw.Render();
}
TEST_F(TabWidgetTest, AddTab_IncreasesCount) {
    unigui::TabWidget tw("tw");
    tw.AddTab({"tab1", "Tab 1", []() {}});
    EXPECT_EQ(tw.GetTabs().size(), 1u);
}
TEST_F(TabWidgetTest, RemoveTab_DecreasesCount) {
    unigui::TabWidget tw("tw");
    tw.AddTab({"t1", "Tab 1", []() {}});
    tw.AddTab({"t2", "Tab 2", []() {}});
    tw.RemoveTab("t1");
    EXPECT_EQ(tw.GetTabs().size(), 1u);
}
TEST_F(TabWidgetTest, GetActiveTab_DefaultsToZero) {
    unigui::TabWidget tw("tw");
    tw.AddTab({"t1", "Tab 1", []() {}});
    EXPECT_EQ(tw.GetActiveTab(), 0);
}
TEST_F(TabWidgetTest, ContentCallback_RendersContent) {
    unigui::TabWidget tw("tw");
    bool called = false;
    tw.AddTab({"t1", "Tab 1", [&]() { called = true; }});
    tw.Render();
    EXPECT_TRUE(called);
}
