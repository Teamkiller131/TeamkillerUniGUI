#include <unigui/unigui.h>
#include <unigui/widgets/cascadingcombo.h>

#include <imgui.h>

#include <gtest/gtest.h>

class CascadingComboTest : public ::testing::Test {
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

TEST_F(CascadingComboTest, ConstructDefault) {
    unigui::CascadingCombo cc("cc");
    EXPECT_EQ(cc.GetName(), "cc");
    EXPECT_EQ(cc.GetSelectedIndex(0), -1);
    EXPECT_EQ(cc.GetSelectedText(0), "");
}

TEST_F(CascadingComboTest, ConstructWithLevels) {
    std::vector<unigui::CascadingCombo::Level> levels = {{"Region", {"US", "EU", "Asia"}, 1},
                                                         {"Country", {"DE", "FR", "UK"}, 0}};
    unigui::CascadingCombo cc("cc", levels);
    EXPECT_EQ(cc.GetSelectedIndex(0), 1);
    EXPECT_EQ(cc.GetSelectedText(0), "EU");
    EXPECT_EQ(cc.GetSelectedIndex(1), 0);
    EXPECT_EQ(cc.GetSelectedText(1), "DE");
}

TEST_F(CascadingComboTest, GetSelectedIndexOutOfBounds) {
    std::vector<unigui::CascadingCombo::Level> levels = {{"Region", {"US", "EU"}}};
    unigui::CascadingCombo cc("cc", levels);
    EXPECT_EQ(cc.GetSelectedIndex(99), -1);
    EXPECT_EQ(cc.GetSelectedIndex(-1), -1);
}

TEST_F(CascadingComboTest, GetSelectedTextOutOfBounds) {
    std::vector<unigui::CascadingCombo::Level> levels = {{"Region", {"US", "EU"}}};
    unigui::CascadingCombo cc("cc", levels);
    EXPECT_EQ(cc.GetSelectedText(99), "");
}

TEST_F(CascadingComboTest, SetOptions) {
    unigui::CascadingCombo cc("cc", {{{"L1", {"A", "B"}}, {"L2", {"X", "Y", "Z"}}}});
    cc.SetOptions(0, {"P", "Q", "R"});
    EXPECT_EQ(cc.GetSelectedIndex(0), 0);
    EXPECT_EQ(cc.GetSelectedText(0), "P");
}

TEST_F(CascadingComboTest, SetOptionsClampsIndex) {
    unigui::CascadingCombo cc("cc", {{{"L1", {"A", "B", "C"}}}});
    // Force index out of bounds, then set smaller options
    unigui::CascadingCombo cc2("cc2", {{{"L1", {"A", "B", "C"}, 2}}});
    cc2.SetOptions(0, {"X"});
    EXPECT_EQ(cc2.GetSelectedIndex(0), 0);
}

TEST_F(CascadingComboTest, SetLevelsReplaces) {
    unigui::CascadingCombo cc("cc", {{{"Old", {"1", "2"}}}});
    std::vector<unigui::CascadingCombo::Level> newLevels = {{"New", {"A", "B", "C"}, 2}};
    cc.SetLevels(newLevels);
    EXPECT_EQ(cc.GetSelectedIndex(0), 2);
    EXPECT_EQ(cc.GetSelectedText(0), "C");
}

TEST_F(CascadingComboTest, SetOnChangedFires) {
    std::vector<unigui::CascadingCombo::Level> levels = {{"L1", {"A", "B"}}, {"L2", {"X", "Y"}}};
    unigui::CascadingCombo cc("cc", levels);
    int fired_level = -1;
    int fired_index = -1;
    cc.SetOnChanged([&](int level, int index) {
        fired_level = level;
        fired_index = index;
    });
    // Render triggers no change because selectedIndex stays 0
    cc.Render();
    EXPECT_EQ(fired_level, -1);
}

TEST_F(CascadingComboTest, RenderDoesNotCrash) {
    std::vector<unigui::CascadingCombo::Level> levels = {{"Region", {"US", "EU", "Asia"}},
                                                         {"Country", {"DE", "FR", "UK"}}};
    unigui::CascadingCombo cc("cc", levels);
    cc.Render();
    SUCCEED();
}

TEST_F(CascadingComboTest, RenderEmptyLevels) {
    unigui::CascadingCombo cc("cc");
    cc.Render();
    SUCCEED();
}

TEST_F(CascadingComboTest, LayoutDefaultsVertical) {
    unigui::CascadingCombo cc("cc");
    EXPECT_EQ(cc.GetLayout(), unigui::CascadingCombo::Layout::Vertical);
}

TEST_F(CascadingComboTest, SetLayoutHorizontal) {
    unigui::CascadingCombo cc("cc");
    cc.SetLayout(unigui::CascadingCombo::Layout::Horizontal);
    EXPECT_EQ(cc.GetLayout(), unigui::CascadingCombo::Layout::Horizontal);
}

TEST_F(CascadingComboTest, SetItemWidthGlobal) {
    unigui::CascadingCombo cc("cc");
    cc.SetItemWidth(120.f);
    EXPECT_FLOAT_EQ(cc.GetItemWidth(), 120.f);
}

TEST_F(CascadingComboTest, SetSpacing) {
    unigui::CascadingCombo cc("cc");
    cc.SetSpacing(16.f);
    EXPECT_FLOAT_EQ(cc.GetSpacing(), 16.f);
}

TEST_F(CascadingComboTest, FluentConfiguration) {
    unigui::CascadingCombo cc("cc");
    cc.WithLayout(unigui::CascadingCombo::Layout::Horizontal).WithItemWidth(90.f).WithSpacing(8.f);
    EXPECT_EQ(cc.GetLayout(), unigui::CascadingCombo::Layout::Horizontal);
    EXPECT_FLOAT_EQ(cc.GetItemWidth(), 90.f);
    EXPECT_FLOAT_EQ(cc.GetSpacing(), 8.f);
}

TEST_F(CascadingComboTest, RenderHorizontalWithWidths) {
    std::vector<unigui::CascadingCombo::Level> levels = {{"Region", {"US", "EU"}, 0, 100.f},
                                                         {"Country", {"DE", "FR"}}};
    unigui::CascadingCombo cc("cc", levels);
    cc.SetLayout(unigui::CascadingCombo::Layout::Horizontal);
    cc.SetItemWidth(120.f);
    cc.SetSpacing(10.f);
    cc.Render();
    SUCCEED();
}

TEST_F(CascadingComboTest, PerLevelItemWidthOverride) {
    unigui::CascadingCombo cc("cc", {{{"L1", {"A", "B"}}, {"L2", {"X", "Y"}}}});
    cc.SetItemWidth(0, 150.f);
    cc.SetItemWidth(0, -1.f);  // clears override
    cc.SetItemWidth(99, 50.f); // out of bounds, ignored
    cc.Render();
    SUCCEED();
}

TEST_F(CascadingComboTest, AutoWidthLongOptionsRender) {
    unigui::CascadingCombo cc("cc", {{{"L1", {"特别特别长的省份名称", "B"}}}});
    cc.Render();
    SUCCEED();
}
