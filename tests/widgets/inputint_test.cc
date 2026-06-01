#include <unigui/unigui.h>
#include <unigui/widgets/inputint.h>
#include <imgui.h>
#include <gtest/gtest.h>

class InputIntTest : public ::testing::Test {
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
TEST_F(InputIntTest, GetValue_DefaultsToZero) {
    unigui::InputInt ii("ii", "Int");
    EXPECT_EQ(ii.GetValue(), 0);
}

// 2. Render doesn't crash
TEST_F(InputIntTest, Render_DoesNotCrash) {
    unigui::InputInt ii("ii", "Int", 42);
    ii.Render();
}

// 3. SetValue works
TEST_F(InputIntTest, SetValue_Works) {
    unigui::InputInt ii("ii", "Int");
    ii.SetValue(99);
    EXPECT_EQ(ii.GetValue(), 99);
}

// 4. SetRange clamps value during render
TEST_F(InputIntTest, SetRange_ClampsValue) {
    unigui::InputInt ii("ii", "Range", 50, 0, 100);
    ii.SetRange(10, 20);
    ii.Render();
    EXPECT_GE(ii.GetValue(), 10);
    EXPECT_LE(ii.GetValue(), 20);
}

// 5. Min clamping
TEST_F(InputIntTest, Min_ClampsLow) {
    unigui::InputInt ii("ii", "Min", -50, -10, 100);
    ii.Render();
    EXPECT_GE(ii.GetValue(), -10);
}

// 6. Max clamping
TEST_F(InputIntTest, Max_ClampsHigh) {
    unigui::InputInt ii("ii", "Max", 200, 0, 100);
    ii.Render();
    EXPECT_LE(ii.GetValue(), 100);
}

// 7. Negative values work within range
TEST_F(InputIntTest, NegativeValue_Works) {
    unigui::InputInt ii("ii", "Neg", -42, -100, 0);
    ii.Render();
    EXPECT_EQ(ii.GetValue(), -42);
}

// 8. Zero value works
TEST_F(InputIntTest, ZeroValue_Works) {
    unigui::InputInt ii("ii", "Zero", 0, -50, 50);
    ii.Render();
    EXPECT_EQ(ii.GetValue(), 0);
}

// 9. OnChange fires when value is clamped during render
TEST_F(InputIntTest, OnChange_Fires_OnClamp) {
    unigui::InputInt ii("ii", "Clamp", 50, 0, 100);
    int call_count = 0;
    ii.SetOnChange([&](int) { call_count++; });
    ii.SetValue(200); // Outside max, will be clamped during Render
    ii.Render();
    EXPECT_EQ(call_count, 1);
    EXPECT_LE(ii.GetValue(), 100);
}

// 10. OnChange does not fire when value unchanged during render
TEST_F(InputIntTest, OnChange_DoesNotFire_OnSameValue) {
    unigui::InputInt ii("ii", "Same", 50, 0, 100);
    int call_count = 0;
    ii.SetOnChange([&](int) { call_count++; });
    ii.SetValue(50); // Same value
    ii.Render();
    EXPECT_EQ(call_count, 0);
}

// 11. SetSuffix renders without crash
TEST_F(InputIntTest, SetSuffix_DoesNotCrash) {
    unigui::InputInt ii("ii", "Width");
    ii.SetSuffix("px");
    ii.Render();
}

// 12. Default min/max boundaries
TEST_F(InputIntTest, DefaultMinMax_Boundaries) {
    unigui::InputInt ii("ii", "Bounds"); // defaults: min=0, max=100
    ii.SetValue(-999); // Below default min
    ii.Render();
    EXPECT_GE(ii.GetValue(), 0);
    ii.SetValue(9999); // Above default max
    ii.Render();
    EXPECT_LE(ii.GetValue(), 100);
}

// 13. Hide/Show visibility
TEST_F(InputIntTest, Hide_Show_Visibility) {
    unigui::InputInt ii("ii", "Viz");
    EXPECT_TRUE(ii.IsVisible());
    ii.Hide();
    EXPECT_FALSE(ii.IsVisible());
    ii.Show();
    EXPECT_TRUE(ii.IsVisible());
}

// 14. Multiple inputs independent
TEST_F(InputIntTest, MultipleInputs_Independent) {
    unigui::InputInt a("a", "A", 10, 0, 100);
    unigui::InputInt b("b", "B", 90, 0, 100);
    a.Render();
    b.Render();
    EXPECT_NE(a.GetID(), b.GetID());
}

// 15. Clamping triggers OnChange with correct value
TEST_F(InputIntTest, OnChange_ReceivesClampedValue) {
    unigui::InputInt ii("ii", "Val", 0, 0, 100);
    int received = -1;
    ii.SetOnChange([&](int v) { received = v; });
    ii.SetValue(250); // Outside range, will clamp to 100
    ii.Render();
    EXPECT_EQ(received, 100);
}
