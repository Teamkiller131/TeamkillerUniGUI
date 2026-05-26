#include <unigui/unigui.h>
#include <unigui/widgets/wizard.h>
#include <imgui.h>
#include <gtest/gtest.h>
class WizardTest:public ::testing::Test{
protected:void SetUp()override{ImGui::CreateContext();ImGui::GetIO().DisplaySize=ImVec2(800,600);ImGui::GetIO().Fonts->Build();ImGui::NewFrame();}
void TearDown()override{ImGui::Render();ImGui::DestroyContext();}};
TEST_F(WizardTest,Render_DoesNotCrash){unigui::Wizard wz("wz");wz.AddStep("s1","Step 1",[](){ImGui::Text("S1");});wz.Render();}
TEST_F(WizardTest,GetStepCount_Works){unigui::Wizard wz("wz");wz.AddStep("a","A",[](){});wz.AddStep("b","B",[](){});EXPECT_EQ(wz.GetStepCount(),2);}
