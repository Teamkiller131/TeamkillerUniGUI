#include <unigui/core/accessibility.h>
#include <unigui/presets/master_detail.h>

#include <imgui.h>

#include <gtest/gtest.h>

class MasterDetailTest : public ::testing::Test {
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

TEST_F(MasterDetailTest, Defaults_NoSelectionAndDefaultSplit) {
    unigui::presets::MasterDetail md("md_defaults");
    EXPECT_EQ(md.GetSelected(), -1);
    EXPECT_FLOAT_EQ(md.GetSplit(), 0.3f);
}

TEST_F(MasterDetailTest, FluentChaining_PreservesDerivedType) {
    unigui::presets::MasterDetail md("md_chain");
    // Base (CRTP) helper first, derived helpers after — the chain only
    // compiles if With* keeps returning MasterDetail&.
    md.WithTooltip("browser")
        .WithItems({"Alpha", "Beta"})
        .WithSplit(0.4f)
        .WithEmptyText("Nothing yet")
        .WithOnSelect([](int) {})
        .WithDetail([](int) {});
    EXPECT_FLOAT_EQ(md.GetSplit(), 0.4f);
    EXPECT_EQ(md.GetSelected(), -1); // configuring items selects nothing
}

TEST_F(MasterDetailTest, WithSplit_ClampsToSplitterRange) {
    unigui::presets::MasterDetail md("md_split");
    md.WithSplit(0.01f);
    EXPECT_FLOAT_EQ(md.GetSplit(), 0.1f);
    md.WithSplit(2.f);
    EXPECT_FLOAT_EQ(md.GetSplit(), 0.9f);
}

TEST_F(MasterDetailTest, SetSelected_ClampsIntoRange) {
    unigui::presets::MasterDetail md("md_clamp");
    md.WithItems({"A", "B", "C"});
    md.SetSelected(1);
    EXPECT_EQ(md.GetSelected(), 1);
    md.SetSelected(99);
    EXPECT_EQ(md.GetSelected(), 2); // clamped to last item
    md.SetSelected(-5);
    EXPECT_EQ(md.GetSelected(), -1); // negative clears
}

TEST_F(MasterDetailTest, SetSelected_DoesNotFireOnSelect) {
    unigui::presets::MasterDetail md("md_prog");
    int fired = 0;
    md.WithItems({"A", "B"}).WithOnSelect([&](int) { ++fired; });
    md.SetSelected(1);
    EXPECT_EQ(md.GetSelected(), 1);
    EXPECT_EQ(fired, 0); // programmatic selection is silent to the callback
}

TEST_F(MasterDetailTest, SetItems_ClampsThenClearsInvalidSelection) {
    unigui::presets::MasterDetail md("md_items");
    md.WithItems({"A", "B", "C"});
    md.SetSelected(2);
    md.SetItems({"A", "B"});
    EXPECT_EQ(md.GetSelected(), 1); // clamped to the shrunk list
    md.SetItems({});
    EXPECT_EQ(md.GetSelected(), -1); // cleared when the list emptied
}

// ── Accessibility ────────────────────────────────────────────────────────────
class MasterDetailA11yTest : public MasterDetailTest {
protected:
    void SetUp() override {
        MasterDetailTest::SetUp();
        unigui::a11y::SetEnabled(true);
        unigui::a11y::BeginFrame();
        unigui::a11y::DrainAnnouncements();
    }
    void TearDown() override {
        unigui::a11y::SetEnabled(false);
        MasterDetailTest::TearDown();
    }
};

TEST_F(MasterDetailA11yTest, SetSelected_AnnouncesSelection) {
    unigui::presets::MasterDetail md("md_a11y");
    md.WithItems({"Alpha", "Beta"});
    md.SetSelected(1);
    bool announced = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements())
        if (a.message == "Beta selected")
            announced = true;
    EXPECT_TRUE(announced);
    md.SetSelected(1); // no change → no repeat announcement
    EXPECT_TRUE(unigui::a11y::DrainAnnouncements().empty());
}

TEST_F(MasterDetailA11yTest, Render_RegistersListItemsAndGroup) {
    unigui::presets::MasterDetail md("md_tree");
    md.WithItems({"Alpha", "Beta"});
    md.Render();
    int listItems = 0, groups = 0;
    for (const auto& n : unigui::a11y::Tree()) {
        if (n.role == unigui::a11y::Role::ListItem)
            ++listItems;
        if (n.role == unigui::a11y::Role::Group)
            ++groups;
    }
    EXPECT_EQ(listItems, 2); // composed ListView registered every item
    EXPECT_GE(groups, 1);    // the preset registered its container
}

// ── Rendering (last: exercises the full composition) ─────────────────────────
TEST_F(MasterDetailTest, Render_WithSelection_InvokesDetailWithIndex) {
    unigui::presets::MasterDetail md("md_detail");
    int received = -100;
    md.WithItems({"A", "B"}).WithDetail([&](int i) { received = i; });
    md.SetSelected(1);
    md.Render();
    EXPECT_EQ(received, 1);
}

TEST_F(MasterDetailTest, Render_DoesNotCrash) {
    unigui::presets::MasterDetail unconfigured("md_bare");
    unconfigured.Render(); // nothing configured: list empty, placeholder pane

    unigui::presets::MasterDetail md("md_render");
    md.WithItems({"A", "B"}).WithEmptyText("Pick one").WithSplit(0.5f);
    md.Render(); // no selection: centred empty text branch
}
