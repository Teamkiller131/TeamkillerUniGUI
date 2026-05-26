#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {

struct WizardStep {
    std::string name;
    std::string title;
    std::function<void()> render;
};

/// Multi-step wizard with Next/Previous navigation.
class Wizard : public Widget {
public:
    Wizard(std::string name, std::string title = "Wizard");

    void Render() override;

    void AddStep(std::string name, std::string title, std::function<void()> renderFn);
    void Clear() { steps_.clear(); curStep_ = 0; }

    int GetCurrentStep() const { return curStep_; }
    int GetStepCount() const { return (int)steps_.size(); }

    void Next();
    void Previous();
    void GoTo(int step);

    void SetOnFinish(std::function<void()> fn) { onFinish_ = std::move(fn); }
    void SetOnCancel(std::function<void()> fn) { onCancel_ = std::move(fn); }

private:
    std::string title_;
    std::vector<WizardStep> steps_;
    int curStep_ = 0;
    std::function<void()> onFinish_;
    std::function<void()> onCancel_;
};

} // namespace unigui
