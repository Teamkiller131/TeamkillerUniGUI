#pragma once
#include <imgui.h>
#include <string>
#include <functional>

namespace unigui {

/// Card — elevated surface widget with shadow, rounded corners, optional image
class Card {
public:
    enum Variant { Elevated, Outlined, Filled };

    Card(const std::string& title = "");

    // ── Configuration ────────────────────────────────────────────────────
    void SetTitle(const std::string& t);
    void SetContent(std::function<void()> fn);
    void SetFooter(std::function<void()> fn);
    void SetVariant(Variant v);
    void SetShadow(bool enable);
    void SetShadowRadius(float r);
    void SetPadding(float p);

    // ── Rendering ────────────────────────────────────────────────────────
    void Render();

private:
    std::string title_;
    std::function<void()> contentFn_, footerFn_;
    Variant variant_ = Elevated;
    bool hasShadow_ = true;
    float shadowRadius_ = 6.f;
    float padding_ = 16.f;
};

} // namespace unigui
