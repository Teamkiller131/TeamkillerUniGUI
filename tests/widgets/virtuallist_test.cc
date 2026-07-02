#include <unigui/core/accessibility.h>
#include <unigui/unigui.h>
#include <unigui/widgets/virtuallist.h>

#include <imgui.h>

#include <gtest/gtest.h>
class VirtualListTest : public ::testing::Test {
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
TEST_F(VirtualListTest, Render_DoesNotCrash) {
    unigui::VirtualList vl("vl", 100);
    vl.SetItemGetter([](int i) { return "Item " + std::to_string(i); });
    vl.Render();
}
TEST_F(VirtualListTest, GetCount_ReturnsCount) {
    unigui::VirtualList vl("vl", 42);
    EXPECT_EQ(vl.GetItemCount(), 42);
}

// ── Accessibility: every visible (clipped) item registers in the a11y tree ──
class VirtualListA11yTest : public VirtualListTest {
protected:
    void SetUp() override {
        VirtualListTest::SetUp();
        unigui::a11y::SetEnabled(true);
        unigui::a11y::BeginFrame();
        unigui::a11y::DrainAnnouncements();
    }
    void TearDown() override {
        unigui::a11y::SetEnabled(false);
        VirtualListTest::TearDown();
    }
};

TEST_F(VirtualListA11yTest, VisibleItems_RegisterAsListItems) {
    unigui::VirtualList vl("vl_a11y", 1000);
    vl.SetItemGetter([](int i) { return "Item " + std::to_string(i); });
    vl.Render();
    int listItems = 0;
    bool sawFirst = false;
    for (const auto& n : unigui::a11y::Tree()) {
        if (n.role == unigui::a11y::Role::ListItem) {
            ++listItems;
            if (n.value == "Item 0")
                sawFirst = true;
        }
    }
    EXPECT_GT(listItems, 0);    // the visible screenful registered
    EXPECT_LT(listItems, 1000); // ...but NOT all 1000 (clipper bounds the work)
    EXPECT_TRUE(sawFirst);      // top of the list is visible and named
}
