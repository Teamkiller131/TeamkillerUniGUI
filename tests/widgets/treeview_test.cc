#include <unigui/core/accessibility.h>
#include <unigui/unigui.h>
#include <unigui/widgets/treeview.h>

#include <imgui.h>

#include <gtest/gtest.h>
class TreeViewTest : public ::testing::Test {
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
TEST_F(TreeViewTest, Render_DoesNotCrash) {
    unigui::TreeView tv("tv");
    tv.Render();
}
TEST_F(TreeViewTest, SetRoot_Works) {
    unigui::TreeView tv("tv");
    tv.SetRoot({"Root", {{"Child1", {}}, {"Child2", {}}}});
    tv.Render();
}
TEST_F(TreeViewTest, RowRenderer_DoesNotCrash) {
    unigui::TreeView tv("tv");
    tv.SetRoot({"Root", {{"A", {}}, {"B", {}}}});
    tv.SetRowRenderer([](int, int, const unigui::TreeNode& node, bool) {
        ImGui::Text("%s", node.label.c_str());
    });
    tv.Render();
}
TEST_F(TreeViewTest, EnhancedFields_Render) {
    unigui::TreeView tv("tv");
    unigui::TreeNode root;
    root.label = "Root";
    root.icon = "\xf0\x9f\x93\x81"; // folder emoji
    root.suffix = "(3)";
    root.labelColor = IM_COL32(255, 0, 0, 255);
    root.bgColor = IM_COL32(40, 40, 40, 255);
    root.progress = 0.75f;
    root.progressColor = IM_COL32(0, 255, 0, 255);
    root.children.push_back({"Child", {}});
    tv.SetRoot(std::move(root));
    tv.Render();
}

TEST_F(TreeViewTest, RowRenderer_RendersClosedNodeRow) {
    unigui::TreeView tv("tv");
    tv.SetRoot({"Root", {{"A", {}}, {"B", {}}}});
    int rendered = 0;
    tv.SetRowRenderer([&](int, int, const unigui::TreeNode& node, bool) {
        ++rendered;
        ImGui::Text("%s", node.label.c_str());
    });
    tv.Render();
    EXPECT_GT(rendered, 0);
}

TEST_F(TreeViewTest, DuplicateLabels_RenderWithCustomRows) {
    unigui::TreeView tv("tv");
    tv.SetRoot({"Root", {{"Node", {}}, {"Node", {}}}});
    tv.SetRowRenderer([](int, int, const unigui::TreeNode& node, bool) {
        ImGui::Text("%s", node.label.c_str());
    });
    tv.Render();
    SUCCEED();
}

// ── Accessibility: container + per-node registration ─────────────────────────
class TreeViewA11yTest : public TreeViewTest {
protected:
    void SetUp() override {
        TreeViewTest::SetUp();
        unigui::a11y::SetEnabled(true);
        unigui::a11y::BeginFrame();
        unigui::a11y::DrainAnnouncements();
    }
    void TearDown() override {
        unigui::a11y::SetEnabled(false);
        TreeViewTest::TearDown();
    }
};

TEST_F(TreeViewA11yTest, Container_AndVisibleNodes_Register) {
    unigui::TreeView tv("tv_a11y");
    tv.SetRoot({"Root", {{"Child1", {}}, {"Child2", {}}}});
    tv.Render();
    bool sawTree = false, sawRootNode = false;
    for (const auto& n : unigui::a11y::Tree()) {
        if (n.role == unigui::a11y::Role::Tree) {
            sawTree = true;
            EXPECT_EQ(n.value, "Root"); // no selection → container value falls back to root
        }
        if (n.role == unigui::a11y::Role::ListItem && n.value == "Root")
            sawRootNode = true; // the visible (collapsed) root registered as a node
    }
    EXPECT_TRUE(sawTree);
    EXPECT_TRUE(sawRootNode);
}
