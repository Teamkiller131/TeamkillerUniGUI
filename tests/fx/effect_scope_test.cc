#include <unigui/fx/effect_scope.h>
#include <imgui.h>
#include <gtest/gtest.h>

using namespace unigui::fx;

class EffectScopeTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); }
    void TearDown() override { ImGui::DestroyContext(); }
    ImDrawList* DL() { return ImGui::GetForegroundDrawList(); }
};

TEST_F(EffectScopeTest, ShadowEffect_Default) {
    ShadowEffect shadow;
    shadow.SetRect({0, 0}, {100, 50});
    shadow.Push(DL());
    shadow.Pop();
    SUCCEED();
}

TEST_F(EffectScopeTest, ShadowEffect_ZeroRadius) {
    ShadowEffect shadow(0.f);
    shadow.SetRect({0, 0}, {100, 50});
    shadow.Push(DL());
    shadow.Pop();
    SUCCEED();
}

TEST_F(EffectScopeTest, GlowEffect_Default) {
    GlowEffect glow;
    glow.SetRect({50, 50}, {150, 100});
    glow.Push(DL());
    glow.Pop();
    SUCCEED();
}

TEST_F(EffectScopeTest, BlurEffect_Default) {
    BlurEffect blur(8.f, 0.15f);
    blur.SetRect({10, 10}, {300, 200});
    blur.Push(DL());
    blur.Pop();
    SUCCEED();
}

TEST_F(EffectScopeTest, Gradient_Horizontal) {
    auto* dl = DL();
    GradientBrush::Horizontal(dl, {0, 0}, {200, 40},
                               IM_COL32(255,0,0,255), IM_COL32(0,0,255,255));
    SUCCEED();
}

TEST_F(EffectScopeTest, Gradient_Vertical) {
    auto* dl = DL();
    GradientBrush::Vertical(dl, {0, 0}, {200, 40},
                             IM_COL32(0,0,0,255), IM_COL32(255,255,255,255));
    SUCCEED();
}

TEST_F(EffectScopeTest, Gradient_MultiStop) {
    auto* dl = DL();
    std::vector<GradientStop> stops = {
        {0.0f, IM_COL32(255,0,0,255)},
        {0.5f, IM_COL32(0,255,0,255)},
        {1.0f, IM_COL32(0,0,255,255)},
    };
    GradientBrush::MultiStop(dl, {0, 0}, {300, 40}, stops, true);
    SUCCEED();
}

TEST_F(EffectScopeTest, Factory_Shadow) {
    auto s = Effects::Shadow(6.f, {3,3}, IM_COL32(0,0,0,100));
    s.SetRect({10,10}, {200,100});
    s.Push(DL()); s.Pop();
    SUCCEED();
}

TEST_F(EffectScopeTest, Factory_Glow) {
    auto g = Effects::Glow(10.f, IM_COL32(100,149,237,150));
    g.SetRect({10,10}, {150,80});
    g.Push(DL()); g.Pop();
    SUCCEED();
}

TEST_F(EffectScopeTest, Factory_GlassPanel) {
    auto g = Effects::GlassPanel(12.f, 0.15f);
    g.SetRect({0,0}, {400,300});
    g.Push(DL()); g.Pop();
    SUCCEED();
}
