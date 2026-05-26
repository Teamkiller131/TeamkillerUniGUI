#include <unigui/fx/easing.h>
#include <gtest/gtest.h>
#include <cmath>

using namespace unigui::fx;

TEST(EasingTest, Linear_StraightLine) {
    EXPECT_NEAR(linear(0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(linear(0.5f), 0.5f, 0.001f);
    EXPECT_NEAR(linear(1.0f), 1.0f, 0.001f);
}

TEST(EasingTest, QuadIn_Accelerating) {
    EXPECT_NEAR(quadIn(0.0f), 0.0f, 0.001f);
    EXPECT_LT(quadIn(0.5f), 0.5f);    // slow start
    EXPECT_NEAR(quadIn(1.0f), 1.0f, 0.001f);
}

TEST(EasingTest, QuadOut_Decelerating) {
    EXPECT_NEAR(quadOut(0.0f), 0.0f, 0.001f);
    EXPECT_GT(quadOut(0.5f), 0.5f);   // fast start
    EXPECT_NEAR(quadOut(1.0f), 1.0f, 0.001f);
}

TEST(EasingTest, QuadInOut_Symmetric) {
    EXPECT_NEAR(quadInOut(0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(quadInOut(0.5f), 0.5f, 0.01f);  // midpoint
    EXPECT_NEAR(quadInOut(1.0f), 1.0f, 0.001f);
}

TEST(EasingTest, Cubic_Range) {
    EXPECT_NEAR(cubicIn(1.0f), 1.0f, 0.001f);
    EXPECT_NEAR(cubicOut(0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(cubicOut(1.0f), 1.0f, 0.001f);
}

TEST(EasingTest, Expo_Endpoints) {
    EXPECT_NEAR(expoIn(0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(expoIn(1.0f), 1.0f, 0.001f);
    EXPECT_NEAR(expoOut(0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(expoOut(1.0f), 1.0f, 0.001f);
}

TEST(EasingTest, ElasticOut_Overshoots) {
    EXPECT_NEAR(elasticOut(0.0f), 0.0f, 0.001f);
    EXPECT_NEAR(elasticOut(1.0f), 1.0f, 0.001f);
    // Elastic should overshoot above 1.0 somewhere in the middle
    bool overshot = false;
    for (float t = 0.f; t <= 1.f; t += 0.005f) {
        if (elasticOut(t) > 1.001f) { overshot = true; break; }
    }
    EXPECT_TRUE(overshot) << "elasticOut should overshoot above 1.0";
}

TEST(EasingTest, BounceOut_StaysInRange) {
    for (float t = 0.f; t <= 1.f; t += 0.01f) {
        float v = bounceOut(t);
        EXPECT_GE(v, 0.f);
        EXPECT_LE(v, 1.f);
    }
}

TEST(EasingTest, Ease_AllCurves_RangeZeroToOne) {
    std::vector<EasingCurve> curves = {
        EasingCurve::Linear,  EasingCurve::QuadIn,  EasingCurve::QuadOut,
        EasingCurve::QuadInOut, EasingCurve::CubicIn, EasingCurve::CubicOut,
        EasingCurve::ExpoIn,  EasingCurve::ExpoOut,
        EasingCurve::ElasticOut, EasingCurve::BounceOut,
    };
    for (auto c : curves) {
        bool isOvershoot = (c == EasingCurve::ElasticOut);
        float hi = isOvershoot ? 1.15f : 1.001f;
        EXPECT_GE(Ease(0.f, c), -0.001f);
        EXPECT_LE(Ease(0.f, c), 1.001f);
        EXPECT_GE(Ease(1.f, c), -0.001f);
        EXPECT_LE(Ease(1.f, c), 1.001f);
        EXPECT_GE(Ease(0.5f, c), -0.001f) << "curve " << (int)c << " out of range low";
        EXPECT_LE(Ease(0.5f, c), hi) << "curve " << (int)c << " out of range high";
    }
}

TEST(EasingTest, ParseEasing_ValidNames) {
    EXPECT_EQ(ParseEasing("linear"), EasingCurve::Linear);
    EXPECT_EQ(ParseEasing("bounceOut"), EasingCurve::BounceOut);
    EXPECT_EQ(ParseEasing("elasticOut"), EasingCurve::ElasticOut);
    EXPECT_EQ(ParseEasing("ease-in-out"), EasingCurve::EaseInOut);
    EXPECT_EQ(ParseEasing("ease"), EasingCurve::Ease);
}

TEST(EasingTest, ParseEasing_Invalid_Fallback) {
    EXPECT_EQ(ParseEasing("unknown"), EasingCurve::Linear);
    EXPECT_EQ(ParseEasing(""), EasingCurve::Linear);
}
