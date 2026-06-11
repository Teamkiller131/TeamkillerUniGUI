#pragma once
#include <unigui/fx/animation.h>

#include <imgui.h>

#include <functional>
#include <vector>

namespace unigui {

/// Shimmer — animated loading placeholder with gradient sweep
class Shimmer {
public:
    Shimmer();

    /// Add a rectangular block (like a card or text line)
    void AddBlock(float width, float height, float x = 0.f, float y = 0.f);

    /// Add a circle (like an avatar placeholder)
    void AddCircle(float radius, float x = 0.f, float y = 0.f);

    /// Start / stop the sweep animation
    void Start();
    void Stop();
    bool IsPlaying() const;

    /// Set sweep speed (lower = slower, default 1.0)
    void SetSpeed(float s);

    /// Render the shimmer in current ImGui context
    void Render();

private:
    struct Element {
        enum { Block, Circle } kind = Block;
        float x, y, w, h; // w=radius for circle
    };

    std::vector<Element> elements_;
    fx::AnimationState anim_;
    float speed_ = 1.5f;
    bool playing_ = false;
};

} // namespace unigui
