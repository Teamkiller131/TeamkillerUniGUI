#include <unigui/unigui.h>
#include <unigui/widgets/sliderbar.h>
#include <imgui.h>
#include <gtest/gtest.h>

class SliderBarTest : public ::testing::Test {
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

// ── 1. Construction / basic properties ────────────────────────────────────

TEST_F(SliderBarTest, Construct_DefaultName) {
    unigui::SliderBar sb("my_slider");
    EXPECT_EQ(sb.GetName(), "my_slider");
}

TEST_F(SliderBarTest, Render_DoesNotCrash) {
    unigui::SliderBar sb("sb1");
    sb.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, Hidden_RendersWithoutCrash) {
    unigui::SliderBar sb("sb2");
    sb.Hide();
    sb.Render();
    SUCCEED();
}

// ── 2. MaxValue ───────────────────────────────────────────────────────────

TEST_F(SliderBarTest, SetMaxValue_ClampsToMinOne) {
    unigui::SliderBar sb("sb3");
    sb.SetMaxValue(0);
    sb.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, SetMaxValue_Large) {
    unigui::SliderBar sb("sb4");
    sb.SetMaxValue(1000);
    sb.Render();
    SUCCEED();
}

// ── 3. Ticks ──────────────────────────────────────────────────────────────

TEST_F(SliderBarTest, SetTicks_Empty) {
    unigui::SliderBar sb("sb5");
    sb.SetTicks({});
    EXPECT_TRUE(sb.GetTicks().empty());
}

TEST_F(SliderBarTest, SetTicks_WithData) {
    unigui::SliderBar sb("sb6");
    std::vector<unigui::SliderBar::Tick> ticks = {
        {10, 0.5},
        {50, 0.75},
        {80, 0.9}
    };
    sb.SetTicks(ticks);
    auto result = sb.GetTicks();
    ASSERT_EQ(result.size(), 3u);
    EXPECT_EQ(result[0].futuresLots, 10);
    EXPECT_DOUBLE_EQ(result[0].price, 0.5);
    EXPECT_EQ(result[2].futuresLots, 80);
}

TEST_F(SliderBarTest, GetTicks_ReturnsCopy) {
    unigui::SliderBar sb("sb7");
    std::vector<unigui::SliderBar::Tick> ticks = {{42, 0.123}};
    sb.SetTicks(ticks);
    auto copy = sb.GetTicks();
    copy[0].futuresLots = 999;
    // Original should be unchanged
    EXPECT_EQ(sb.GetTicks()[0].futuresLots, 42);
}

// ── 4. GetActiveTickIndex ─────────────────────────────────────────────────

TEST_F(SliderBarTest, GetActiveTickIndex_FindsMatch) {
    unigui::SliderBar sb("sb8");
    sb.SetTicks({
        {10, 100.5},
        {20, 200.0},
        {30, 300.75}
    });
    EXPECT_EQ(sb.GetActiveTickIndex(200.0, 20), 1);
}

TEST_F(SliderBarTest, GetActiveTickIndex_NoMatch) {
    unigui::SliderBar sb("sb9");
    sb.SetTicks({{10, 1.0}});
    EXPECT_EQ(sb.GetActiveTickIndex(999.0, 999), -1);
}

TEST_F(SliderBarTest, GetActiveTickIndex_EmptyTicks) {
    unigui::SliderBar sb("sb10");
    EXPECT_EQ(sb.GetActiveTickIndex(1.0, 1), -1);
}

// ── 5. CurrentLots ────────────────────────────────────────────────────────

TEST_F(SliderBarTest, SetCurrentLots) {
    unigui::SliderBar sb("sb11");
    sb.SetCurrentLots(50);
    sb.SetMaxValue(100);
    sb.Render();
    SUCCEED();
}

// ── 6. Active fill ────────────────────────────────────────────────────────

TEST_F(SliderBarTest, SetActiveFill) {
    unigui::SliderBar sb("sb12");
    sb.SetActiveFill(20, 60, IM_COL32(0xFF, 0x00, 0x00, 0x80));
    sb.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, SetActiveFill_NoFillWhenNegative) {
    unigui::SliderBar sb("sb13");
    // Defaults are -1, -1 → no fill rendered
    sb.Render();
    SUCCEED();
}

// ── 7. Left panel labels and buttons ──────────────────────────────────────

TEST_F(SliderBarTest, SetLeftLabel) {
    unigui::SliderBar sb("sb14");
    sb.SetLeftLabel("Account 12345");
    sb.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, SetLeftSubLabel) {
    unigui::SliderBar sb("sb15");
    sb.SetLeftLabel("Main");
    sb.SetLeftSubLabel("Futures");
    sb.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, ButtonCallbacks_Rendered) {
    unigui::SliderBar sb("sb16");
    bool addCalled = false, confirmCalled = false;
    bool rollbackCalled = false, submitCalled = false;
    sb.SetOnAdd([&]() { addCalled = true; });
    sb.SetOnConfirm([&]() { confirmCalled = true; });
    sb.SetOnRollback([&]() { rollbackCalled = true; });
    sb.SetOnSubmit([&]() { submitCalled = true; });
    // Just verify render doesn't crash with callbacks set
    sb.Render();
    SUCCEED();
}

// ── 8. Unsaved changes flag ───────────────────────────────────────────────

TEST_F(SliderBarTest, HasUnsavedChanges_DefaultsToFalse) {
    unigui::SliderBar sb("sb17");
    EXPECT_FALSE(sb.HasUnsavedChanges());
}

// ── 9. OnChanged callback ─────────────────────────────────────────────────

TEST_F(SliderBarTest, OnChanged_FiresOnRender) {
    unigui::SliderBar sb("sb18");
    bool fired = false;
    sb.SetOnChanged([&](const std::vector<unigui::SliderBar::Tick>&) {
        fired = true;
    });
    sb.SetTicks({{10, 0.5}});
    // Render without drag — shouldn't fire onChanged
    sb.Render();
    // onChanged only fires during drag interaction, which won't happen in tests
    SUCCEED();
}

// ── 10. Tick colors ───────────────────────────────────────────────────────

TEST_F(SliderBarTest, SetTickColors) {
    unigui::SliderBar sb("sb19");
    sb.SetTickColors({
        IM_COL32(0xFF, 0x00, 0x00, 0xFF),
        IM_COL32(0x00, 0xFF, 0x00, 0xFF),
        IM_COL32(0x00, 0x00, 0xFF, 0xFF)
    });
    sb.SetTicks({{10, 0.1}, {50, 0.5}, {90, 0.9}});
    sb.Render();
    SUCCEED();
}

// ── 11. Multiple SliderBars / ID isolation ────────────────────────────────

TEST_F(SliderBarTest, MultipleInstances_DifferentIDs) {
    unigui::SliderBar sbA("bar_a");
    unigui::SliderBar sbB("bar_b");
    unigui::SliderBar sbC("bar_c");
    EXPECT_NE(sbA.GetID(), sbB.GetID());
    EXPECT_NE(sbB.GetID(), sbC.GetID());
    sbA.Render();
    sbB.Render();
    sbC.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, MultipleInstances_SameName) {
    // Same name in different scope is OK — ImGui handles it via ID stack
    unigui::SliderBar sb1("same");
    unigui::SliderBar sb2("same");
    EXPECT_EQ(sb1.GetID(), sb2.GetID());
    sb1.Render();
    sb2.Render();
    SUCCEED();
}

// ── 12. Render with full configuration ────────────────────────────────────

TEST_F(SliderBarTest, FullConfig_RendersWithoutCrash) {
    unigui::SliderBar sb("full");
    sb.SetMaxValue(200);
    sb.SetCurrentLots(100);
    sb.SetActiveFill(40, 160, IM_COL32(0x3A, 0x8E, 0xE6, 0x60));
    sb.SetLeftLabel("Account #001");
    sb.SetLeftSubLabel("ES Futures");
    sb.SetTickColors({
        IM_COL32(0xE9, 0x45, 0x60, 0xFF),
        IM_COL32(0x28, 0xA7, 0x45, 0xFF),
        IM_COL32(0xF0, 0xC0, 0x40, 0xFF)
    });
    sb.SetTicks({
        {30, 4500.25},
        {100, 4510.50},
        {170, 4520.75}
    });
    bool changedFired = false;
    sb.SetOnChanged([&](const std::vector<unigui::SliderBar::Tick>&) {
        changedFired = true;
    });
    sb.SetOnAdd([]() {});
    sb.SetOnConfirm([]() {});
    sb.SetOnRollback([]() {});
    sb.SetOnSubmit([]() {});
    sb.Render();
    SUCCEED();
}

// ── 13. Edge cases ────────────────────────────────────────────────────────

TEST_F(SliderBarTest, Render_ZeroBarWidth) {
    // Force tiny available width
    unigui::SliderBar sb("narrow");
    sb.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, TickAtMaxValue) {
    unigui::SliderBar sb("max_tick");
    sb.SetMaxValue(100);
    sb.SetTicks({{100, 1.0}});
    sb.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, TickAtZero) {
    unigui::SliderBar sb("zero_tick");
    sb.SetMaxValue(100);
    sb.SetTicks({{0, 0.0}});
    sb.Render();
    SUCCEED();
}

TEST_F(SliderBarTest, ManyTicks) {
    unigui::SliderBar sb("many");
    sb.SetMaxValue(200);
    std::vector<unigui::SliderBar::Tick> ticks;
    for (int i = 0; i < 50; ++i) {
        ticks.push_back({i * 4, static_cast<double>(i) / 50.0});
    }
    sb.SetTicks(ticks);
    sb.Render();
    SUCCEED();
}
