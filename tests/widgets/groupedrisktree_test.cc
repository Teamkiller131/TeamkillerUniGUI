#include <unigui/widgets/groupedrisktree.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui;
using RiskNode = GroupedRiskTree::RiskNode;
using Rollup = GroupedRiskTree::Rollup;

class GroupedRiskTreeTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
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

// ── Pure rollup logic (no frame needed) ──────────────────────────────────────

TEST_F(GroupedRiskTreeTest, ComputeRatio_LeafReturnsOwnRatio) {
    RiskNode leaf{"a", 0.42, {}};
    EXPECT_NEAR(GroupedRiskTree::ComputeRatio(leaf, Rollup::Worst), 0.42, 1e-9);
}

TEST_F(GroupedRiskTreeTest, ComputeRatio_WorstTakesMaxChild) {
    RiskNode group{"g", 0.0, {{"a", 0.3, {}}, {"b", 0.9, {}}, {"c", 0.5, {}}}};
    EXPECT_NEAR(GroupedRiskTree::ComputeRatio(group, Rollup::Worst), 0.9, 1e-9);
}

TEST_F(GroupedRiskTreeTest, ComputeRatio_MeanAndSum) {
    RiskNode group{"g", 0.0, {{"a", 0.2, {}}, {"b", 0.4, {}}}};
    EXPECT_NEAR(GroupedRiskTree::ComputeRatio(group, Rollup::Mean), 0.3, 1e-9);
    EXPECT_NEAR(GroupedRiskTree::ComputeRatio(group, Rollup::Sum), 0.6, 1e-9);
}

TEST_F(GroupedRiskTreeTest, ComputeRatio_RecursesThroughDepth) {
    RiskNode tree{"root",
                  0.0,
                  {{"groupA", 0.0, {{"x", 0.1, {}}, {"y", 0.8, {}}}},
                   {"groupB", 0.0, {{"z", 0.5, {}}}}}};
    // Worst rollup: groupA→0.8, groupB→0.5, root→0.8
    EXPECT_NEAR(GroupedRiskTree::ComputeRatio(tree, Rollup::Worst), 0.8, 1e-9);
}

// ── Render ───────────────────────────────────────────────────────────────────

TEST_F(GroupedRiskTreeTest, Render_DoesNotCrash) {
    GroupedRiskTree t("risk");
    t.SetThresholds(0.7, 0.85);
    t.SetData({"Accounts",
               0.0,
               {{"GroupA", 0.0, {{"Acct1", 0.65, {}}, {"Acct2", 0.92, {}}}},
                {"GroupB", 0.0, {{"Acct3", 0.40, {}}}}}});
    EXPECT_NO_THROW(t.Render());
}

TEST_F(GroupedRiskTreeTest, RollupSetting_RoundTrips) {
    GroupedRiskTree t("risk2");
    t.SetRollup(Rollup::Mean);
    EXPECT_EQ(t.GetRollup(), Rollup::Mean);
    EXPECT_NO_THROW(t.Render()); // empty data
}
