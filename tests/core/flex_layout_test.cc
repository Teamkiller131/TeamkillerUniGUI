#include <unigui/core/flex_layout.h>

#include <cstddef>
#include <gtest/gtest.h>
#include <vector>

using unigui::layout::FlexAlign;
using unigui::layout::FlexItem;
using unigui::layout::FlexJustify;
using unigui::layout::FlexParams;
using unigui::layout::SolveFlex;
using unigui::layout::SolveFlexWrap;

namespace {
constexpr float kEps = 1e-3f;
}

TEST(FlexLayoutTest, EmptyReturnsEmpty) {
    auto spans = SolveFlex({}, {.containerSize = 100.0f});
    EXPECT_TRUE(spans.empty());
}

TEST(FlexLayoutTest, BasisOnlyPacksAtStartWithGap) {
    auto spans =
        SolveFlex({{.basis = 100.0f}, {.basis = 100.0f}}, {.containerSize = 400.0f, .gap = 10.0f});
    ASSERT_EQ(spans.size(), 2u);
    EXPECT_NEAR(spans[0].offset, 0.0f, kEps);
    EXPECT_NEAR(spans[0].size, 100.0f, kEps);
    EXPECT_NEAR(spans[1].offset, 110.0f, kEps); // 100 + 10 gap
    EXPECT_NEAR(spans[1].size, 100.0f, kEps);
}

TEST(FlexLayoutTest, GrowDistributesFreeSpaceByWeight) {
    // 200px free split 1:2 over two items starting at basis 100.
    auto spans = SolveFlex({{.basis = 100.0f, .grow = 1.0f}, {.basis = 100.0f, .grow = 2.0f}},
                           {.containerSize = 400.0f});
    ASSERT_EQ(spans.size(), 2u);
    EXPECT_NEAR(spans[0].size, 100.0f + 200.0f / 3.0f, kEps); // 166.667
    EXPECT_NEAR(spans[1].size, 100.0f + 400.0f / 3.0f, kEps); // 233.333
    EXPECT_NEAR(spans[1].offset, spans[0].size, kEps);
    EXPECT_NEAR(spans[0].size + spans[1].size, 400.0f, kEps); // fully fills
}

TEST(FlexLayoutTest, GrowRespectsMaxAndRedistributes) {
    // item0 would grow to 200 but is capped at 120; its surplus flows to item1.
    auto spans = SolveFlex(
        {{.basis = 100.0f, .grow = 1.0f, .maxSize = 120.0f}, {.basis = 100.0f, .grow = 1.0f}},
        {.containerSize = 400.0f});
    ASSERT_EQ(spans.size(), 2u);
    EXPECT_NEAR(spans[0].size, 120.0f, kEps);
    EXPECT_NEAR(spans[1].size, 280.0f, kEps);
}

TEST(FlexLayoutTest, ShrinkDistributesOverflowByScaledFactor) {
    // 40px overflow split by shrink*basis = 100 : 300  →  10 : 30.
    auto spans = SolveFlex({{.basis = 100.0f, .shrink = 1.0f}, {.basis = 100.0f, .shrink = 3.0f}},
                           {.containerSize = 160.0f});
    ASSERT_EQ(spans.size(), 2u);
    EXPECT_NEAR(spans[0].size, 90.0f, kEps);
    EXPECT_NEAR(spans[1].size, 70.0f, kEps);
}

TEST(FlexLayoutTest, ShrinkRespectsMinAndRedistributes) {
    // item0 floored at min 80; remaining overflow shrinks item1 further.
    auto spans = SolveFlex(
        {{.basis = 100.0f, .shrink = 1.0f, .minSize = 80.0f}, {.basis = 100.0f, .shrink = 1.0f}},
        {.containerSize = 120.0f});
    ASSERT_EQ(spans.size(), 2u);
    EXPECT_NEAR(spans[0].size, 80.0f, kEps);
    EXPECT_NEAR(spans[1].size, 40.0f, kEps);
}

TEST(FlexLayoutTest, SingleItemGrowFillsContainer) {
    auto spans = SolveFlex({{.basis = 50.0f, .grow = 1.0f}}, {.containerSize = 200.0f});
    ASSERT_EQ(spans.size(), 1u);
    EXPECT_NEAR(spans[0].offset, 0.0f, kEps);
    EXPECT_NEAR(spans[0].size, 200.0f, kEps);
}

TEST(FlexLayoutTest, GapCountedWithGrow) {
    auto spans = SolveFlex({{.basis = 100.0f, .grow = 1.0f}, {.basis = 100.0f, .grow = 1.0f}},
                           {.containerSize = 410.0f, .gap = 10.0f});
    ASSERT_EQ(spans.size(), 2u);
    EXPECT_NEAR(spans[0].size, 200.0f, kEps); // (410 - 200 - 10) / 2 added to each
    EXPECT_NEAR(spans[1].size, 200.0f, kEps);
    EXPECT_NEAR(spans[1].offset, 210.0f, kEps); // 200 + 10 gap
}

// ── justify-content (no grow → 100px leftover over two 100px items) ──────────
TEST(FlexLayoutTest, JustifyCenter) {
    auto spans = SolveFlex({{.basis = 100.0f}, {.basis = 100.0f}},
                           {.containerSize = 300.0f, .justify = FlexJustify::Center});
    EXPECT_NEAR(spans[0].offset, 50.0f, kEps);
    EXPECT_NEAR(spans[1].offset, 150.0f, kEps);
}

TEST(FlexLayoutTest, JustifyEnd) {
    auto spans = SolveFlex({{.basis = 100.0f}, {.basis = 100.0f}},
                           {.containerSize = 300.0f, .justify = FlexJustify::End});
    EXPECT_NEAR(spans[0].offset, 100.0f, kEps);
    EXPECT_NEAR(spans[1].offset, 200.0f, kEps);
}

TEST(FlexLayoutTest, JustifySpaceBetween) {
    auto spans = SolveFlex({{.basis = 100.0f}, {.basis = 100.0f}},
                           {.containerSize = 300.0f, .justify = FlexJustify::SpaceBetween});
    EXPECT_NEAR(spans[0].offset, 0.0f, kEps);
    EXPECT_NEAR(spans[1].offset, 200.0f, kEps); // 100px pushed entirely between
}

TEST(FlexLayoutTest, JustifySpaceAround) {
    auto spans = SolveFlex({{.basis = 100.0f}, {.basis = 100.0f}},
                           {.containerSize = 300.0f, .justify = FlexJustify::SpaceAround});
    EXPECT_NEAR(spans[0].offset, 25.0f, kEps);  // half of 100/2 before first
    EXPECT_NEAR(spans[1].offset, 175.0f, kEps); // 25 + 100 + 50
}

TEST(FlexLayoutTest, JustifySpaceEvenly) {
    auto spans = SolveFlex({{.basis = 100.0f}, {.basis = 100.0f}},
                           {.containerSize = 300.0f, .justify = FlexJustify::SpaceEvenly});
    EXPECT_NEAR(spans[0].offset, 100.0f / 3.0f, kEps);                 // 33.333
    EXPECT_NEAR(spans[1].offset, 100.0f / 3.0f * 2.0f + 100.0f, kEps); // 166.667
}

// ── cross-axis placement (align-items); item crossSize 40 in a 100-tall line ──
TEST(FlexLayoutTest, AlignStartKeepsItemCrossSizeAtZeroOffset) {
    auto spans =
        SolveFlex({{.basis = 50.0f, .crossSize = 40.0f}},
                  {.containerSize = 50.0f, .crossSize = 100.0f, .align = FlexAlign::Start});
    ASSERT_EQ(spans.size(), 1u);
    EXPECT_NEAR(spans[0].crossOffset, 0.0f, kEps);
    EXPECT_NEAR(spans[0].crossSize, 40.0f, kEps);
}

TEST(FlexLayoutTest, AlignCenterCentersOnCrossAxis) {
    auto spans =
        SolveFlex({{.basis = 50.0f, .crossSize = 40.0f}},
                  {.containerSize = 50.0f, .crossSize = 100.0f, .align = FlexAlign::Center});
    EXPECT_NEAR(spans[0].crossOffset, 30.0f, kEps); // (100 - 40) / 2
    EXPECT_NEAR(spans[0].crossSize, 40.0f, kEps);
}

TEST(FlexLayoutTest, AlignEndPushesToCrossEnd) {
    auto spans = SolveFlex({{.basis = 50.0f, .crossSize = 40.0f}},
                           {.containerSize = 50.0f, .crossSize = 100.0f, .align = FlexAlign::End});
    EXPECT_NEAR(spans[0].crossOffset, 60.0f, kEps); // 100 - 40
    EXPECT_NEAR(spans[0].crossSize, 40.0f, kEps);
}

TEST(FlexLayoutTest, AlignStretchFillsCrossAxis) {
    auto spans =
        SolveFlex({{.basis = 50.0f, .crossSize = 40.0f}},
                  {.containerSize = 50.0f, .crossSize = 100.0f, .align = FlexAlign::Stretch});
    EXPECT_NEAR(spans[0].crossOffset, 0.0f, kEps);
    EXPECT_NEAR(spans[0].crossSize, 100.0f, kEps); // stretched to the line height
}

// ── flex-wrap line-breaking (SolveFlexWrap) ──────────────────────────────────
TEST(FlexLayoutWrapTest, EmptyReturnsEmpty) {
    auto lines = SolveFlexWrap({}, {.containerSize = 100.0f});
    EXPECT_TRUE(lines.empty());
}

TEST(FlexLayoutWrapTest, AllItemsFitProduceSingleLineEqualToSolveFlex) {
    // Two 100px items in a 400px container fit on one line; the wrapped result
    // must equal the single-line SolveFlex result exactly.
    const std::vector<FlexItem> items = {{.basis = 100.0f, .grow = 1.0f},
                                         {.basis = 100.0f, .grow = 2.0f}};
    const FlexParams params{.containerSize = 400.0f, .gap = 10.0f};

    auto lines = SolveFlexWrap(items, params);
    auto flat = SolveFlex(items, params);

    ASSERT_EQ(lines.size(), 1u);
    ASSERT_EQ(lines[0].size(), flat.size());
    for (std::size_t i = 0; i < flat.size(); ++i) {
        EXPECT_NEAR(lines[0][i].offset, flat[i].offset, kEps);
        EXPECT_NEAR(lines[0][i].size, flat[i].size, kEps);
        EXPECT_NEAR(lines[0][i].crossOffset, flat[i].crossOffset, kEps);
        EXPECT_NEAR(lines[0][i].crossSize, flat[i].crossSize, kEps);
    }
}

TEST(FlexLayoutWrapTest, OverflowWrapsIntoIndependentlySolvedLines) {
    // Four 100px items in a 250px container: greedy packing fits two per line
    // (100 + 10 gap + 100 = 210 <= 250; adding a third = 320 > 250). Each line is
    // solved on its own and stacked by the explicit lineHeight on the cross axis.
    const std::vector<FlexItem> items = {{.basis = 100.0f, .crossSize = 30.0f},
                                         {.basis = 100.0f, .crossSize = 30.0f},
                                         {.basis = 100.0f, .crossSize = 30.0f},
                                         {.basis = 100.0f, .crossSize = 30.0f}};
    const FlexParams params{.containerSize = 250.0f, .gap = 10.0f};

    auto lines = SolveFlexWrap(items, params, /*lineHeight=*/50.0f);
    ASSERT_EQ(lines.size(), 2u);
    ASSERT_EQ(lines[0].size(), 2u);
    ASSERT_EQ(lines[1].size(), 2u);

    // Each line is solved independently: items packed at the start with the gap.
    EXPECT_NEAR(lines[0][0].offset, 0.0f, kEps);
    EXPECT_NEAR(lines[0][0].size, 100.0f, kEps);
    EXPECT_NEAR(lines[0][1].offset, 110.0f, kEps); // 100 + 10 gap
    EXPECT_NEAR(lines[0][1].size, 100.0f, kEps);
    EXPECT_NEAR(lines[1][0].offset, 0.0f, kEps);
    EXPECT_NEAR(lines[1][0].size, 100.0f, kEps);
    EXPECT_NEAR(lines[1][1].offset, 110.0f, kEps);
    EXPECT_NEAR(lines[1][1].size, 100.0f, kEps);

    // Line 0 sits at cross 0; line 1 is shifted down by exactly one lineHeight.
    EXPECT_NEAR(lines[0][0].crossOffset, 0.0f, kEps);
    EXPECT_NEAR(lines[0][1].crossOffset, 0.0f, kEps);
    EXPECT_NEAR(lines[1][0].crossOffset, 50.0f, kEps);
    EXPECT_NEAR(lines[1][1].crossOffset, 50.0f, kEps);
    // crossOffset increases by exactly lineHeight between consecutive lines.
    EXPECT_NEAR(lines[1][0].crossOffset - lines[0][0].crossOffset, 50.0f, kEps);
}

TEST(FlexLayoutWrapTest, AutoLineHeightUsesPerLineMaxCrossSize) {
    // No explicit lineHeight → each line's height is its tallest item crossSize.
    // Container 250, no gap: two 100px items pack per line (200 <= 250; a third
    // = 300 > 250 wraps). Line 0's tallest crossSize is 40, so line 1 is shifted
    // down by 40 (not by item-2's own 25).
    const std::vector<FlexItem> items = {{.basis = 100.0f, .crossSize = 20.0f},
                                         {.basis = 100.0f, .crossSize = 40.0f},
                                         {.basis = 100.0f, .crossSize = 25.0f}};
    const FlexParams params{.containerSize = 250.0f};

    auto lines = SolveFlexWrap(items, params);
    ASSERT_EQ(lines.size(), 2u);
    ASSERT_EQ(lines[0].size(), 2u);
    ASSERT_EQ(lines[1].size(), 1u);

    EXPECT_NEAR(lines[0][0].crossOffset, 0.0f, kEps);
    EXPECT_NEAR(lines[0][1].crossOffset, 0.0f, kEps);
    // Shifted by line 0's tallest item crossSize (40), the auto line height.
    EXPECT_NEAR(lines[1][0].crossOffset, 40.0f, kEps);
}

TEST(FlexLayoutWrapTest, OversizedItemOccupiesItsOwnLine) {
    // A single item whose basis exceeds the container gets a line to itself, and
    // neighbours wrap around it. Container 100; item1 basis 250 (oversized).
    // shrink = 0 so the per-line SolveFlex leaves each item at its basis, isolating
    // the line-breaking behaviour under test.
    const std::vector<FlexItem> items = {{.basis = 80.0f, .shrink = 0.0f, .crossSize = 10.0f},
                                         {.basis = 250.0f, .shrink = 0.0f, .crossSize = 10.0f},
                                         {.basis = 80.0f, .shrink = 0.0f, .crossSize = 10.0f}};
    const FlexParams params{.containerSize = 100.0f};

    auto lines = SolveFlexWrap(items, params, /*lineHeight=*/12.0f);
    ASSERT_EQ(lines.size(), 3u);
    ASSERT_EQ(lines[0].size(), 1u);
    ASSERT_EQ(lines[1].size(), 1u); // the oversized item alone on its own line
    ASSERT_EQ(lines[2].size(), 1u);

    EXPECT_NEAR(lines[0][0].size, 80.0f, kEps);
    EXPECT_NEAR(lines[1][0].size, 250.0f, kEps); // kept at basis (shrink = 0)
    EXPECT_NEAR(lines[2][0].size, 80.0f, kEps);

    EXPECT_NEAR(lines[0][0].crossOffset, 0.0f, kEps);
    EXPECT_NEAR(lines[1][0].crossOffset, 12.0f, kEps);
    EXPECT_NEAR(lines[2][0].crossOffset, 24.0f, kEps);
}
