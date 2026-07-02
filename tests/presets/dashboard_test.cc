#include <unigui/core/accessibility.h>
#include <unigui/presets/dashboard.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>

using unigui::presets::Dashboard;

class DashboardPresetTest : public ::testing::Test {
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

TEST_F(DashboardPresetTest, Defaults_AreDecent) {
    Dashboard d("dash");
    EXPECT_EQ(d.GetCardCount(), 0);
    EXPECT_FLOAT_EQ(d.GetGap(), 8.f);
    EXPECT_FLOAT_EQ(d.GetMinCardWidth(), 260.f);
    EXPECT_EQ(d.GetColumns(), 0); // no frame rendered yet
}

TEST_F(DashboardPresetTest, AddCardAndAddMetric_CountAndChain) {
    Dashboard d("dash");
    Dashboard& r = d.AddCard("A", [] {})
                       .AddMetric("CPU", [] { return std::string("42%"); })
                       .AddMetric("PnL", [] { return std::string("+120"); }, [] { return 1.5; });
    EXPECT_EQ(&r, &d); // fluent chain preserves the derived type
    EXPECT_EQ(d.GetCardCount(), 3);
}

TEST_F(DashboardPresetTest, FluentSetters_ApplyAndClamp) {
    Dashboard d("dash");
    d.WithGap(12.f).WithMinCardWidth(300.f);
    EXPECT_FLOAT_EQ(d.GetGap(), 12.f);
    EXPECT_FLOAT_EQ(d.GetMinCardWidth(), 300.f);
    d.WithGap(-5.f).WithMinCardWidth(0.f);
    EXPECT_FLOAT_EQ(d.GetGap(), 0.f);          // clamped to >= 0
    EXPECT_FLOAT_EQ(d.GetMinCardWidth(), 1.f); // clamped to >= 1
}

TEST_F(DashboardPresetTest, MetricGetters_PolledOncePerRender) {
    int valueCalls = 0, deltaCalls = 0;
    Dashboard d("dash");
    d.AddMetric(
        "M",
        [&] {
            ++valueCalls;
            return std::string("7");
        },
        [&] {
            ++deltaCalls;
            return -0.5;
        });
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("host");
    d.Render();
    ImGui::End();
    EXPECT_EQ(valueCalls, 1);
    EXPECT_EQ(deltaCalls, 1);
}

TEST_F(DashboardPresetTest, CardBody_InvokedDuringRender) {
    int bodyCalls = 0;
    Dashboard d("dash");
    d.AddCard("Log", [&] {
        ++bodyCalls;
        ImGui::TextUnformatted("line");
    });
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("host");
    d.Render();
    ImGui::End();
    EXPECT_EQ(bodyCalls, 1);
}

TEST_F(DashboardPresetTest, HugeMinCardWidth_ForcesSingleColumn) {
    Dashboard d("dash");
    d.WithMinCardWidth(10000.f).AddCard("A", [] {}).AddCard("B", [] {});
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("host");
    d.Render();
    ImGui::End();
    EXPECT_EQ(d.GetColumns(), 1);
}

TEST_F(DashboardPresetTest, Columns_CappedAtCardCount) {
    Dashboard d("dash");
    d.WithMinCardWidth(1.f).AddCard("A", [] {}).AddCard("B", [] {});
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("host");
    d.Render();
    ImGui::End();
    EXPECT_EQ(d.GetColumns(), 2); // room for many 1px columns, capped at 2 cards
}

TEST_F(DashboardPresetTest, DuplicateTitles_RenderSafely) {
    Dashboard d("dash");
    d.AddCard("Same", [] { ImGui::TextUnformatted("1"); })
        .AddCard("Same", [] { ImGui::TextUnformatted("2"); })
        .AddMetric("Same", [] { return std::string("3"); });
    d.Render(); // index-scoped PushID: no ImGui ID collision, no crash
    SUCCEED();
}

// ── Accessibility: every card registers as a Group in the a11y tree ────────
class DashboardA11yTest : public DashboardPresetTest {
protected:
    void SetUp() override {
        DashboardPresetTest::SetUp();
        unigui::a11y::SetEnabled(true);
        unigui::a11y::BeginFrame();
        unigui::a11y::DrainAnnouncements();
    }
    void TearDown() override {
        unigui::a11y::SetEnabled(false);
        DashboardPresetTest::TearDown();
    }
};

TEST_F(DashboardA11yTest, Cards_RegisterAsGroups) {
    Dashboard d("dash_a11y");
    d.AddCard("Alpha", [] {}).AddMetric("Beta", [] { return std::string("1"); });
    ImGui::SetNextWindowSize(ImVec2(600, 400));
    ImGui::Begin("host");
    d.Render();
    ImGui::End();

    int groups = 0;
    bool sawAlpha = false, sawBeta = false;
    for (const auto& n : unigui::a11y::Tree()) {
        if (n.role == unigui::a11y::Role::Group) {
            ++groups;
            if (n.value == "Alpha")
                sawAlpha = true;
            if (n.value == "Beta")
                sawBeta = true;
        }
    }
    EXPECT_EQ(groups, 2);
    EXPECT_TRUE(sawAlpha);
    EXPECT_TRUE(sawBeta);
}

TEST_F(DashboardA11yTest, Render_WithNothingConfigured_DoesNotCrash) {
    Dashboard d("dash_empty");
    d.Render();
    EXPECT_EQ(d.GetColumns(), 0);
}
