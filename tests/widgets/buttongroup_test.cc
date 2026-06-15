#include <unigui/widgets/buttongroup.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui;

class ButtonGroupTest : public ::testing::Test {
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

TEST_F(ButtonGroupTest, Empty_DoesNotCrash) {
    ButtonGroup g("g");
    EXPECT_NO_THROW(g.Render());
    EXPECT_EQ(g.Count(), 0u);
}

TEST_F(ButtonGroupTest, AddButtons_CountsAndRenders) {
    ButtonGroup g("g2");
    g.AddButton("Edit", [] {}).AddTintedButton("Delete", [] {}, theme::Semantic::Danger);
    EXPECT_EQ(g.Count(), 2u);
    EXPECT_NO_THROW(g.Render());
}

TEST_F(ButtonGroupTest, AllAlignments_Render) {
    for (auto a : {ButtonGroup::Align::Left, ButtonGroup::Align::Right, ButtonGroup::Align::Fill}) {
        ButtonGroup g("g3");
        g.AddButton("A", [] {}).AddButton("B", [] {}).WithAlign(a);
        EXPECT_NO_THROW(g.Render());
    }
}

TEST_F(ButtonGroupTest, Clear_Empties) {
    ButtonGroup g("g4");
    g.AddButton("X", [] {});
    g.Clear();
    EXPECT_EQ(g.Count(), 0u);
}
