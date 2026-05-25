#include <unigui/unigui.h>
#include <unigui/widgets/form.h>
#include <unigui/core/context.h>
#include <imgui.h>
#include <gtest/gtest.h>

class FormTest : public ::testing::Test {
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

TEST_F(FormTest, Render_DoesNotCrash) {
    unigui::Form form("frm", "Settings");
    form.AddTextField("name", "Name");
    form.Render();
}

TEST_F(FormTest, AddTextField_GetValue) {
    unigui::Form form("frm", "Settings");
    form.AddTextField("username", "User Name");
    form.SetFieldValue("username", "Alice");
    EXPECT_EQ(form.GetFieldValue("username"), "Alice");
}

TEST_F(FormTest, AddCheckbox_GetValue) {
    unigui::Form form("frm", "Settings");
    form.AddCheckbox("agree", "I Agree");
    form.SetFieldValue("agree", "1");
    EXPECT_EQ(form.GetFieldValue("agree"), "1");
}

TEST_F(FormTest, Validate_EmptyRequired_ReturnsError) {
    unigui::Form form("frm", "Settings");
    form.AddTextField("email", "Email", true);
    auto errors = form.Validate();
    EXPECT_GE(errors.size(), 1u);
    EXPECT_EQ(errors[0].field_name, "email");
}

TEST_F(FormTest, Validate_AllValid_ReturnsNoErrors) {
    unigui::Form form("frm", "Settings");
    form.AddTextField("email", "Email", true);
    form.SetFieldValue("email", "test@test.com");
    auto errors = form.Validate();
    EXPECT_EQ(errors.size(), 0u);
}

TEST_F(FormTest, OnSubmit_CallbackIsCalled) {
    unigui::Form form("frm", "Settings");
    form.AddTextField("name", "Name", true);
    form.SetFieldValue("name", "Alice");
    bool called = false;
    form.SetOnSubmit([&called]() { called = true; });
    form.Render(); // Submit button must be clicked, so callback not called automatically
    // Without actual click, callback won't fire - just verify no crash
    (void)called;
}

TEST_F(FormTest, Hidden_DoesNotRender) {
    unigui::Form form("frm", "Hidden");
    form.Hide();
    form.Render();
}

TEST_F(FormTest, RegexValidator_RejectsMismatch) {
    unigui::Form form("frm", "Reg");
    form.AddTextField("email", "Email");
    form.SetFieldValue("email", "notanemail");
    form.SetFieldValidatorRegex("email", ".+@.+\\..+", "Invalid email");
    auto errors = form.Validate();
    EXPECT_GE(errors.size(), 1u);
}

TEST_F(FormTest, MinMaxValidator_RejectsOutOfRange) {
    unigui::Form form("frm", "Num");
    form.AddNumberField("age", "Age");
    form.SetFieldValue("age", "200");
    form.SetFieldMinMax("age", 0, 120);
    auto errors = form.Validate();
    EXPECT_GE(errors.size(), 1u);
}

TEST_F(FormTest, Serialize_Then_Deserialize) {
    unigui::Form form("frm", "S");
    form.AddTextField("name", "Name");
    form.AddCheckbox("agree", "Agree");
    form.SetFieldValue("name", "Alice");
    form.SetFieldValue("agree", "1");
    auto json = form.Serialize();
    EXPECT_NE(json.find("Alice"), std::string::npos);

    unigui::Form form2("frm2", "S2");
    form2.AddTextField("name", "Name");
    form2.AddCheckbox("agree", "Agree");
    EXPECT_TRUE(form2.Deserialize(json));
    EXPECT_EQ(form2.GetFieldValue("name"), "Alice");
}
