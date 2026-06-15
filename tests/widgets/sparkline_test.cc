#include <unigui/widgets/sparkline.h>

#include <imgui.h>

#include <gtest/gtest.h>

class SparklineTest : public ::testing::Test {
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

using unigui::Sparkline;

TEST_F(SparklineTest, Render_EmptyDoesNotCrash) {
    Sparkline s("sp");
    s.Render(); // no data → just reserves layout
    EXPECT_EQ(s.PointCount(), 0u);
}

TEST_F(SparklineTest, SetData_StoresValues) {
    Sparkline s("sp");
    s.SetData({1.f, 2.f, 3.f, 2.f});
    EXPECT_EQ(s.PointCount(), 4u);
    EXPECT_FLOAT_EQ(s.GetData()[2], 3.f);
}

TEST_F(SparklineTest, PushValue_AppendsAndCaps) {
    Sparkline s("sp");
    s.SetMaxPoints(3);
    for (int i = 0; i < 6; ++i)
        s.PushValue(static_cast<float>(i));
    // Capped to the 3 most recent: 3, 4, 5.
    ASSERT_EQ(s.PointCount(), 3u);
    EXPECT_FLOAT_EQ(s.GetData().front(), 3.f);
    EXPECT_FLOAT_EQ(s.GetData().back(), 5.f);
}

TEST_F(SparklineTest, PushValue_UnboundedByDefault) {
    Sparkline s("sp");
    for (int i = 0; i < 100; ++i)
        s.PushValue(static_cast<float>(i));
    EXPECT_EQ(s.PointCount(), 100u);
}

TEST_F(SparklineTest, Clear_EmptiesData) {
    Sparkline s("sp");
    s.SetData({1.f, 2.f});
    s.Clear();
    EXPECT_EQ(s.PointCount(), 0u);
}

TEST_F(SparklineTest, Mode_DefaultsToLineAndChanges) {
    Sparkline s("sp");
    EXPECT_EQ(s.GetMode(), Sparkline::Mode::Line);
    s.SetMode(Sparkline::Mode::Bar);
    EXPECT_EQ(s.GetMode(), Sparkline::Mode::Bar);
}

TEST_F(SparklineTest, Range_AutoByDefaultThenFixed) {
    Sparkline s("sp");
    EXPECT_TRUE(s.IsAutoRange());
    s.SetRange(0.f, 10.f);
    EXPECT_FALSE(s.IsAutoRange());
    s.SetAutoRange();
    EXPECT_TRUE(s.IsAutoRange());
}

TEST_F(SparklineTest, AllModes_RenderWithoutCrash) {
    Sparkline s("sp");
    s.SetData({1.f, 5.f, 2.f, 8.f, 3.f});
    for (auto m : {Sparkline::Mode::Line, Sparkline::Mode::Area, Sparkline::Mode::Bar}) {
        s.SetMode(m);
        EXPECT_NO_THROW(s.Render());
    }
}

TEST_F(SparklineTest, FlatSeries_RenderWithoutCrash) {
    Sparkline s("sp");
    s.SetData({5.f, 5.f, 5.f}); // zero span must not divide-by-zero
    EXPECT_NO_THROW(s.Render());
}

TEST_F(SparklineTest, Fluent_ChainsAndKeepsType) {
    Sparkline s("sp");
    Sparkline& ref = s.WithMode(Sparkline::Mode::Area)
                         .WithSize(120.f, 30.f)
                         .WithColorByTrend()
                         .WithShowLastDot()
                         .WithData({1.f, 2.f, 3.f});
    EXPECT_EQ(&ref, &s);
    EXPECT_EQ(s.GetMode(), Sparkline::Mode::Area);
    EXPECT_FLOAT_EQ(s.GetSize().x, 120.f);
    EXPECT_EQ(s.PointCount(), 3u);
}

TEST_F(SparklineTest, TrendColoring_RendersUpAndDown) {
    Sparkline up("up"), down("down");
    up.SetColorByTrend(true);
    up.SetData({1.f, 2.f, 3.f}); // rising
    down.SetColorByTrend(true);
    down.SetData({3.f, 2.f, 1.f}); // falling
    EXPECT_NO_THROW(up.Render());
    EXPECT_NO_THROW(down.Render());
}
