#include <unigui/unigui.h>
#include <imgui.h>
#include <gtest/gtest.h>
class InputFloatTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(InputFloatTest, GetValue_DefaultsToZero) { unigui::InputFloat iif("iif","Float"); EXPECT_FLOAT_EQ(iif.GetValue(), 0.0f); }
TEST_F(InputFloatTest, Render_DoesNotCrash) { unigui::InputFloat iif("iif","Float",3.14f); iif.Render(); }
