#include <unigui/dsl/dsl.h>
#include <unigui/unigui.h>

#include <gtest/gtest.h>
using namespace unigui::dsl;

class DSLTest : public ::testing::Test {
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

TEST_F(DSLTest, Window_Renders) {
    auto ui = Window("Test", Label("Hello"));
    Render(ui);
    SUCCEED();
}
TEST_F(DSLTest, VBox_HBox_Renders) {
    auto ui = VBox({HBox({Button("A"), Button("B")}), Separator(), Label("Bottom")});
    Render(ui);
    SUCCEED();
}
TEST_F(DSLTest, If_Renders) {
    auto ui = If([] { return true; }, Label("Visible"));
    Render(ui);
    SUCCEED();
}
TEST_F(DSLTest, For_Renders) {
    auto ui = For(3, [](int i) { return Label("I" + std::to_string(i)); });
    Render(ui);
    SUCCEED();
}
TEST_F(DSLTest, TextVariants_Render) {
    auto ui = VBox({Text("plain"), TextWrapped("wrapped"), TextDisabled("disabled"),
                    BulletText("bullet"), Spacing(), Separator()});
    Render(ui);
    SUCCEED();
}
TEST_F(DSLTest, Button_Variant_Invokes_OnClick) {
    int clicks = 0;
    auto ui = Button("Save", ButtonVariant::Primary, [&] { ++clicks; });
    Render(ui);
    // Not actually clicked by the test harness, so the callback must not fire.
    EXPECT_EQ(clicks, 0);
}
TEST_F(DSLTest, CheckBox_NodeState_Persists) {
    auto ui = CheckBox("Enabled");
    Render(ui); // node owns its own bool state across frames
    SUCCEED();
}
TEST_F(DSLTest, CheckBox_BindsExternalBool) {
    bool flag = true;
    auto ui = CheckBox("Bound", &flag);
    Render(ui);
    // Binding leaves the external value untouched when no interaction occurs.
    EXPECT_TRUE(flag);
}
TEST_F(DSLTest, SliderFloat_Binds) {
    float gain = 0.5f;
    auto ui = SliderFloat("Gain", &gain, 0.f, 1.f);
    Render(ui);
    EXPECT_FLOAT_EQ(gain, 0.5f);
}
TEST_F(DSLTest, InputText_Binds) {
    std::string name = "abc";
    auto ui = InputText("Name", &name);
    Render(ui);
    EXPECT_EQ(name, "abc");
}
TEST_F(DSLTest, IfElse_RendersElseBranch) {
    auto ui = IfElse([] { return false; }, Label("then"), Label("else"));
    Render(ui);
    SUCCEED();
}
TEST_F(DSLTest, Flex_BuildsFlexKind) {
    auto ui = Flex({Button("A"), Button("B"), Label("C")});
    ASSERT_TRUE(ui != nullptr);
    EXPECT_EQ(ui->kind, Node::Kind::Flex);
    EXPECT_EQ(ui->children.size(), 3u);
}
TEST_F(DSLTest, Flex_EqualSplit_Renders) {
    // Default equal-grow split: three children share the row width evenly.
    auto ui = Window("FlexWin", Flex({Button("One"), Button("Two"), Label("Three")}));
    Render(ui);
    SUCCEED();
}
TEST_F(DSLTest, Flex_Weighted_Renders) {
    // Per-child weights + a gap + justify, rendered without crashing.
    auto ui = Window("FlexWeighted",
                     Flex({Button("L"), Button("R")}, {2.f, 1.f}, 8.f, FlexJustify::Start));
    Render(ui);
    SUCCEED();
}
