#include <unigui/unigui.h>
#include <unigui/widgets/multicombo.h>
#include <imgui.h>
#include <gtest/gtest.h>
class MultiComboTest:public ::testing::Test{
protected:void SetUp()override{ImGui::CreateContext();ImGui::GetIO().DisplaySize=ImVec2(800,600);ImGui::GetIO().Fonts->Build();ImGui::NewFrame();}
void TearDown()override{ImGui::Render();ImGui::DestroyContext();}};
TEST_F(MultiComboTest,Render_DoesNotCrash){unigui::MultiCombo mc("mc","Select",{"A","B","C"});mc.Render();}
TEST_F(MultiComboTest,Selected_DefaultsEmpty){unigui::MultiCombo mc("mc","X",{"1","2","3"});EXPECT_TRUE(mc.GetSelectedIndices().empty());}
TEST_F(MultiComboTest,Render_LongItemsDoesNotCrash){unigui::MultiCombo mc("mc","Select",{"超长下拉选项内容A","超长下拉选项内容B"});mc.Render();}
