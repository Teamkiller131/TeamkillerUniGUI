#include <unigui/styling/style_engine.h>
#include <imgui.h>
#include <gtest/gtest.h>
using namespace unigui::styling;

class StyleTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); Engine::Instance().Parse(""); }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(StyleTest, Parse_SingleRule) {
    int n = Engine::Instance().Parse("Window { bg: #ff0000; rounding: 8; }");
    EXPECT_EQ(n, 1);
    Engine::Instance().Apply("Window");
    auto& c = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    EXPECT_NEAR(c.x, 1.0f, 0.01f); // #ff0000 = red
}

TEST_F(StyleTest, Parse_MultipleRules) {
    int n = Engine::Instance().Parse("Window { bg: #111; } Button { bg: #222; }");
    EXPECT_EQ(n, 2);
}

TEST_F(StyleTest, ClassSelector) {
    Engine::Instance().Parse("Button.primary { rounding: 12; }");
    ImGui::GetStyle().WindowRounding = 6;
    Engine::Instance().Apply("Button", "primary");
    EXPECT_NEAR(ImGui::GetStyle().WindowRounding, 12.0f, 0.1f);
}

TEST_F(StyleTest, IDSelector_Priority) {
    Engine::Instance().Parse("Button { rounding: 4; } #submit { rounding: 16; }");
    Engine::Instance().Apply("Button", "", "submit");
    EXPECT_NEAR(ImGui::GetStyle().WindowRounding, 16.0f, 0.1f);
}
