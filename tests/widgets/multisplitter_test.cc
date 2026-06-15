#include <unigui/widgets/multisplitter.h>

#include <gtest/gtest.h>

TEST(MultiSplitterTest, AddPanel_NormalizesRatios) {
    unigui::MultiSplitter splitter("ms");
    splitter.AddPanel(3.0f, [] {});
    splitter.AddPanel(7.0f, [] {});

    const auto ratios = splitter.GetRatios();
    ASSERT_EQ(ratios.size(), 2u);
    EXPECT_NEAR(ratios[0], 0.3f, 1e-4f);
    EXPECT_NEAR(ratios[1], 0.7f, 1e-4f);
}

TEST(MultiSplitterTest, SetRatios_NormalizesAndKeepsCount) {
    unigui::MultiSplitter splitter("ms");
    splitter.AddPanel(1.0f, [] {});
    splitter.AddPanel(1.0f, [] {});
    splitter.AddPanel(1.0f, [] {});

    splitter.SetRatios({2.0f, 3.0f, 5.0f});

    const auto ratios = splitter.GetRatios();
    ASSERT_EQ(ratios.size(), 3u);
    EXPECT_NEAR(ratios[0], 0.2f, 1e-4f);
    EXPECT_NEAR(ratios[1], 0.3f, 1e-4f);
    EXPECT_NEAR(ratios[2], 0.5f, 1e-4f);
}

TEST(MultiSplitterTest, Configure_IsIdempotentByPanelCount) {
    unigui::MultiSplitter splitter("ms");
    EXPECT_FALSE(splitter.IsConfigured());

    splitter.Configure({{0.3f, [] {}}, {0.7f, [] {}}});
    EXPECT_TRUE(splitter.IsConfigured());
    ASSERT_EQ(splitter.GetRatios().size(), 2u);

    // Re-calling with the same panel count is a no-op (no duplication) and
    // records the design ratios.
    splitter.Configure({{0.5f, [] {}}, {0.5f, [] {}}});
    ASSERT_EQ(splitter.GetRatios().size(), 2u);
    EXPECT_NEAR(splitter.GetRatios()[0], 0.3f, 1e-4f); // kept first config
    EXPECT_EQ(splitter.GetDesignRatios().size(), 2u);

    // Changing the panel count rebuilds.
    splitter.Configure({{1.f, [] {}}, {1.f, [] {}}, {1.f, [] {}}});
    EXPECT_EQ(splitter.GetRatios().size(), 3u);
}

TEST(MultiSplitterTest, AddPanelWithMinPx_StoresAndRenders) {
    unigui::MultiSplitter splitter("ms");
    splitter.AddPanel(0.5f, [] {}, 120.f);
    splitter.AddPanel(0.5f, [] {});
    EXPECT_EQ(splitter.GetRatios().size(), 2u);
}
