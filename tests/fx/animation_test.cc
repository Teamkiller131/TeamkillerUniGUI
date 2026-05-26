#include <unigui/fx/animation.h>
#include <gtest/gtest.h>

using namespace unigui::fx;

TEST(AnimationTest, Play_SetsPlaying) {
    AnimationState s;
    s.Play(0.5f, EasingCurve::Linear);
    EXPECT_TRUE(s.IsPlaying());
    EXPECT_NEAR(s.progress, 0.f, 0.001f);
}

TEST(AnimationTest, Linear_ProgressToCompletion) {
    AnimationState s;
    s.Play(0.5f, EasingCurve::Linear);
    float v = s.Update(0.25f);
    EXPECT_NEAR(v, 0.5f, 0.01f);
    EXPECT_TRUE(s.IsPlaying());
    v = s.Update(0.25f);
    EXPECT_NEAR(v, 1.0f, 0.01f);
    EXPECT_FALSE(s.IsPlaying());
}

TEST(AnimationTest, Stop_StopsPlaying) {
    AnimationState s;
    s.Play(0.3f);
    s.Stop();
    EXPECT_FALSE(s.IsPlaying());
}

TEST(AnimationTest, Restart_ResetsElapsed) {
    AnimationState s;
    s.Play(0.3f);
    s.Update(0.2f);
    s.Play(0.3f);
    EXPECT_NEAR(s.Update(0.f), 0.f, 0.01f);
    EXPECT_TRUE(s.IsPlaying());
}

TEST(AnimationTest, Loop_ContinuesPlaying) {
    AnimationState s;
    s.loop = true;
    s.Play(0.3f, EasingCurve::Linear);
    s.Update(0.3f);
    EXPECT_TRUE(s.IsPlaying());
    EXPECT_NEAR(s.progress, 0.f, 0.01f);
}

TEST(AnimationTest, PingPong_ReversesAtEnd) {
    AnimationState s;
    s.pingPong = true;
    s.Play(0.4f, EasingCurve::Linear);
    float v1 = s.Update(0.4f);   // completes first pass
    EXPECT_NEAR(v1, 1.0f, 0.01f);
    float v2 = s.Update(0.2f);   // half-way back
    EXPECT_NEAR(v2, 0.5f, 0.05f);
    EXPECT_TRUE(s.IsPlaying());
    float v3 = s.Update(0.2f);   // back to 0
    EXPECT_NEAR(v3, 0.0f, 0.01f);
}

TEST(AnimationTest, OnComplete_FiresAtEnd) {
    AnimationState s;
    bool fired = false;
    s.onComplete = [&] { fired = true; };
    s.Play(0.1f, EasingCurve::Linear);
    s.Update(0.1f);
    EXPECT_TRUE(fired);
}

TEST(AnimationTest, UpdateFromStart_ClampsToOne) {
    AnimationState s;
    s.Play(0.5f);
    float v = s.UpdateFromStart(1.0f);   // overshoot duration
    EXPECT_NEAR(v, 1.0f, 0.01f);
    EXPECT_FALSE(s.IsPlaying());
}

TEST(AnimationTest, Manager_RegisterAndUpdate) {
    AnimationState s1, s2;
    auto& mgr = AnimationManager::Instance();
    int h1 = mgr.Register(&s1);
    int h2 = mgr.Register(&s2);
    s1.Play(0.5f, EasingCurve::Linear);
    s2.Play(0.5f, EasingCurve::Linear);
    mgr.UpdateAll(0.25f);
    EXPECT_NEAR(s1.progress, 0.5f, 0.01f);
    EXPECT_NEAR(s2.progress, 0.5f, 0.01f);
    mgr.Unregister(h1);
    mgr.Unregister(h2);
}
