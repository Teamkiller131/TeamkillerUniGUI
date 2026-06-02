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
