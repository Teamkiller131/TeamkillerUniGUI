#include <unigui/unigui.h>
#include <unigui/widgets/propertygrid.h>

#include <imgui.h>

#include <gtest/gtest.h>
class PropertyGridTest : public ::testing::Test {
protected:
    void SetUp() override {
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
TEST_F(PropertyGridTest, Render_DoesNotCrash) {
    unigui::PropertyGrid pg("pg");
    pg.AddProperty({"a", "Alpha", unigui::PropType::Bool, bool{true}});
    pg.Render();
}
TEST_F(PropertyGridTest, AddProperty_IncreasesCount) {
    unigui::PropertyGrid pg("pg");
    pg.AddProperty({"x", "X", unigui::PropType::Int, int{0}});
    EXPECT_EQ(pg.GetProperties().size(), 1u);
}
