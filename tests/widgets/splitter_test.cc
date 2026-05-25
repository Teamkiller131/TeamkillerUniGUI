#include <unigui/unigui.h>
#include <unigui/widgets/splitter.h>
#include <imgui.h>
#include <gtest/gtest.h>
class SplitterTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(SplitterTest, Render_DoesNotCrash) { unigui::Splitter sp("sp"); sp.Render(); }
TEST_F(SplitterTest, Orientation_DefaultsToHorizontal) { unigui::Splitter sp("sp"); sp.SetContentA([]{ImGui::Text("A");}); sp.SetContentB([]{ImGui::Text("B");}); sp.Render(); }
