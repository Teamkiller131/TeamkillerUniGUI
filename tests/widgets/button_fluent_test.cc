#include <unigui/widgets/button.h>

#include <imgui.h>
#include <gtest/gtest.h>

#include <type_traits>

// CRTP fluent API: base With* helpers must return Button& (not Widget&) so that
// derived-specific With* helpers can be chained after them.
class ButtonFluentTest : public ::testing::Test {
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

TEST_F(ButtonFluentTest, BaseWithReturnsDerivedReference) {
    unigui::Button btn("b", "Save");
    static_assert(
        std::is_same_v<decltype(btn.WithTooltip("x")), unigui::Button&>,
        "WithTooltip must return Button& via the CRTP FluentWidget base");
    static_assert(
        std::is_same_v<decltype(btn.WithEnabled(true)), unigui::Button&>,
        "WithEnabled must return Button&");
}

TEST_F(ButtonFluentTest, ChainMixesBaseAndDerivedHelpers) {
    bool clicked = false;
    unigui::Button btn("b", "Save");
    // Base helper (WithTooltip) followed by derived helpers (WithPrimary,
    // WithOnClick) — only compiles if the chain stays Button&.
    btn.WithTooltip("Ctrl+S")
        .WithEnabled(true)
        .WithPrimary()
        .WithOnClick([&] { clicked = true; });
    EXPECT_TRUE(btn.IsEnabled());
    (void)clicked;
}

TEST_F(ButtonFluentTest, DerivedWithHelpersMutateState) {
    unigui::Button btn("b", "Old");
    btn.WithLabel("New");
    EXPECT_EQ(btn.GetLabel(), "New");
}

TEST_F(ButtonFluentTest, UpcastToWidgetStillWorks) {
    unigui::Button btn("b", "Save");
    unigui::Widget* w = &btn;  // FluentWidget<Button> IS-A Widget
    w->SetEnabled(false);
    EXPECT_FALSE(btn.IsEnabled());
}
