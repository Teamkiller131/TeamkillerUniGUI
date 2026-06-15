#include <unigui/widgets/segmentedcontrol.h>

#include <imgui.h>

#include <gtest/gtest.h>

class SegmentedControlTest : public ::testing::Test {
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

using unigui::SegmentedControl;

TEST_F(SegmentedControlTest, Render_EmptyDoesNotCrash) {
    SegmentedControl s("seg");
    EXPECT_NO_THROW(s.Render());
    EXPECT_EQ(s.SegmentCount(), 0u);
}

TEST_F(SegmentedControlTest, Constructor_TakesSegments) {
    SegmentedControl s("seg", {"1D", "1W", "1M"});
    EXPECT_EQ(s.SegmentCount(), 3u);
    EXPECT_EQ(s.GetSelected(), 0);
    EXPECT_EQ(s.GetSelectedLabel(), "1D");
}

TEST_F(SegmentedControlTest, AddSegment_Appends) {
    SegmentedControl s("seg");
    s.AddSegment("A");
    s.AddSegment("B");
    EXPECT_EQ(s.SegmentCount(), 2u);
    EXPECT_EQ(s.GetSegments()[1], "B");
}

TEST_F(SegmentedControlTest, SetSelected_ValidIndex) {
    SegmentedControl s("seg", {"A", "B", "C"});
    s.SetSelected(2);
    EXPECT_EQ(s.GetSelected(), 2);
    EXPECT_EQ(s.GetSelectedLabel(), "C");
}

TEST_F(SegmentedControlTest, SetSelected_OutOfRangeIgnored) {
    SegmentedControl s("seg", {"A", "B"});
    s.SetSelected(5);
    EXPECT_EQ(s.GetSelected(), 0); // unchanged
    s.SetSelected(-1);
    EXPECT_EQ(s.GetSelected(), 0);
}

TEST_F(SegmentedControlTest, SetSegments_ClampsSelection) {
    SegmentedControl s("seg", {"A", "B", "C"});
    s.SetSelected(2);
    s.SetSegments({"X"}); // shrink — selection must clamp into range
    EXPECT_EQ(s.GetSelected(), 0);
    EXPECT_EQ(s.GetSelectedLabel(), "X");
}

TEST_F(SegmentedControlTest, Clear_ResetsState) {
    SegmentedControl s("seg", {"A", "B"});
    s.SetSelected(1);
    s.Clear();
    EXPECT_EQ(s.SegmentCount(), 0u);
    EXPECT_EQ(s.GetSelected(), 0);
    EXPECT_EQ(s.GetSelectedLabel(), "");
}

TEST_F(SegmentedControlTest, Render_DoesNotCrash) {
    SegmentedControl s("seg", {"1D", "1W", "1M", "1Y"});
    s.SetSelected(1);
    EXPECT_NO_THROW(s.Render());
}

TEST_F(SegmentedControlTest, FillWidth_RendersWithoutCrash) {
    SegmentedControl s("seg", {"Left", "Right"});
    s.SetFillWidth(true);
    EXPECT_NO_THROW(s.Render());
}

TEST_F(SegmentedControlTest, Fluent_ChainsAndKeepsType) {
    SegmentedControl s("seg");
    int changed = -1;
    SegmentedControl& ref =
        s.WithSegments({"A", "B", "C"}).WithSelected(2).WithFillWidth().WithOnChange(
            [&](int i, const std::string&) { changed = i; });
    EXPECT_EQ(&ref, &s);
    EXPECT_EQ(s.GetSelected(), 2);
    EXPECT_EQ(s.SegmentCount(), 3u);
    (void) changed;
}
