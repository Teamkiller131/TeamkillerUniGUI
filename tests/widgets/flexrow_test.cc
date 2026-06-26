// Layout::FlexRow applies the flexbox solver through ImGui child regions. These
// run in a headless frame and read each child's child-region geometry (via
// GetWindowWidth()/GetWindowPos() inside the render callback) to confirm the row
// sizes and positions children per SolveFlex.

#include <unigui/widgets/layout.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <vector>

using unigui::Layout::FlexChild;
using unigui::layout::FlexJustify;
using unigui::Layout::FlexRow;

class FlexRowTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
        ImGui::Begin("t");
    }
    void TearDown() override {
        ImGui::End();
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(FlexRowTest, EmptyDoesNotCrash) {
    EXPECT_NO_THROW(FlexRow("empty", {}));
}

TEST_F(FlexRowTest, RendersEachChildExactlyOnce) {
    int calls = 0;
    FlexRow("row",
            {{{.basis = 100.0f}, [&] { ++calls; }},
             {{.basis = 100.0f}, [&] { ++calls; }},
             {{.basis = 100.0f}, [&] { ++calls; }}},
            {.width = 400.0f, .height = 20.0f, .gap = 8.0f});
    EXPECT_EQ(calls, 3);
}

TEST_F(FlexRowTest, GrowSplitsContainerWidth) {
    std::vector<float> w(2, -1.0f);
    FlexRow("grow",
            {{{.grow = 1.0f}, [&] { w[0] = ImGui::GetWindowWidth(); }},
             {{.grow = 3.0f}, [&] { w[1] = ImGui::GetWindowWidth(); }}},
            {.width = 400.0f, .height = 20.0f});
    // grow 1:3 over 400px of free space → 100 / 300
    EXPECT_NEAR(w[0], 100.0f, 2.0f);
    EXPECT_NEAR(w[1], 300.0f, 2.0f);
}

TEST_F(FlexRowTest, GapIsSubtractedBeforeGrow) {
    std::vector<float> w(2, -1.0f);
    FlexRow("gap",
            {{{.grow = 1.0f}, [&] { w[0] = ImGui::GetWindowWidth(); }},
             {{.grow = 1.0f}, [&] { w[1] = ImGui::GetWindowWidth(); }}},
            {.width = 420.0f, .height = 20.0f, .gap = 20.0f});
    // (420 - 20 gap) split 1:1 → 200 each
    EXPECT_NEAR(w[0], 200.0f, 2.0f);
    EXPECT_NEAR(w[1], 200.0f, 2.0f);
}

TEST_F(FlexRowTest, ShrinkOverflowThroughContainer) {
    std::vector<float> w(2, -1.0f);
    FlexRow("shrink",
            {{{.basis = 300.0f}, [&] { w[0] = ImGui::GetWindowWidth(); }},
             {{.basis = 300.0f}, [&] { w[1] = ImGui::GetWindowWidth(); }}},
            {.width = 400.0f, .height = 20.0f});
    // 200px overflow split evenly by shrink*basis → 200 / 200
    EXPECT_NEAR(w[0], 200.0f, 2.0f);
    EXPECT_NEAR(w[1], 200.0f, 2.0f);
}

TEST_F(FlexRowTest, ZeroWidthChildIsOmittedNotBallooned) {
    // A basis=0/grow=0 child solves to size 0. It must be skipped (not rendered
    // full-width), leaving the next child correctly sized — this guards the
    // ImGui "BeginChild(width==0) == fill remaining" trap.
    bool aRendered = false;
    float bWidth = -1.0f;
    FlexRow("zero",
            {{{.basis = 0.0f}, [&] { aRendered = true; }},
             {{.basis = 100.0f}, [&] { bWidth = ImGui::GetWindowWidth(); }}},
            {.width = 400.0f, .height = 20.0f});
    EXPECT_FALSE(aRendered);           // collapsed child omitted
    EXPECT_NEAR(bWidth, 100.0f, 2.0f); // neighbour keeps its own width
}

TEST_F(FlexRowTest, JustifySpaceBetweenShiftsChildren) {
    std::vector<float> x(2, 0.0f);
    FlexRow("just",
            {{{.basis = 100.0f}, [&] { x[0] = ImGui::GetWindowPos().x; }},
             {{.basis = 100.0f}, [&] { x[1] = ImGui::GetWindowPos().x; }}},
            {.width = 400.0f, .height = 20.0f, .justify = FlexJustify::SpaceBetween});
    // 200px leftover pushed entirely between the two 100px children → 100 + 200.
    EXPECT_NEAR(x[1] - x[0], 300.0f, 2.0f);
}

TEST_F(FlexRowTest, TwoRowsInOneWindowDoNotCollide) {
    int a = 0, b = 0;
    FlexRow("first", {{{.grow = 1.0f}, [&] { ++a; }}, {{.grow = 1.0f}, [&] { ++a; }}},
            {.width = 200.0f, .height = 20.0f});
    FlexRow("second", {{{.grow = 1.0f}, [&] { ++b; }}, {{.grow = 1.0f}, [&] { ++b; }}},
            {.width = 200.0f, .height = 20.0f});
    EXPECT_EQ(a, 2);
    EXPECT_EQ(b, 2);
}
