#include <unigui/unigui.h>
#include <unigui/widgets/virtuallist.h>
#include <imgui.h>
#include <gtest/gtest.h>
class VirtualListTest:public ::testing::Test{
protected:void SetUp()override{ImGui::CreateContext();ImGui::GetIO().DisplaySize=ImVec2(800,600);ImGui::GetIO().Fonts->Build();ImGui::NewFrame();}
void TearDown()override{ImGui::Render();ImGui::DestroyContext();}};
TEST_F(VirtualListTest,Render_DoesNotCrash){unigui::VirtualList vl("vl",100);vl.SetItemGetter([](int i){return "Item "+std::to_string(i);});vl.Render();}
TEST_F(VirtualListTest,GetCount_ReturnsCount){unigui::VirtualList vl("vl",42);EXPECT_EQ(vl.GetItemCount(),42);}
