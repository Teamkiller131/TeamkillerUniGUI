#include <unigui/im/im.h>

#include <imgui.h>
#include <gtest/gtest.h>

#include <string>
#include <vector>

// Immediate-mode free functions (unigui::im). These exercise the wrappers for
// crashes and correct value binding within a headless ImGui frame, mirroring the
// existing widget tests' fixture pattern.
class ImTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
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

TEST_F(ImTest, Button_DoesNotCrash) {
    unigui::im::Button("Click");
    unigui::im::Button("Primary", unigui::im::ButtonVariant::Primary);
    unigui::im::Button("Danger", unigui::im::ButtonVariant::Danger);
    unigui::im::Button("Success", unigui::im::ButtonVariant::Success);
    unigui::im::SmallButton("Small");
}

TEST_F(ImTest, Text_VariantsDoNotCrash) {
    unigui::im::Text("plain");
    unigui::im::TextWrapped("wrapped text that is reasonably long");
    unigui::im::TextDisabled("disabled");
    unigui::im::TextColored(ImVec4(1, 0, 0, 1), "red");
    unigui::im::BulletText("bullet");
    unigui::im::LabelText("Label", "Value");
}

TEST_F(ImTest, Checkbox_BindsValue) {
    bool v = false;
    unigui::im::Checkbox("flag", &v);  // no click: stays false
    EXPECT_FALSE(v);
}

TEST_F(ImTest, SliderFloat_DoesNotCrash) {
    float f = 0.5f;
    unigui::im::SliderFloat("gain", &f, 0.f, 1.f);
    EXPECT_GE(f, 0.f);
    EXPECT_LE(f, 1.f);
}

TEST_F(ImTest, SliderInt_DoesNotCrash) {
    int i = 3;
    unigui::im::SliderInt("count", &i, 0, 10);
    EXPECT_EQ(i, 3);
}

TEST_F(ImTest, DragAndInputNumeric_DoNotCrash) {
    float f = 1.0f;
    int i = 2;
    unigui::im::DragFloat("df", &f);
    unigui::im::DragInt("di", &i);
    unigui::im::InputFloat("if", &f);
    unigui::im::InputInt("ii", &i);
}

TEST_F(ImTest, InputText_BindsStringAndStaysUnchanged) {
    std::string s = "hello";
    bool changed = unigui::im::InputText("name", &s);
    EXPECT_FALSE(changed);
    EXPECT_EQ(s, "hello");
}

TEST_F(ImTest, InputText_NullPointerReturnsFalse) {
    EXPECT_FALSE(unigui::im::InputText("name", nullptr));
    EXPECT_FALSE(unigui::im::InputTextMultiline("ml", nullptr));
}

TEST_F(ImTest, InputTextMultiline_DoesNotCrash) {
    std::string s = "line1\nline2";
    unigui::im::InputTextMultiline("body", &s);
    EXPECT_EQ(s, "line1\nline2");
}

TEST_F(ImTest, Combo_BindsIndex) {
    int idx = 1;
    std::vector<std::string> items = {"A", "B", "C"};
    unigui::im::Combo("pick", &idx, items);
    EXPECT_EQ(idx, 1);
}

TEST_F(ImTest, Combo_NullPointerReturnsFalse) {
    std::vector<std::string> items = {"A"};
    EXPECT_FALSE(unigui::im::Combo("pick", nullptr, items));
}

TEST_F(ImTest, Layout_HelpersDoNotCrash) {
    unigui::im::Text("a");
    unigui::im::SameLine();
    unigui::im::Text("b");
    unigui::im::NewLine();
    unigui::im::Spacing();
    unigui::im::Separator();
    unigui::im::SeparatorText("section");
    unigui::im::Dummy(10, 10);
    unigui::im::Indent();
    unigui::im::Unindent();
    unigui::im::Bullet();
}
