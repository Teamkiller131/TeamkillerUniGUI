#include <unigui/unigui.h>
#include <unigui/widgets/multihandleslider.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui;

TEST(MultiHandleSliderTest, AddTick_RemoveTick) {
    MultiHandleSlider s("mhs");
    s.AddTick({1, 10.f});
    s.AddTick({2, 20.f});
    s.AddTick({3, 30.f});
    EXPECT_EQ(s.GetTicks().size(), 3u);
    s.RemoveTick(2);
    ASSERT_EQ(s.GetTicks().size(), 2u);
    EXPECT_EQ(s.GetTicks()[0].id, 1);
    EXPECT_EQ(s.GetTicks()[1].id, 3); // only the matching tick is removed
    s.RemoveTick(99);                 // absent id -> no-op
    EXPECT_EQ(s.GetTicks().size(), 2u);
}

TEST(MultiHandleSliderTest, SetTicks_Replaces) {
    MultiHandleSlider s("mhs");
    s.AddTick({1, 5.f});
    s.SetTicks({{7, 1.f}, {8, 2.f}}); // replaces, not appends
    ASSERT_EQ(s.GetTicks().size(), 2u);
    EXPECT_EQ(s.GetTicks()[0].id, 7);
}

TEST(MultiHandleSliderTest, SetRange_ClampsToNonDegenerate) {
    MultiHandleSlider s("mhs");
    s.SetRange(0.f, 100.f);
    EXPECT_FLOAT_EQ(s.GetRangeMin(), 0.f);
    EXPECT_FLOAT_EQ(s.GetRangeMax(), 100.f);
    s.SetRange(5.f, 5.f); // equal -> max bumped to min+1 (renderer divides by the span)
    EXPECT_FLOAT_EQ(s.GetRangeMin(), 5.f);
    EXPECT_FLOAT_EQ(s.GetRangeMax(), 6.f);
    s.SetRange(10.f, 2.f); // inverted -> max clamped to min+1
    EXPECT_FLOAT_EQ(s.GetRangeMin(), 10.f);
    EXPECT_FLOAT_EQ(s.GetRangeMax(), 11.f);
}

// Render path needs an ImGui frame. A degenerate range and multiple instances must
// not crash or divide by zero.
class MultiHandleSliderRenderTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(MultiHandleSliderRenderTest, Render_NoCrash) {
    ImGui::Begin("w");
    MultiHandleSlider s("mhs");
    s.SetRange(5.f, 5.f); // degenerate input, clamped — must not divide by zero
    s.AddTick({1, 25.f});
    s.AddTick({2, 75.f});
    EXPECT_NO_THROW(s.Render());
    ImGui::End();
}

TEST_F(MultiHandleSliderRenderTest, MultiInstance_IdIsolation) {
    ImGui::Begin("w");
    MultiHandleSlider a("mhs_a"), b("mhs_b");
    a.AddTick({1, 10.f});
    b.AddTick({1, 90.f});
    EXPECT_NO_THROW({
        a.Render();
        b.Render();
    });
    ImGui::End();
}
