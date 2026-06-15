#include <unigui/widgets/gauge.h>

#include <imgui.h>

#include <gtest/gtest.h>

class GaugeTest : public ::testing::Test {
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

using unigui::Gauge;

TEST_F(GaugeTest, Render_DoesNotCrash) {
    Gauge g("g", 0.5f);
    EXPECT_NO_THROW(g.Render());
}

TEST_F(GaugeTest, Value_DefaultsAndSets) {
    Gauge g("g");
    EXPECT_FLOAT_EQ(g.GetValue(), 0.f);
    g.SetValue(0.7f);
    EXPECT_FLOAT_EQ(g.GetValue(), 0.7f);
}

TEST_F(GaugeTest, Fraction_DefaultUnitRange) {
    Gauge g("g", 0.25f);
    EXPECT_FLOAT_EQ(g.GetFraction(), 0.25f);
}

TEST_F(GaugeTest, Fraction_MapsCustomRange) {
    Gauge g("g");
    g.SetRange(0.f, 200.f);
    g.SetValue(50.f);
    EXPECT_FLOAT_EQ(g.GetFraction(), 0.25f);
}

TEST_F(GaugeTest, Fraction_ClampsOutOfRange) {
    Gauge g("g");
    g.SetRange(0.f, 10.f);
    g.SetValue(-5.f);
    EXPECT_FLOAT_EQ(g.GetFraction(), 0.f);
    g.SetValue(99.f);
    EXPECT_FLOAT_EQ(g.GetFraction(), 1.f);
}

TEST_F(GaugeTest, Fraction_ZeroSpanIsSafe) {
    Gauge g("g");
    g.SetRange(5.f, 5.f); // degenerate range must not divide-by-zero
    g.SetValue(5.f);
    EXPECT_FLOAT_EQ(g.GetFraction(), 0.f);
    EXPECT_NO_THROW(g.Render());
}

TEST_F(GaugeTest, Geometry_Setters) {
    Gauge g("g");
    g.SetRadius(60.f);
    g.SetThickness(12.f);
    g.SetSweepDegrees(360.f);
    EXPECT_FLOAT_EQ(g.GetRadius(), 60.f);
    EXPECT_FLOAT_EQ(g.GetSweepDegrees(), 360.f);
    EXPECT_NO_THROW(g.Render());
}

TEST_F(GaugeTest, CenterLabel_OverridesPercent) {
    Gauge g("g", 0.5f);
    g.SetCenterLabel("CPU");
    EXPECT_EQ(g.GetCenterLabel(), "CPU");
    EXPECT_NO_THROW(g.Render());
}

TEST_F(GaugeTest, Fluent_ChainsAndKeepsType) {
    Gauge g("g");
    Gauge& ref = g.WithValue(0.4f).WithRange(0.f, 1.f).WithRadius(50.f).WithSweepDegrees(270.f);
    EXPECT_EQ(&ref, &g);
    EXPECT_FLOAT_EQ(g.GetValue(), 0.4f);
    EXPECT_FLOAT_EQ(g.GetRadius(), 50.f);
}

TEST_F(GaugeTest, FullRingAndArc_RenderWithoutCrash) {
    Gauge ring("ring"), arc("arc");
    ring.SetSweepDegrees(360.f);
    ring.SetValue(0.9f);
    arc.SetSweepDegrees(270.f);
    arc.SetValue(0.3f);
    EXPECT_NO_THROW(ring.Render());
    EXPECT_NO_THROW(arc.Render());
}
