#include <unigui/unigui.h>
#include <unigui/widgets/selectable.h>
#include <imgui.h>
#include <gtest/gtest.h>

class SelectableTest : public ::testing::Test {
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

TEST_F(SelectableTest, DefaultsToNotSelected) {
    unigui::Selectable sel("sel", "Option");
    EXPECT_FALSE(sel.IsSelected());
}

TEST_F(SelectableTest, Constructor_RespectsInitialSelected) {
    unigui::Selectable sel("sel", "Option", true);
    EXPECT_TRUE(sel.IsSelected());
}

TEST_F(SelectableTest, SetSelected_Works) {
    unigui::Selectable sel("sel", "Option");
    sel.SetSelected(true);
    EXPECT_TRUE(sel.IsSelected());
    sel.SetSelected(false);
    EXPECT_FALSE(sel.IsSelected());
}

TEST_F(SelectableTest, GetLabel_ReturnsLabel) {
    unigui::Selectable sel("sel", "Hello World");
    EXPECT_EQ(sel.GetLabel(), "Hello World");
}

TEST_F(SelectableTest, Render_DoesNotCrash) {
    unigui::Selectable sel("sel", "Opt");
    sel.Render();
}

TEST_F(SelectableTest, WasClicked_InitiallyFalse) {
    unigui::Selectable sel("sel", "Opt");
    EXPECT_FALSE(sel.WasClicked());
}

TEST_F(SelectableTest, Render_TracksClickState) {
    unigui::Selectable sel("sel", "Opt");
    sel.Render();
    // In unit test mode without actual interaction, clicked_ will be false
    EXPECT_FALSE(sel.WasClicked());
}
