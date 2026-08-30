#pragma once
#include <unigui/widgets/widget_base.h>

#include <string>
namespace unigui {
class LoadingIndicator : public FluentWidget<LoadingIndicator> {
public:
    LoadingIndicator(std::string name, float radius = 16.0f);
    void Render() override;
    void SetActive(bool active);
    bool IsActive() const;

    // ── Fluent (chainable) helpers — return LoadingIndicator& via CRTP base ──────────
    LoadingIndicator& WithActive(bool active) {
        SetActive(active);
        return *this;
    }

private:
    float radius_;
    bool active_ = true;
    float angle_ = 0;
};
} // namespace unigui
