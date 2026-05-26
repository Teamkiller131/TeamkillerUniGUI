#include <unigui/unigui.h>
#include <unigui/v2/dsl.h>
#include <gtest/gtest.h>
using namespace unigui::v2::dsl;

class DSLTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800,600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};

TEST_F(DSLTest, Window_Renders) {
    auto ui = Window("Test", Label("Hello"));
    Render(ui); SUCCEED();
}
TEST_F(DSLTest, VBox_HBox_Renders) {
    auto ui = VBox({
        HBox({Button("A"), Button("B")}),
        Separator(),
        Label("Bottom")
    });
    Render(ui); SUCCEED();
}
TEST_F(DSLTest, If_Renders) {
    auto ui = If([]{return true;}, Label("Visible"));
    Render(ui); SUCCEED();
}
TEST_F(DSLTest, For_Renders) {
    auto ui = For(3, [](int i){ return Label("I"+std::to_string(i)); });
    Render(ui); SUCCEED();
}
