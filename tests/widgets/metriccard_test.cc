#include <unigui/widgets/metriccard.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui;

class MetricCardTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1000, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(MetricCardTest, Render_DoesNotCrash) {
    MetricCard c("m");
    c.WithTitle("账户A").WithValue("1,234,567").WithDelta(1.2, "+1.20%").WithSubtext("可用 50万");
    EXPECT_NO_THROW(c.Render());
}

TEST_F(MetricCardTest, Fluent_KeepsTypeAndStoresFields) {
    MetricCard c("m2");
    MetricCard& ref = c.WithTitle("T").WithValue("V").WithAccentRail().WithStatusDot(
        theme::Semantic::Success);
    EXPECT_EQ(&ref, &c);
    EXPECT_EQ(c.GetTitle(), "T");
    EXPECT_EQ(c.GetValue(), "V");
}

TEST_F(MetricCardTest, NegativeDelta_RendersWithoutCrash) {
    MetricCard c("m3");
    c.WithTitle("PnL").WithValue("-8,500").WithDelta(-2.4, "-2.40%");
    EXPECT_NO_THROW(c.Render());
}

TEST_F(MetricCardTest, CustomBodyAndHeaderActions_DoNotCrash) {
    MetricCard c("m4");
    bool bodyRan = false, actRan = false;
    c.WithTitle("Custom")
        .WithStatusDot(theme::Semantic::Warning)
        .WithHeaderActions([&] { actRan = true; ImGui::SmallButton("X"); })
        .WithBody([&] { bodyRan = true; ImGui::TextUnformatted("body"); })
        .WithSize(220.f, 90.f);
    EXPECT_NO_THROW(c.Render());
    EXPECT_TRUE(bodyRan);
    EXPECT_TRUE(actRan);
}
