#include <unigui/core/accessibility.h>
#include <unigui/presets/wizard_flow.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>

class WizardFlowTest : public ::testing::Test {
protected:
    void SetUp() override {
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

// ── Defaults: decent with nothing configured beyond the ctor ────────────────
TEST_F(WizardFlowTest, Defaults) {
    unigui::presets::WizardFlow flow("flow");
    EXPECT_EQ(flow.GetStepCount(), 0);
    EXPECT_EQ(flow.GetCurrentStep(), -1); // no steps yet
    EXPECT_FALSE(flow.CanAdvance());      // empty flow: nothing to advance
}

// ── Fluent chaining preserves the derived type and applies every setter ─────
TEST_F(WizardFlowTest, FluentChaining_AppliesConfiguration) {
    unigui::presets::WizardFlow flow("flow");
    unigui::presets::WizardFlow& chained =
        flow.AddStep("Welcome", [] {})
            .AddStep(
                "License", [] {}, [] { return false; })
            .WithOnFinish([] {})
            .WithOnCancel([] {})
            .WithNextLabel("Weiter")
            .WithBackLabel("Zurück")
            .WithFinishLabel("Fertig")
            .WithTooltip("flow tooltip"); // base helper stays WizardFlow&
    EXPECT_EQ(&chained, &flow);
    EXPECT_EQ(flow.GetStepCount(), 2);
    EXPECT_EQ(flow.GetCurrentStep(), 0); // first AddStep activates step 0
    EXPECT_TRUE(flow.CanAdvance());      // step 0 has no gate
}

// ── Next/Back walk the steps; Back stops at the first step ──────────────────
TEST_F(WizardFlowTest, NextAndBack_WalkTheSteps) {
    unigui::presets::WizardFlow flow("flow");
    flow.AddStep("A", [] {}).AddStep("B", [] {}).AddStep("C", [] {});
    flow.Next();
    EXPECT_EQ(flow.GetCurrentStep(), 1);
    flow.Next();
    EXPECT_EQ(flow.GetCurrentStep(), 2);
    flow.Back();
    EXPECT_EQ(flow.GetCurrentStep(), 1);
    flow.Back();
    flow.Back(); // already on the first step: no-op
    EXPECT_EQ(flow.GetCurrentStep(), 0);
}

// ── Next() on the last step finishes the flow and stays put ─────────────────
TEST_F(WizardFlowTest, Next_OnLastStep_Finishes) {
    unigui::presets::WizardFlow flow("flow");
    int finished = 0;
    flow.AddStep("A", [] {}).AddStep("B", [] {}).WithOnFinish([&] { ++finished; });
    flow.GoTo(1);
    flow.Next(); // last step: == Finish
    EXPECT_EQ(finished, 1);
    EXPECT_EQ(flow.GetCurrentStep(), 1); // finishing does not move the step
}

// ── Gating: Next and Finish are no-ops while the step's gate fails ──────────
TEST_F(WizardFlowTest, Gating_BlocksNextAndFinish) {
    unigui::presets::WizardFlow flow("flow");
    bool firstOk = false, lastOk = false;
    int finished = 0;
    flow.AddStep(
            "First", [] {}, [&] { return firstOk; })
        .AddStep(
            "Last", [] {}, [&] { return lastOk; })
        .WithOnFinish([&] { ++finished; });

    EXPECT_FALSE(flow.CanAdvance());
    flow.Next(); // gated: stays on step 0
    EXPECT_EQ(flow.GetCurrentStep(), 0);

    firstOk = true;
    EXPECT_TRUE(flow.CanAdvance());
    flow.Next();
    EXPECT_EQ(flow.GetCurrentStep(), 1);

    flow.Next(); // last step gated: Finish must not fire
    EXPECT_EQ(finished, 0);
    lastOk = true;
    flow.Next();
    EXPECT_EQ(finished, 1);
}

// ── GoTo clamps into range, skips gates, and no-ops on an empty flow ────────
TEST_F(WizardFlowTest, GoTo_ClampsAndSkipsGates) {
    unigui::presets::WizardFlow flow("flow");
    flow.GoTo(5); // empty flow: stays -1, no crash
    EXPECT_EQ(flow.GetCurrentStep(), -1);

    flow.AddStep(
            "A", [] {}, [] { return false; }) // gate never passes...
        .AddStep("B", [] {})
        .AddStep("C", [] {});
    flow.GoTo(99);                       // ...but GoTo is a deliberate jump: gates are skipped
    EXPECT_EQ(flow.GetCurrentStep(), 2); // clamped to last
    flow.GoTo(-7);
    EXPECT_EQ(flow.GetCurrentStep(), 0); // clamped to first
    flow.GoTo(1);
    EXPECT_EQ(flow.GetCurrentStep(), 1);
}

// ── A11y: a step change announces "Step i of N: title" ──────────────────────
TEST_F(WizardFlowTest, StepChange_AnnouncesStep) {
    unigui::a11y::SetEnabled(true);
    unigui::a11y::BeginFrame();
    unigui::a11y::DrainAnnouncements();

    unigui::presets::WizardFlow flow("flow");
    flow.AddStep("Intro", [] {}).AddStep("Details", [] {});
    flow.Next();

    bool announced = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements())
        if (a.message == "Step 2 of 2: Details")
            announced = true;
    EXPECT_TRUE(announced);
    unigui::a11y::SetEnabled(false);
}

// ── A11y: finishing the flow announces "Completed" ──────────────────────────
TEST_F(WizardFlowTest, Finish_AnnouncesCompleted) {
    unigui::a11y::SetEnabled(true);
    unigui::a11y::BeginFrame();
    unigui::a11y::DrainAnnouncements();

    unigui::presets::WizardFlow flow("flow");
    flow.AddStep("Only", [] {}).WithOnFinish([] {});
    flow.Next(); // single step: Next == Finish

    bool announced = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements())
        if (a.message == "Completed")
            announced = true;
    EXPECT_TRUE(announced);
    unigui::a11y::SetEnabled(false);
}

// ── Render: empty flow shows the hint instead of crashing ───────────────────
TEST_F(WizardFlowTest, Render_EmptyFlow_DoesNotCrash) {
    unigui::presets::WizardFlow flow("flow");
    flow.Render();
    EXPECT_EQ(flow.GetCurrentStep(), -1);
}

// ── Render (last: full composition) draws only the current step ─────────────
TEST_F(WizardFlowTest, Render_InvokesCurrentStepOnly) {
    unigui::presets::WizardFlow flow("flow");
    bool first = false, second = false;
    flow.AddStep("First", [&] { first = true; })
        .AddStep(
            "Second",
            [&] {
                second = true;
                ImGui::TextUnformatted("second");
            },
            [] { return false; }) // gated: Finish renders disabled
        .WithOnCancel([] {})      // Cancel button shown
        .WithNextLabel("Continue")
        .WithBackLabel("Previous")
        .WithFinishLabel("Done");
    flow.GoTo(1);
    flow.Render();
    EXPECT_FALSE(first);
    EXPECT_TRUE(second);
}
