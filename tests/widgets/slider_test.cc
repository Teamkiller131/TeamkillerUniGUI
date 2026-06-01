#include <unigui/unigui.h>
#include <unigui/widgets/slider.h>
#include <imgui.h>
#include <gtest/gtest.h>

class SliderTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};

TEST_F(SliderTest, IntSlider_Defaults) {
    unigui::Slider<int> s("s", "Val", 50, 0, 100);
    EXPECT_EQ(s.GetValue(), 50);
}

TEST_F(SliderTest, IntSlider_SetValue) {
    unigui::Slider<int> s("s", "Val", 50, 0, 100);
    s.SetValue(75);
    EXPECT_EQ(s.GetValue(), 75);
}

TEST_F(SliderTest, IntSlider_Render_DoesNotCrash) {
    unigui::Slider<int> s("s", "Val", 50, 0, 100);
    s.Render();
}

TEST_F(SliderTest, FloatSlider_Defaults) {
    unigui::Slider<float> s("s", "Val", 0.5f, 0.0f, 1.0f);
    EXPECT_FLOAT_EQ(s.GetValue(), 0.5f);
}

TEST_F(SliderTest, FloatSlider_Render_DoesNotCrash) {
    unigui::Slider<float> s("s", "Val", 0.5f, 0.0f, 1.0f);
    s.Render();
}
