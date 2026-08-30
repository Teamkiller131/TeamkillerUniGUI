#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
namespace unigui {
class Splitter : public FluentWidget<Splitter> {
public:
    enum Orientation { Horizontal, Vertical };
    Splitter(std::string name, Orientation orientation = Horizontal, float split = 0.5f);
    void Render() override;
    float GetSplit() const;
    void SetContentA(std::function<void()> cb);
    void SetContentB(std::function<void()> cb);

    // ── Fluent (chainable) helpers — return Splitter& via CRTP base ────────
    Splitter& WithContentA(std::function<void()> cb) {
        SetContentA(std::move(cb));
        return *this;
    }
    Splitter& WithContentB(std::function<void()> cb) {
        SetContentB(std::move(cb));
        return *this;
    }

private:
    Orientation ori_;
    float split_;
    std::function<void()> cbA_, cbB_;
};
} // namespace unigui
