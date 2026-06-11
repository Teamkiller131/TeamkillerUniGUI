#include <unigui/unigui.h>
#include <unigui/widgets/colorpicker.h>

#include <imgui.h>

#include <gtest/gtest.h>
class ColorPickerTest : public ::testing::Test {
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
TEST_F(ColorPickerTest, Render_DoesNotCrash) {
    unigui::ColorPicker cp("cp", "Color");
    cp.Render();
}
TEST_F(ColorPickerTest, GetColor_Defaults) {
    unigui::ColorPicker cp("cp", "Color");
    auto c = cp.GetColor();
    EXPECT_FLOAT_EQ(c[0], 0.0f);
}
