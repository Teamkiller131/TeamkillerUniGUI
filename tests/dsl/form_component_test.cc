// FormComponent + FormField (unigui::dsl) — forms/validation in the Component
// idiom. Headless: rule chains, touched semantics, the Submit() gate, and the
// ready-made Node() rows rendering inside a real ImGui frame.

#include <unigui/dsl/form_component.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>

using unigui::dsl::FormComponent;
using unigui::dsl::FormField;
using unigui::dsl::NodePtr;
using unigui::dsl::VBox;

namespace {

class SignupForm : public FormComponent {
public:
    FormField<std::string> user{this, "Username"};
    FormField<std::string> mail{this, "Email"};
    FormField<bool> terms{this, "Accept terms"};
    FormField<int> age{this, "Age"};
    int submits = 0;
    int rejects = 0;

    SignupForm() {
        user.Required().MinLength(3);
        mail.Required().Rule([](const std::string& v) {
            return v.find('@') == std::string::npos ? "Not an email" : std::string();
        });
        terms.Required("You must accept the terms");
        age.Range(18, 120);
    }
    void OnSubmit() override { ++submits; }
    void OnSubmitRejected() override { ++rejects; }
    NodePtr Build() override { return VBox({user.Node(), mail.Node(), terms.Node()}); }
};

} // namespace

class FormComponentTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
        ImGui::Begin("t");
    }
    void TearDown() override {
        ImGui::End();
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(FormComponentTest, RulesRunInOrder_FirstFailureWins) {
    SignupForm f;
    // Empty username: Required fires before MinLength.
    EXPECT_FALSE(f.user.Valid());
    EXPECT_EQ(f.user.Error(), "This field is required");
    f.user.Set("ab"); // non-empty but short: MinLength is now the first failure
    EXPECT_EQ(f.user.Error(), "Must be at least 3 characters");
    f.user.Set("abc");
    EXPECT_TRUE(f.user.Valid());
    EXPECT_TRUE(f.user.Error().empty());
}

TEST_F(FormComponentTest, Required_Bool_MeansTrue) {
    SignupForm f;
    EXPECT_FALSE(f.terms.Valid());
    EXPECT_EQ(f.terms.Error(), "You must accept the terms");
    f.terms.Set(true);
    EXPECT_TRUE(f.terms.Valid());
}

TEST_F(FormComponentTest, Range_Numeric_InclusiveBounds) {
    SignupForm f;
    EXPECT_FALSE(f.age.Valid()); // default 0 < 18
    f.age.Set(18);
    EXPECT_TRUE(f.age.Valid());
    f.age.Set(120);
    EXPECT_TRUE(f.age.Valid());
    f.age.Set(121);
    EXPECT_FALSE(f.age.Valid());
    EXPECT_EQ(f.age.Error(), "Must be between 18 and 120");
}

TEST_F(FormComponentTest, SetMarksTouched_TouchRevealsWithoutSet) {
    SignupForm f;
    EXPECT_FALSE(f.user.Touched()); // invalid but untouched: error exists, view hides it
    EXPECT_FALSE(f.user.Valid());
    f.user.Set("ok-name");
    EXPECT_TRUE(f.user.Touched());
    // Touch() alone (no Set) also reveals + revalidates.
    EXPECT_FALSE(f.mail.Touched());
    f.mail.Touch();
    EXPECT_TRUE(f.mail.Touched());
    EXPECT_FALSE(f.mail.Valid());
}

TEST_F(FormComponentTest, Submit_Invalid_TouchesAll_Rejects) {
    SignupForm f;
    EXPECT_FALSE(f.Submit());
    EXPECT_EQ(f.rejects, 1);
    EXPECT_EQ(f.submits, 0);
    // Every field is now touched, so every error is visible.
    for (const auto* field : f.Fields())
        EXPECT_TRUE(field->Touched()) << field->Label();
}

TEST_F(FormComponentTest, Submit_Valid_CallsOnSubmit) {
    SignupForm f;
    f.user.Set("alice");
    f.mail.Set("a@b.c");
    f.terms.Set(true);
    f.age.Set(30);
    EXPECT_TRUE(f.FormValid());
    EXPECT_TRUE(f.Submit());
    EXPECT_EQ(f.submits, 1);
    EXPECT_EQ(f.rejects, 0);
}

TEST_F(FormComponentTest, FormValid_TracksEveryField) {
    SignupForm f;
    EXPECT_FALSE(f.FormValid());
    f.user.Set("alice");
    f.mail.Set("a@b.c");
    f.terms.Set(true);
    EXPECT_FALSE(f.FormValid()); // age still 0
    f.age.Set(25);
    EXPECT_TRUE(f.FormValid());
}

TEST_F(FormComponentTest, FieldWrite_MarksComponentDirty_Rebuilds) {
    SignupForm f;
    f.Render();
    const int builds = f.BuildCount();
    f.user.Set("bob-name");
    f.Render();
    EXPECT_EQ(f.BuildCount(), builds + 1);
    f.Render(); // no change -> no rebuild
    EXPECT_EQ(f.BuildCount(), builds + 1);
}

TEST_F(FormComponentTest, NodeRows_RenderInFrame_NoCrash) {
    SignupForm f;
    f.user.Set(""); // touched + invalid: the danger error line draws too
    f.Render();
    SUCCEED();
}

TEST_F(FormComponentTest, FieldsEnumerateInDeclarationOrder) {
    SignupForm f;
    ASSERT_EQ(f.Fields().size(), 4u);
    EXPECT_EQ(f.Fields()[0]->Label(), "Username");
    EXPECT_EQ(f.Fields()[3]->Label(), "Age");
}
