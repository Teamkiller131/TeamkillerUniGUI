#include <unigui/unigui.h>
#include <unigui/widgets/button.h>
#include <unigui/core/context.h>
#include <imgui.h>
#include <gtest/gtest.h>

class ButtonTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
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

// 1. Basic rendering doesn't crash
TEST_F(ButtonTest, Render_DoesNotCrash) {
    unigui::Button btn("btn", "Click Me");
    btn.Render();
}

// 2. Label getter works
TEST_F(ButtonTest, GetLabel_ReturnsGivenLabel) {
    unigui::Button btn("btn", "Submit");
    EXPECT_EQ(btn.GetLabel(), "Submit");
}

// 3. Label setter works
TEST_F(ButtonTest, SetLabel_UpdatesLabel) {
    unigui::Button btn("btn", "Submit");
    btn.SetLabel("Save");
    EXPECT_EQ(btn.GetLabel(), "Save");
}

// 4. Default enabled state is true
TEST_F(ButtonTest, IsEnabled_DefaultsToTrue) {
    unigui::Button btn("btn", "Click");
    EXPECT_TRUE(btn.IsEnabled());
}

// 5. Enable/disable toggling
TEST_F(ButtonTest, SetEnabled_DisablesAndEnables) {
    unigui::Button btn("btn", "Click");
    btn.SetEnabled(false);
    EXPECT_FALSE(btn.IsEnabled());
    btn.SetEnabled(true);
    EXPECT_TRUE(btn.IsEnabled());
}

TEST_F(ButtonTest, BasePointerSetEnabled_UpdatesButtonState) {
    unigui::Button btn("btn", "Click");
    unigui::Widget* widget = &btn;
    widget->SetEnabled(false);
    EXPECT_FALSE(btn.IsEnabled());
}

// Fluent (chainable) configuration returns the same widget and applies state.
TEST_F(ButtonTest, FluentApi_ChainsAndAppliesState) {
    unigui::Button btn("btn", "Click");
    unigui::Widget& ref = btn.WithTooltip("save").WithEnabled(false).WithShadow();
    EXPECT_EQ(&ref, static_cast<unigui::Widget*>(&btn));
    EXPECT_FALSE(btn.IsEnabled());
    EXPECT_TRUE(btn.GetShadowConfig().enabled);
    btn.WithVisible(false);
    EXPECT_FALSE(btn.IsVisible());
    btn.WithVisible(true).WithEnabled(true);
    EXPECT_TRUE(btn.IsVisible());
    EXPECT_TRUE(btn.IsEnabled());
}

// 6. Disabled button renders without crash
TEST_F(ButtonTest, Disabled_RendersWithoutCrash) {
    unigui::Button btn("btn", "Click");
    btn.SetEnabled(false);
    btn.Render();
    EXPECT_FALSE(btn.IsEnabled());
}

// 7. Hidden button renders without crash
TEST_F(ButtonTest, Hidden_RendersWithoutCrash) {
    unigui::Button btn("btn", "Hidden");
    btn.Hide();
    EXPECT_FALSE(btn.IsVisible());
    btn.Render();
}

// 8. Show/Hide toggles visibility
TEST_F(ButtonTest, Show_Hide_TogglesVisibility) {
    unigui::Button btn("btn", "Vis");
    EXPECT_TRUE(btn.IsVisible());
    btn.Hide();
    EXPECT_FALSE(btn.IsVisible());
    btn.Show();
    EXPECT_TRUE(btn.IsVisible());
}

// 9. WasClicked defaults to false
TEST_F(ButtonTest, WasClicked_DefaultsToFalse) {
    unigui::Button btn("btn", "Click");
    EXPECT_FALSE(btn.WasClicked());
}

// 10. ColorVariant variants render without crash
TEST_F(ButtonTest, ColorVariant_AllVariantsRender) {
    unigui::Button btn("btn", "Color");
    btn.SetColorVariant(unigui::Button::Primary);
    btn.Render();
    btn.SetColorVariant(unigui::Button::Danger);
    btn.Render();
    btn.SetColorVariant(unigui::Button::Success);
    btn.Render();
    btn.SetColorVariant(unigui::Button::Default);
    btn.Render();
}

// 11. Size variants render without crash
TEST_F(ButtonTest, SetSize_VariantsRender) {
    unigui::Button btn("btn", "Size");
    btn.SetSize(unigui::Button::Small);
    btn.Render();
    btn.SetSize(unigui::Button::Large);
    btn.Render();
}

// 12. Multiple buttons with different names don't conflict
TEST_F(ButtonTest, MultipleButtons_DifferentNames_NoConflict) {
    unigui::Button btn1("btn_a", "A");
    unigui::Button btn2("btn_b", "B");
    unigui::Button btn3("btn_c", "C");
    btn1.Render();
    btn2.Render();
    btn3.Render();
    EXPECT_NE(btn1.GetID(), btn2.GetID());
    EXPECT_NE(btn2.GetID(), btn3.GetID());
    EXPECT_NE(btn1.GetID(), btn3.GetID());
}

// 13. Same label different names don't conflict
TEST_F(ButtonTest, SameLabel_DifferentNames_NoConflict) {
    unigui::Button btn1("save_top", "Save");
    unigui::Button btn2("save_bottom", "Save");
    btn1.Render();
    btn2.Render();
    EXPECT_NE(btn1.GetID(), btn2.GetID());
}

// 14. SetTooltip renders without crash
TEST_F(ButtonTest, SetTooltip_DoesNotCrash) {
    unigui::Button btn("btn", "Hover");
    btn.SetTooltip("Click here to proceed");
    btn.Render();
}

// 15. Rapid enable/disable doesn't crash
TEST_F(ButtonTest, RapidEnableDisable_Stable) {
    unigui::Button btn("btn", "Rapid");
    for (int i = 0; i < 10; i++) {
        btn.SetEnabled(i % 2 == 0);
        btn.Render();
        EXPECT_EQ(btn.IsEnabled(), i % 2 == 0);
    }
}
