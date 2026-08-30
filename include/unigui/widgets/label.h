#pragma once

#include <unigui/core/observable.h>
#include <unigui/widgets/widget_base.h>

#include <string>

namespace unigui {

class Label : public FluentWidget<Label> {
public:
    Label(std::string name, std::string text = "");
    void Render() override;
    void SetText(std::string text);
    const std::string& GetText() const;

    // ── Fluent (chainable) helpers — return Label& via CRTP base ──────────
    Label& WithText(std::string text) {
        SetText(std::move(text));
        return *this;
    }

    // Address-sensitivity note: BindText() registers a `this`-capturing observer
    // in the source observable, so a *bound* Label must not be relocated. Unbound
    // Labels stay movable (copy is ill-formed via the move-only Subscription
    // member), so they can still live in containers.

    /// One-way bind the label's text to an Observable: the label adopts the
    /// current value and updates on every change. The label owns the
    /// subscription, so it auto-detaches on destruction.
    void BindText(Observable<std::string>& src) {
        textBinding_ = src.SubscribeAndFire([this](const std::string& v) { SetText(v); });
    }

private:
    std::string text_;
    Subscription textBinding_;
};

} // namespace unigui
