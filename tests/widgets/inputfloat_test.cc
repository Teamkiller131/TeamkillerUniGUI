#include <unigui/unigui.h>
#include <unigui/widgets/inputfloat.h>

#include <imgui.h>

#include <cmath>
#include <gtest/gtest.h>

class InputFloatTest : public ::testing::Test {
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

// 1. Default value is zero
TEST_F(InputFloatTest, GetValue_DefaultsToZero) {
    unigui::InputFloat iif("iif", "Float");
    EXPECT_FLOAT_EQ(iif.GetValue(), 0.0f);
}

// 2. Render doesn't crash
TEST_F(InputFloatTest, Render_DoesNotCrash) {
    unigui::InputFloat iif("iif", "Float", 3.14f);
    iif.Render();
}

// 3. SetValue works
TEST_F(InputFloatTest, SetValue_Works) {
    unigui::InputFloat iif("iif", "Float");
    iif.SetValue(99.5f);
    EXPECT_FLOAT_EQ(iif.GetValue(), 99.5f);
}

// 4. SetRange clamps value during render
TEST_F(InputFloatTest, SetRange_ClampsValue) {
    unigui::InputFloat iif("iif", "Range", 50.0f, 0.0f, 100.0f);
    iif.SetRange(10.0f, 20.0f);
    iif.Render();
    EXPECT_GE(iif.GetValue(), 10.0f);
    EXPECT_LE(iif.GetValue(), 20.0f);
}

// 5. Min clamping
TEST_F(InputFloatTest, Min_ClampsLow) {
    unigui::InputFloat iif("iif", "Min", -50.0f, -10.0f, 100.0f);
    iif.Render();
    EXPECT_GE(iif.GetValue(), -10.0f);
}

// 6. Max clamping
TEST_F(InputFloatTest, Max_ClampsHigh) {
    unigui::InputFloat iif("iif", "Max", 200.0f, 0.0f, 100.0f);
    iif.Render();
    EXPECT_LE(iif.GetValue(), 100.0f);
}

// 7. Negative values work within range
TEST_F(InputFloatTest, NegativeValue_Works) {
    unigui::InputFloat iif("iif", "Neg", -42.5f, -100.0f, 0.0f);
    iif.Render();
    EXPECT_FLOAT_EQ(iif.GetValue(), -42.5f);
}

// 8. Zero value works
TEST_F(InputFloatTest, ZeroValue_Works) {
    unigui::InputFloat iif("iif", "Zero", 0.0f, -50.0f, 50.0f);
    iif.Render();
    EXPECT_FLOAT_EQ(iif.GetValue(), 0.0f);
}

// 9. OnChange fires when value is clamped during render
TEST_F(InputFloatTest, OnChange_Fires_OnClamp) {
    unigui::InputFloat iif("iif", "Clamp", 50.0f, 0.0f, 100.0f);
    int call_count = 0;
    iif.SetOnChange([&](float) { call_count++; });
    iif.SetValue(200.0f); // Outside max, will be clamped during Render
    iif.Render();
    EXPECT_EQ(call_count, 1);
    EXPECT_LE(iif.GetValue(), 100.0f);
}

// 10. OnChange does not fire when value unchanged during render
TEST_F(InputFloatTest, OnChange_DoesNotFire_OnSameValue) {
    unigui::InputFloat iif("iif", "Same", 50.0f, 0.0f, 100.0f);
    int call_count = 0;
    iif.SetOnChange([&](float) { call_count++; });
    iif.SetValue(50.0f);
    iif.Render();
    EXPECT_EQ(call_count, 0);
}

// 11. SetFormat changes format string
TEST_F(InputFloatTest, SetFormat_DoesNotCrash) {
    unigui::InputFloat iif("iif", "Precision", 3.14159f);
    iif.SetFormat("%.5f");
    iif.Render();
}

// 12. SetSuffix renders without crash
TEST_F(InputFloatTest, SetSuffix_DoesNotCrash) {
    unigui::InputFloat iif("iif", "Speed");
    iif.SetSuffix("m/s");
    iif.Render();
}

// 13. Default min/max boundaries
TEST_F(InputFloatTest, DefaultMinMax_Boundaries) {
    unigui::InputFloat iif("iif", "Bounds"); // defaults: min=0, max=100
    iif.SetValue(-999.0f);
    iif.Render();
    EXPECT_GE(iif.GetValue(), 0.0f);
    iif.SetValue(9999.0f);
    iif.Render();
    EXPECT_LE(iif.GetValue(), 100.0f);
}

// 14. Hide/Show visibility
TEST_F(InputFloatTest, Hide_Show_Visibility) {
    unigui::InputFloat iif("iif", "Viz");
    EXPECT_TRUE(iif.IsVisible());
    iif.Hide();
    EXPECT_FALSE(iif.IsVisible());
    iif.Show();
    EXPECT_TRUE(iif.IsVisible());
}

// 15. Clamping triggers OnChange with correct value
TEST_F(InputFloatTest, OnChange_ReceivesClampedValue) {
    unigui::InputFloat iif("iif", "Val", 0.0f, 0.0f, 100.0f);
    float received = -1.0f;
    iif.SetOnChange([&](float v) { received = v; });
    iif.SetValue(250.0f); // Outside range, will clamp to 100
    iif.Render();
    EXPECT_FLOAT_EQ(received, 100.0f);
}
