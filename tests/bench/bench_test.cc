#include <unigui/unigui.h>
#include <imgui.h>
#include <gtest/gtest.h>
#include <chrono>
#include <vector>
class BenchTest:public ::testing::Test{
protected:void SetUp()override{ImGui::CreateContext();ImGui::GetIO().DisplaySize=ImVec2(800,600);ImGui::GetIO().Fonts->Build();ImGui::NewFrame();}
void TearDown()override{ImGui::Render();ImGui::DestroyContext();}};
TEST_F(BenchTest,FrameTime_100Buttons){std::vector<unigui::Button> bs;for(int i=0;i<100;i++)bs.emplace_back("b"+std::to_string(i),"B");auto t0=std::chrono::high_resolution_clock::now();for(auto&b:bs)b.Render();auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-t0).count();EXPECT_LT(us,20000);}
TEST_F(BenchTest,FrameTime_100Labels){std::vector<unigui::Label> ls;for(int i=0;i<100;i++)ls.emplace_back("l"+std::to_string(i),"L");auto t0=std::chrono::high_resolution_clock::now();for(auto&l:ls)l.Render();auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-t0).count();EXPECT_LT(us,10000);}
TEST_F(BenchTest,VirtualList_10k){unigui::VirtualList vl("vl",10000);vl.SetItemGetter([](int i){return"I"+std::to_string(i);});auto t0=std::chrono::high_resolution_clock::now();vl.Render();ImGui::Render();auto ms=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now()-t0).count();EXPECT_LT(ms,100);}
TEST_F(BenchTest,Form_20Fields){unigui::Form f("f","T");for(int i=0;i<20;i++)f.AddTextField("f"+std::to_string(i),"F");auto t0=std::chrono::high_resolution_clock::now();f.Render();auto us=std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now()-t0).count();EXPECT_LT(us,30000);}
