#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui::presets {

/// WizardFlow — a prefab multi-step flow: a step indicator across the top
/// ("Step i of N — <title>" plus one dot per step, filled with the theme
/// accent for done/current steps), the current step's content in a child
/// region, and a button row pinned bottom-right ([Cancel?] [Back] [Next |
/// Finish on the last step]). Unlike the bare Wizard widget it adds per-step
/// validation gating (Next/Finish stay disabled while the step's canAdvance
/// gate returns false), localizable button labels, and a11y announcements on
/// every transition.
///
/// Looks decent with nothing configured beyond the constructor (an empty flow
/// shows a friendly hint instead of crashing):
///
///     unigui::presets::WizardFlow flow("setup");
///     flow.AddStep("Welcome", [] { ImGui::TextUnformatted("hi"); })
///         .AddStep(
///             "License", [&] { ImGui::Checkbox("I agree", &agreed); },
///             [&] { return agreed; }) // Next disabled until checked
///         .WithOnFinish([] { Install(); });
///     // per frame: flow.Render();
class WizardFlow : public FluentWidget<WizardFlow> {
public:
    explicit WizardFlow(std::string name);

    void Render() override;

    // ── Fluent configuration ────────────────────────────────────────────
    /// Append an ungated step (Next/Finish always enabled while it shows).
    WizardFlow& AddStep(std::string title, std::function<void()> content);
    /// Append a gated step: Next (or Finish on the last step) is disabled —
    /// and Next() is a no-op — while `canAdvance` returns false.
    WizardFlow& AddStep(std::string title, std::function<void()> content,
                        std::function<bool()> canAdvance);
    /// Fired when the flow finishes: Finish pressed (or Next() called) on the
    /// last step while its gate passes. The finish announces "Completed".
    WizardFlow& WithOnFinish(std::function<void()> fn);
    /// Show a Cancel button (leftmost in the row) firing this callback; no
    /// Cancel button is drawn until a callback is set.
    WizardFlow& WithOnCancel(std::function<void()> fn);
    /// Localizable button labels (defaults "Next" / "Back" / "Finish").
    WizardFlow& WithNextLabel(std::string label);
    WizardFlow& WithBackLabel(std::string label);
    WizardFlow& WithFinishLabel(std::string label);

    // ── Live state ──────────────────────────────────────────────────────
    /// Current step index, or -1 while the flow has no steps.
    int GetCurrentStep() const;
    int GetStepCount() const;
    /// Does the current step's gate pass? True when the step has no gate;
    /// false on an empty flow (there is nothing to advance).
    bool CanAdvance() const;
    /// Advance to the next step. Respects the gate (a no-op while it fails);
    /// on the last step this finishes the flow instead (fires WithOnFinish).
    /// Announces the newly-shown step to assistive technology.
    void Next();
    /// Return to the previous step (ungated: going back is always allowed).
    void Back();
    /// Jump to a step programmatically. Clamps to [0, steps-1] and skips the
    /// gates (a deliberate jump); announces the step. A no-op on an empty
    /// flow or when `step` resolves to the current step.
    void GoTo(int step);

private:
    struct Step {
        std::string title;
        std::function<void()> content;
        std::function<bool()> canAdvance; // empty = ungated
    };

    std::string StepLabel() const;
    void Finish();
    void RenderStepIndicator();
    void RenderButtonRow();

    std::vector<Step> steps_;
    int current_ = -1;
    std::function<void()> onFinish_;
    std::function<void()> onCancel_;
    std::string nextLabel_ = "Next";
    std::string backLabel_ = "Back";
    std::string finishLabel_ = "Finish";
};

} // namespace unigui::presets
