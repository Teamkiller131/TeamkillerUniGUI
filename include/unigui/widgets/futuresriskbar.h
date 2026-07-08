#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <string>

namespace unigui {

class FuturesRiskBar : public FluentWidget<FuturesRiskBar> {
public:
    explicit FuturesRiskBar(std::string name);

    void Render() override;

    void SetAccountName(std::string name);
    void SetMarginText(std::string text);
    void SetActualRatio(double r);
    void SetEstimatedRatio(double r);
    void SetOvernightRatio(double r);
    void SetAnimated(bool on);

    // ── Fluent (chainable) helpers — return FuturesRiskBar& via CRTP base ──────────
    FuturesRiskBar& WithAccountName(std::string name) {
        SetAccountName(std::move(name));
        return *this;
    }
    FuturesRiskBar& WithMarginText(std::string text) {
        SetMarginText(std::move(text));
        return *this;
    }
    FuturesRiskBar& WithActualRatio(double r) {
        SetActualRatio(r);
        return *this;
    }
    FuturesRiskBar& WithEstimatedRatio(double r) {
        SetEstimatedRatio(r);
        return *this;
    }
    FuturesRiskBar& WithOvernightRatio(double r) {
        SetOvernightRatio(r);
        return *this;
    }
    FuturesRiskBar& WithAnimated(bool on) {
        SetAnimated(on);
        return *this;
    }

private:
    std::string accountName_;
    std::string marginText_;
    double actualRatio_ = 0.0;
    double estimatedRatio_ = 0.0;
    double overnightRatio_ = 0.0;
    bool animated_ = false;
    float animActual_ = 0.0f;
    float animEst_ = 0.0f;
    float animOvernight_ = 0.0f;
};

} // namespace unigui
