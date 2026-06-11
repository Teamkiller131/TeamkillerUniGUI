#include <unigui/unigui.h>
#include <unigui/widgets/listview.h>

#include <imgui.h>

#include <gtest/gtest.h>

class ListViewTest : public ::testing::Test {
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

// ---- basic rendering ----

TEST_F(ListViewTest, Render_DoesNotCrash) {
    unigui::ListView lv("lv", {"A", "B"});
    lv.Render();
}

TEST_F(ListViewTest, Render_EmptyList) {
    unigui::ListView lv("lv", {});
    lv.Render();
}

TEST_F(ListViewTest, Render_EmptyList_AfterConstruction) {
    unigui::ListView lv("lv");
    lv.Render();
}

// ---- default selected ----

TEST_F(ListViewTest, DefaultSelected_IsNegativeOne) {
    unigui::ListView lv("lv", {"A"});
    EXPECT_EQ(lv.GetSelected(), -1);
}

TEST_F(ListViewTest, DefaultSelected_EmptyList) {
    unigui::ListView lv("lv");
    EXPECT_EQ(lv.GetSelected(), -1);
}

// ---- set items ----

TEST_F(ListViewTest, SetItems_Works) {
    unigui::ListView lv("lv");
    lv.SetItems({"X", "Y", "Z"});
    lv.Render();
}

TEST_F(ListViewTest, SetItems_ReplacesPrevious) {
    unigui::ListView lv("lv", {"Old"});
    lv.SetItems({"New1", "New2", "New3"});
    lv.Render();
}

TEST_F(ListViewTest, SetItems_Empty) {
    unigui::ListView lv("lv", {"A", "B"});
    lv.SetItems({});
    lv.Render();
    EXPECT_EQ(lv.GetSelected(), -1);
}

TEST_F(ListViewTest, SetItems_SingleItem) {
    unigui::ListView lv("lv");
    lv.SetItems({"Only"});
    lv.Render();
}

TEST_F(ListViewTest, SetItems_LargeCount) {
    unigui::ListView lv("lv");
    std::vector<std::string> big;
    for (int i = 0; i < 200; ++i) {
        big.push_back("Item " + std::to_string(i));
    }
    lv.SetItems(big);
    lv.Render();
}

// ---- selection callback ----

TEST_F(ListViewTest, SetOnSelect_Registered) {
    unigui::ListView lv("lv", {"A", "B"});
    bool called = false;
    lv.SetOnSelect([&called](int idx) {
        (void) idx;
        called = true;
    });
    lv.Render();
    // Callback fires on user interaction, not on Render
    EXPECT_FALSE(called);
}

TEST_F(ListViewTest, SetOnSelect_NullCallback) {
    unigui::ListView lv("lv", {"A", "B"});
    lv.SetOnSelect(nullptr);
    lv.Render(); // Should not crash
}

// ---- multi-select ----

TEST_F(ListViewTest, MultiSelect_DefaultsToEmpty) {
    unigui::ListView lv("lv", {"A", "B", "C"});
    auto selected = lv.GetSelectedItems();
    EXPECT_TRUE(selected.empty());
}

TEST_F(ListViewTest, SetMultiSelect_DoesNotCrash) {
    unigui::ListView lv("lv", {"A", "B", "C"});
    lv.SetMultiSelect(true);
    lv.Render();
}

TEST_F(ListViewTest, SetMultiSelect_Toggle) {
    unigui::ListView lv("lv", {"A", "B"});
    lv.SetMultiSelect(true);
    lv.Render();
    lv.SetMultiSelect(false);
    lv.Render();
}

// ---- visibility ----

TEST_F(ListViewTest, Hidden_DoesNotRender) {
    unigui::ListView lv("lv", {"A", "B"});
    lv.Hide();
    lv.Render();
    EXPECT_FALSE(lv.IsVisible());
}

TEST_F(ListViewTest, Show_AfterHide) {
    unigui::ListView lv("lv", {"A"});
    lv.Hide();
    EXPECT_FALSE(lv.IsVisible());
    lv.Show();
    EXPECT_TRUE(lv.IsVisible());
    lv.Render();
}

// ---- base Widget features ----

TEST_F(ListViewTest, GetName_ReturnsName) {
    unigui::ListView lv("list_id");
    EXPECT_EQ(lv.GetName(), "list_id");
}

TEST_F(ListViewTest, Tooltip_DoesNotCrash) {
    unigui::ListView lv("lv", {"A", "B"});
    lv.SetTooltip("List tooltip");
    lv.Render();
}
