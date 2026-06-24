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

TEST(MultiSplitterTest, SerializeRestore_RoundTrips) {
    unigui::MultiSplitter a("ms");
    a.AddPanel(2.f, [] {});
    a.AddPanel(3.f, [] {});
    a.AddPanel(5.f, [] {});
    const std::string layout = a.SerializeLayout(); // "0.2000,0.3000,0.5000"

    unigui::MultiSplitter b("ms2");
    b.AddPanel(1.f, [] {});
    b.AddPanel(1.f, [] {});
    b.AddPanel(1.f, [] {});
    EXPECT_TRUE(b.RestoreLayout(layout));
    const auto r = b.GetRatios();
    ASSERT_EQ(r.size(), 3u);
    EXPECT_NEAR(r[0], 0.2f, 1e-3f);
    EXPECT_NEAR(r[1], 0.3f, 1e-3f);
    EXPECT_NEAR(r[2], 0.5f, 1e-3f);
}

TEST(MultiSplitterTest, RestoreLayout_RejectsWrongCountAndGarbage) {
    unigui::MultiSplitter s("ms");
    s.AddPanel(1.f, [] {});
    s.AddPanel(1.f, [] {});
    EXPECT_FALSE(s.RestoreLayout("0.5"));          // too few
    EXPECT_FALSE(s.RestoreLayout("0.3,0.3,0.4"));  // too many
    EXPECT_FALSE(s.RestoreLayout("garbage"));      // non-throwing, ignored
    EXPECT_TRUE(s.RestoreLayout("0.25,0.75"));     // matching count → applied
    EXPECT_NEAR(s.GetRatios()[0], 0.25f, 1e-3f);
}
