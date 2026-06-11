#include <unigui/unigui.h>
#include <unigui/widgets/listbox.h>

#include <imgui.h>

#include <gtest/gtest.h>

class ListBoxTest : public ::testing::Test {
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

TEST_F(ListBoxTest, DefaultsToNoSelection) {
    unigui::ListBox lb("lb", "Choose", {"A", "B", "C"});
    EXPECT_EQ(lb.GetSelectedIndex(), -1);
    EXPECT_EQ(lb.GetSelectedValue(), "");
}

TEST_F(ListBoxTest, Constructor_RespectsInitialSelection) {
    unigui::ListBox lb("lb", "Pick", {"X", "Y", "Z"}, 1);
    EXPECT_EQ(lb.GetSelectedIndex(), 1);
    EXPECT_EQ(lb.GetSelectedValue(), "Y");
}

TEST_F(ListBoxTest, SetSelectedIndex_Works) {
    unigui::ListBox lb("lb", "Items", {"One", "Two", "Three"});
    lb.SetSelectedIndex(2);
    EXPECT_EQ(lb.GetSelectedIndex(), 2);
    EXPECT_EQ(lb.GetSelectedValue(), "Three");
}

TEST_F(ListBoxTest, GetSelectedValue_EmptyWhenOutOfRange) {
    unigui::ListBox lb("lb", "Vals", {"A"});
    lb.SetSelectedIndex(5);
    EXPECT_EQ(lb.GetSelectedValue(), "");
}

TEST_F(ListBoxTest, GetItems_ReturnsItems) {
    unigui::ListBox lb("lb", "Items", {"Foo", "Bar"});
    EXPECT_EQ(lb.GetItems().size(), 2u);
    EXPECT_EQ(lb.GetItems()[0], "Foo");
    EXPECT_EQ(lb.GetItems()[1], "Bar");
}

TEST_F(ListBoxTest, SetItems_ReplacesItems) {
    unigui::ListBox lb("lb", "Items", {"Old"});
    lb.SetItems({"New1", "New2"});
    EXPECT_EQ(lb.GetItems().size(), 2u);
    EXPECT_EQ(lb.GetItems()[0], "New1");
}

TEST_F(ListBoxTest, Render_DoesNotCrash) {
    unigui::ListBox lb("lb", "Choose", {"A", "B", "C"});
    lb.Render();
}

TEST_F(ListBoxTest, SetOnChange_CallbackFiredOnRender) {
    unigui::ListBox lb("lb", "Pick", {"A", "B", "C"}, 0);
    int captured = -1;
    lb.SetOnChange([&](int idx) { captured = idx; });
    lb.SetSelectedIndex(2); // This updates prev_selected_ too
    // Render won't fire callback since selected == prev_selected
    lb.Render();
    EXPECT_EQ(captured, -1);
}

TEST_F(ListBoxTest, GetSelectedValue_EmptyWhenNoItems) {
    unigui::ListBox lb("lb", "Empty", {});
    EXPECT_EQ(lb.GetSelectedValue(), "");
}

TEST_F(ListBoxTest, GetSelectedValue_EmptyWhenNegativeIndex) {
    unigui::ListBox lb("lb", "Neg", {"A", "B"}, -1);
    EXPECT_EQ(lb.GetSelectedValue(), "");
}
