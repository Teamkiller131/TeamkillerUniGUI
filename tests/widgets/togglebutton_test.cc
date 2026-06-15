#include <unigui/widgets/togglebutton.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui;

class ToggleButtonTest : public ::testing::Test {
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

TEST_F(ToggleButtonTest, DefaultsOffAndStoresState) {
    ToggleButton b("tb", "Start", "Stop");
    EXPECT_FALSE(b.IsOn());
    b.SetOn(true);
    EXPECT_TRUE(b.IsOn());
}

TEST_F(ToggleButtonTest, Render_NoClickNoToggle) {
    ToggleButton b("tb2");
    b.WithLabels("Run", "Halt").WithOnToggle([](bool) {});
    EXPECT_NO_THROW(b.Render());
    EXPECT_FALSE(b.WasToggled()); // headless: no click
}

TEST_F(ToggleButtonTest, DisabledPredicate_RendersDisabled) {
    ToggleButton b("tb3");
    b.WithEnabledPredicate([] { return false; }).WithDisabledTooltip("not ready");
    EXPECT_NO_THROW(b.Render());
    EXPECT_FALSE(b.WasToggled());
}

TEST_F(ToggleButtonTest, Fluent_KeepsType) {
    ToggleButton b("tb4");
    ToggleButton& ref = b.WithLabels("A", "B")
                            .WithColors(theme::Semantic::Success, theme::Semantic::Danger)
                            .WithButtonSize(80.f, 0.f);
    EXPECT_EQ(&ref, &b);
}
