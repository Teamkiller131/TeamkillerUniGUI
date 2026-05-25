#include <unigui/unigui.h>
#include <imgui.h>
#include <gtest/gtest.h>
class InputIntTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(InputIntTest, GetValue_DefaultsToZero) { unigui::InputInt ii("ii","Int"); EXPECT_EQ(ii.GetValue(), 0); }
TEST_F(InputIntTest, Render_DoesNotCrash) { unigui::InputInt ii("ii","Int",42); ii.Render(); }
