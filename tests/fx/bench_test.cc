#include <unigui/fx/easing.h>
#include <unigui/fx/animation.h>
#include <imgui.h>
#include <gtest/gtest.h>
#include <chrono>

using namespace unigui::fx;

TEST(EffectBench, Easing_All10Curves_Under1ms) {
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) {
        float x = (float)i / 10000.f;
        Ease(x, EasingCurve::Linear);
        Ease(x, EasingCurve::QuadIn);
        Ease(x, EasingCurve::QuadOut);
        Ease(x, EasingCurve::QuadInOut);
        Ease(x, EasingCurve::CubicIn);
        Ease(x, EasingCurve::CubicOut);
        Ease(x, EasingCurve::ExpoIn);
        Ease(x, EasingCurve::ExpoOut);
        Ease(x, EasingCurve::ElasticOut);
        Ease(x, EasingCurve::BounceOut);
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    EXPECT_LT(us, 1000) << "100k easing evaluations took " << us << " us — too slow!";
}

TEST(EffectBench, AnimationState_100Updates_Fast) {
    AnimationState s;
    s.Play(1.f, EasingCurve::ExpoOut);
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; i++)
        s.Update(0.016f);  // simulate 60fps
    auto t1 = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();
    EXPECT_LT(us, 2000) << "10k animation updates took " << us << " us";
}
