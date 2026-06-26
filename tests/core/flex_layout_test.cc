#include <unigui/core/flex_layout.h>

#include <gtest/gtest.h>

using unigui::layout::FlexItem;
using unigui::layout::FlexJustify;
using unigui::layout::FlexParams;
using unigui::layout::SolveFlex;

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
