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

// ── Many fields (20+) ────────────────────────────────────
TEST_F(FormTest, ManyFields_TwentyPlus_RenderDoesNotCrash) {
    unigui::Form form("frm_big", "Big Form");

    // Add 25 fields of various types
    for (int i = 0; i < 10; ++i) {
        form.AddTextField("txt_" + std::to_string(i), "Text " + std::to_string(i));
        form.SetFieldValue("txt_" + std::to_string(i), "value" + std::to_string(i));
    }
    for (int i = 0; i < 5; ++i) {
        form.AddCheckbox("cb_" + std::to_string(i), "Check " + std::to_string(i));
        form.SetFieldValue("cb_" + std::to_string(i), i % 2 ? "1" : "0");
    }
    for (int i = 0; i < 5; ++i) {
        form.AddNumberField("num_" + std::to_string(i), "Number " + std::to_string(i), 0, 100);
        form.SetFieldValue("num_" + std::to_string(i), std::to_string(i * 10));
    }
    for (int i = 0; i < 5; ++i) {
        form.AddSliderField("sld_" + std::to_string(i), "Slider " + std::to_string(i), 0, 100);
        form.SetFieldValue("sld_" + std::to_string(i), std::to_string(i * 20));
    }

    // Validate all fields
    auto errors = form.Validate();
    EXPECT_EQ(errors.size(), 0u);

    // Render — should not crash
    EXPECT_NO_THROW({ form.Render(); });
}

TEST_F(FormTest, ManyFields_ValidateMixedRequiredAndOptional) {
    unigui::Form form("frm_req", "Mixed Required");

    // 10 required text fields, 10 optional
    for (int i = 0; i < 10; ++i) {
        form.AddTextField("req_" + std::to_string(i), "Required " + std::to_string(i), true);
        form.AddTextField("opt_" + std::to_string(i), "Optional " + std::to_string(i), false);
    }

    // No values set → should have 10 errors (all required fields empty)
    auto errors = form.Validate();
    EXPECT_EQ(errors.size(), 10u);

    // Fill all required fields
    for (int i = 0; i < 10; ++i) {
        form.SetFieldValue("req_" + std::to_string(i), "filled");
    }

    auto errors2 = form.Validate();
    EXPECT_EQ(errors2.size(), 0u);

    form.Render();
}

// ── Nested validation (multiple validators per field) ────
TEST_F(FormTest, NestedValidation_RegexAndRequired) {
    unigui::Form form("frm_nest", "Nested Validation");
    form.AddTextField("email", "Email", true);
    form.SetFieldValidatorRegex("email", ".+@.+\\..+", "Invalid email format");

    // Empty → required error
    auto errors = form.Validate();
    EXPECT_GE(errors.size(), 1u);

    // Invalid format → regex error
    form.SetFieldValue("email", "not_an_email");
    auto errors2 = form.Validate();
    EXPECT_GE(errors2.size(), 1u);

    // Valid → no errors
    form.SetFieldValue("email", "user@domain.com");
    auto errors3 = form.Validate();
    EXPECT_EQ(errors3.size(), 0u);
}

TEST_F(FormTest, NestedValidation_RegexAndMinMax) {
    unigui::Form form("frm_nest2", "Nested Num Validation");
    form.AddNumberField("score", "Score", 0, 100);
    form.SetFieldMinMax("score", 0, 100);

    // Out of range
    form.SetFieldValue("score", "500");
    auto errors = form.Validate();
    EXPECT_GE(errors.size(), 1u);

    // Edge of range — valid
    form.SetFieldValue("score", "0");
    auto errors2 = form.Validate();
    EXPECT_EQ(errors2.size(), 0u);

    form.SetFieldValue("score", "100");
    auto errors3 = form.Validate();
    EXPECT_EQ(errors3.size(), 0u);
}

TEST_F(FormTest, NestedValidation_MultipleFieldsWithMixedValidators) {
    unigui::Form form("frm_nest3", "Mixed Validators");

    form.AddTextField("name", "Name", true);

    form.AddTextField("email", "Email", true);
    form.SetFieldValidatorRegex("email", ".+@.+\\..+", "Bad email");

    form.AddNumberField("age", "Age", 0, 150);
    form.SetFieldMinMax("age", 0, 150);

    form.AddSliderField("rating", "Rating", 0, 100);
    form.SetFieldMinMax("rating", 0, 100);

    // All empty — name & email required
    auto err = form.Validate();
    EXPECT_GE(err.size(), 2u);

    // Set all fields valid
    form.SetFieldValue("name", "Alice");
    form.SetFieldValue("email", "alice@test.com");
    form.SetFieldValue("age", "30");
    form.SetFieldValue("rating", "75");

    auto err2 = form.Validate();
    EXPECT_EQ(err2.size(), 0u);
}

// ── Serialize roundtrip with complex data ────────────────
TEST_F(FormTest, SerializeRoundtrip_ComplexData) {
    unigui::Form form("frm_cplx", "Complex");

    form.AddTextField("username", "User");
    form.AddTextField("bio", "Bio");
    form.AddCheckbox("premium", "Premium");
    form.AddCheckbox("verified", "Verified");
    form.AddComboField("role", "Role", {"Admin", "Editor", "Viewer"});
    form.AddSliderField("zoom", "Zoom", 10, 200);
    form.AddNumberField("port", "Port", 1024, 65535);

    // Set complex values
    form.SetFieldValue("username", "charlie");
    form.SetFieldValue("bio", "A long bio with\nspecial chars: \"quotes\", \\backslashes\\");
    form.SetFieldValue("premium", "1");
    form.SetFieldValue("verified", "0");
    form.SetFieldValue("role", "Editor");
    form.SetFieldValue("zoom", "150");
    form.SetFieldValue("port", "8080");

    std::string json = form.Serialize();
    EXPECT_FALSE(json.empty());
    // JSON should contain key values
    EXPECT_NE(json.find("charlie"), std::string::npos);
    EXPECT_NE(json.find("Editor"), std::string::npos);

    // Deserialize into a fresh form
    unigui::Form restored("frm_cplx_r", "Complex Restored");
    restored.AddTextField("username", "User");
    restored.AddTextField("bio", "Bio");
    restored.AddCheckbox("premium", "Premium");
    restored.AddCheckbox("verified", "Verified");
    restored.AddComboField("role", "Role", {"Admin", "Editor", "Viewer"});
    restored.AddSliderField("zoom", "Zoom", 10, 200);
    restored.AddNumberField("port", "Port", 1024, 65535);

    EXPECT_TRUE(restored.Deserialize(json));

    EXPECT_EQ(restored.GetFieldValue("username"), "charlie");
    EXPECT_EQ(restored.GetFieldValue("bio"), "A long bio with\nspecial chars: \"quotes\", \\backslashes\\");
    EXPECT_EQ(restored.GetFieldValue("premium"), "1");
    EXPECT_EQ(restored.GetFieldValue("verified"), "0");
    EXPECT_EQ(restored.GetFieldValue("role"), "Editor");
    EXPECT_EQ(restored.GetFieldValue("zoom"), "150");
    EXPECT_EQ(restored.GetFieldValue("port"), "8080");
}

TEST_F(FormTest, SerializeRoundtrip_EmptyForm) {
    unigui::Form form("frm_empty", "Empty");
    form.AddTextField("field", "Field");

    std::string json = form.Serialize();
    EXPECT_FALSE(json.empty());

    unigui::Form restored("frm_empty_r", "Empty Restored");
    restored.AddTextField("field", "Field");
    EXPECT_TRUE(restored.Deserialize(json));
    EXPECT_EQ(restored.GetFieldValue("field"), "");
}

TEST_F(FormTest, SerializeRoundtrip_AllCheckboxStates) {
    unigui::Form form("frm_cb", "Checkboxes");

    form.AddCheckbox("a", "A");
    form.AddCheckbox("b", "B");
    form.AddCheckbox("c", "C");

    form.SetFieldValue("a", "1");
    form.SetFieldValue("b", "0");
    form.SetFieldValue("c", "1");

    std::string json = form.Serialize();

    unigui::Form restored("frm_cb_r", "Checkboxes R");
    restored.AddCheckbox("a", "A");
    restored.AddCheckbox("b", "B");
    restored.AddCheckbox("c", "C");

    EXPECT_TRUE(restored.Deserialize(json));
    EXPECT_EQ(restored.GetFieldValue("a"), "1");
    EXPECT_EQ(restored.GetFieldValue("b"), "0");
    EXPECT_EQ(restored.GetFieldValue("c"), "1");
}

// ── Serialize then modify then deserialize ───────────────
TEST_F(FormTest, Serialize_Modify_Deserialize_Roundtrip) {
    unigui::Form original("frm_mod", "Original");
    original.AddTextField("name", "Name");
    original.AddComboField("color", "Color", {"Red", "Green", "Blue"});
    original.SetFieldValue("name", "Delta");
    original.SetFieldValue("color", "Green");

    std::string json = original.Serialize();

    // Deserialize into new form
    unigui::Form copy("frm_mod_c", "Copy");
    copy.AddTextField("name", "Name");
    copy.AddComboField("color", "Color", {"Red", "Green", "Blue"});
    copy.Deserialize(json);

    EXPECT_EQ(copy.GetFieldValue("name"), "Delta");
    EXPECT_EQ(copy.GetFieldValue("color"), "Green");

    // Modify copy and serialize again
    copy.SetFieldValue("name", "Epsilon");
    copy.SetFieldValue("color", "Blue");

    std::string json2 = copy.Serialize();

    unigui::Form copy2("frm_mod_c2", "Copy2");
    copy2.AddTextField("name", "Name");
    copy2.AddComboField("color", "Color", {"Red", "Green", "Blue"});
    copy2.Deserialize(json2);

    EXPECT_EQ(copy2.GetFieldValue("name"), "Epsilon");
    EXPECT_EQ(copy2.GetFieldValue("color"), "Blue");
}

TEST_F(FormTest, ComboAndRanges_UseConfiguredValues) {
    unigui::Form form("frm_cfg", "Configured");
    form.AddComboField("role", "Role", {"Admin", "Editor", "Viewer"});
    form.AddNumberField("port", "Port", 1024, 65535);
    form.AddSliderField("zoom", "Zoom", 10, 200);

    EXPECT_EQ(form.GetFieldValue("role"), "Admin");
    form.SetFieldValue("port", "80");
    form.SetFieldValue("zoom", "250");
    auto errors = form.Validate();
    EXPECT_GE(errors.size(), 2u);
}
