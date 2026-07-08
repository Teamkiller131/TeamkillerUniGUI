#pragma once
#include <unigui/widgets/widget_base.h>

#include <array>
#include <string>
namespace unigui {
class Tag : public FluentWidget<Tag> {
public:
    Tag(std::string name, std::string text, std::array<float, 3> color = {0.2f, 0.5f, 1.0f});
    void Render() override;
    void SetText(std::string t);
    void SetColor(std::array<float, 3> c);
    void SetRemovable(bool r);
    bool RemoveClicked() const;

    // ── Fluent (chainable) helpers — return Tag& via CRTP base ──────────
    Tag& WithText(std::string t) {
        SetText(std::move(t));
        return *this;
    }
    Tag& WithColor(std::array<float, 3> c) {
        SetColor(c);
        return *this;
    }
    Tag& WithRemovable(bool r) {
        SetRemovable(r);
        return *this;
    }

private:
    std::string text_;
    std::array<float, 3> color_;
    bool removable_ = false;
    bool removeClicked_ = false;
};
} // namespace unigui
