#include <unigui/unigui.h>
#include <unigui/widgets/tabwidget.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>
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

// ── Re-entrancy: structural mutation issued from inside a tab's content_callback
//    must be deferred, not applied mid-iteration. The old code held `auto& tab =
//    tabs_[i]` across the callback; an AddTab (reallocation) or RemoveTab (erase)
//    from the callback freed the executing std::function — a use-after-free. ──
TEST_F(TabWidgetTest, AddTabFromCallback_DeferredNoCrash) {
    unigui::TabWidget tw("tw");
    bool added = false;
    tw.AddTab({"t1", "Tab 1", [&]() {
                   if (!added) {
                       added = true;
                       // Force tabs_ to reallocate from within the iteration.
                       for (int i = 0; i < 32; i++)
                           tw.AddTab({"x" + std::to_string(i), "X", []() {}});
                   }
               }});
    EXPECT_NO_THROW(tw.Render());
    // The 32 queued adds are flushed after EndTabBar: 1 original + 32 = 33.
    EXPECT_EQ(tw.GetTabs().size(), 33u);
}

TEST_F(TabWidgetTest, RemoveActiveTabFromCallback_DeferredNoCrash) {
    unigui::TabWidget tw("tw");
    tw.AddTab({"t1", "Tab 1", [&]() { tw.RemoveTab("t1"); }}); // self-removal from callback
    tw.AddTab({"t2", "Tab 2", []() {}});
    EXPECT_NO_THROW(tw.Render());
    // The active tab (t1) removes itself; the erase is deferred to after the frame.
    EXPECT_EQ(tw.GetTabs().size(), 1u);
    EXPECT_EQ(tw.GetTabs()[0].name, "t2");
}
