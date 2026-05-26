#include <unigui/unigui.h>
#include <unigui/widgets/passwordinput.h>
#include <imgui.h>
#include <gtest/gtest.h>
class PasswordInputTest:public ::testing::Test{
protected:void SetUp()override{ImGui::CreateContext();ImGui::GetIO().DisplaySize=ImVec2(800,600);ImGui::GetIO().Fonts->Build();ImGui::NewFrame();}
void TearDown()override{ImGui::Render();ImGui::DestroyContext();}};
TEST_F(PasswordInputTest,Render_DoesNotCrash){unigui::PasswordInput pi("pi","Password");pi.Render();}
TEST_F(PasswordInputTest,EmptyPassword_StrengthZero){unigui::PasswordInput pi("pi","Pwd","");EXPECT_EQ(pi.GetStrengthScore(),0);}
