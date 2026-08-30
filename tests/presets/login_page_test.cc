#include <unigui/core/accessibility.h>
#include <unigui/presets/login_page.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>

using unigui::presets::LoginPage;

class LoginPageTest : public ::testing::Test {
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

TEST_F(LoginPageTest, Defaults) {
    LoginPage login("lp");
    EXPECT_EQ(login.GetTitle(), "Sign in");
    EXPECT_EQ(login.GetUsername(), "");
    EXPECT_FALSE(login.GetRememberMe());
    EXPECT_FALSE(login.IsBusy());
    EXPECT_EQ(login.GetStatus(), "");
}

TEST_F(LoginPageTest, FluentChaining_ReturnsSelf) {
    LoginPage login("lp");
    LoginPage& ref = login.WithTitle("Connect").WithRememberMe(false).WithOnSubmit(
        [](const std::string&, const std::string&, bool) {});
    EXPECT_EQ(&ref, &login);
    EXPECT_EQ(login.GetTitle(), "Connect");
}

TEST_F(LoginPageTest, Submit_FiresCallback_WithCredentials) {
    LoginPage login("lp");
    login.SetUsername("alice");
    std::string user, pass;
    bool remembered = true;
    login.WithOnSubmit([&](const std::string& u, const std::string& p, bool r) {
        user = u;
        pass = p;
        remembered = r;
    });
    login.Submit();
    EXPECT_EQ(user, "alice");
    EXPECT_EQ(pass, ""); // password untouched programmatically
    EXPECT_FALSE(remembered);
}

TEST_F(LoginPageTest, Submit_WhileBusy_IsNoOp) {
    LoginPage login("lp");
    int fires = 0;
    login.WithOnSubmit([&](const std::string&, const std::string&, bool) { ++fires; });
    login.SetBusy(true);
    login.Submit();
    EXPECT_EQ(fires, 0);
    login.SetBusy(false);
    login.Submit();
    EXPECT_EQ(fires, 1);
}

TEST_F(LoginPageTest, SetStatus_TracksErrorFlagAndText) {
    LoginPage login("lp");
    login.SetStatus("Invalid credentials", /*isError=*/true);
    EXPECT_EQ(login.GetStatus(), "Invalid credentials");
}

// ── Accessibility ────────────────────────────────────────────────────────────
class LoginPageA11yTest : public LoginPageTest {
protected:
    void SetUp() override {
        LoginPageTest::SetUp();
        unigui::a11y::SetEnabled(true);
        unigui::a11y::BeginFrame();
        unigui::a11y::DrainAnnouncements();
    }
    void TearDown() override {
        unigui::a11y::SetEnabled(false);
        LoginPageTest::TearDown();
    }
};

TEST_F(LoginPageA11yTest, Submit_AnnouncesSigningIn_NeverThePassword) {
    LoginPage login("lp_a11y");
    login.SetUsername("bob");
    login.Submit();
    bool announced = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements()) {
        EXPECT_EQ(a.message.find("hunter2"), std::string::npos);
        if (a.message == "Signing in")
            announced = true;
    }
    EXPECT_TRUE(announced);
}

TEST_F(LoginPageA11yTest, ErrorStatus_AnnouncesAssertively) {
    LoginPage login("lp_a11y_err");
    login.SetStatus("Connection refused", /*isError=*/true);
    bool assertive = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements())
        if (a.message == "Connection refused" && a.politeness == unigui::a11y::Live::Assertive)
            assertive = true;
    EXPECT_TRUE(assertive);
}

TEST_F(LoginPageA11yTest, Render_ReportsGroup_WithoutSecrets) {
    LoginPage login("lp_a11y_r");
    login.Render();
    bool sawGroup = false;
    for (const auto& n : unigui::a11y::Tree()) {
        EXPECT_EQ(n.value.find("hunter2"), std::string::npos);
        if (n.role == unigui::a11y::Role::Group && n.value == "Sign in")
            sawGroup = true;
    }
    EXPECT_TRUE(sawGroup);
}

TEST_F(LoginPageTest, Render_DoesNotCrash) {
    LoginPage login("lp_render");
    login.WithLogo([] { ImGui::TextUnformatted("LOGO"); });
    login.SetBusy(true);
    login.Render();
    login.SetBusy(false);
    login.SetStatus("ok");
    login.Render();
}
