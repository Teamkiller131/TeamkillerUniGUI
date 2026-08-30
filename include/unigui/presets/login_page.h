#pragma once
#include <unigui/widgets/button.h>
#include <unigui/widgets/checkbox.h>
#include <unigui/widgets/lineedit.h>
#include <unigui/widgets/passwordinput.h>
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui::presets {

// ─────────────────────────────────────────────────────────────────────────────
// LoginPage — sign-in / connect scaffold.
//
// A centred credentials card: optional logo slot, title, username, password
// (with the PasswordInput visibility toggle), optional remember-me, a
// full-width submit button, and a status/error line. Enter submits; a busy
// state disables the whole form while the host authenticates:
//
//     unigui::presets::LoginPage login("login");
//     login.WithTitle("Connect to feed")
//          .WithOnSubmit([&](const std::string& user, const std::string& pass,
//                            bool remember) {
//              login.SetBusy(true);
//              Authenticate(user, pass, remember); // async; then SetBusy(false)
//                                                  // + SetStatus(...) on the UI thread
//          });
//     // per frame: login.Render();
//
// The password value is never exposed to the accessibility layer or announced
// (PasswordInput reports presence only).
// ─────────────────────────────────────────────────────────────────────────────
class LoginPage : public FluentWidget<LoginPage> {
public:
    explicit LoginPage(std::string name);

    // ── Configuration (chainable) ───────────────────────────────────────
    /// Card heading (default "Sign in").
    LoginPage& WithTitle(std::string title);
    /// Draw a logo / banner above the title (any immediate-mode content).
    LoginPage& WithLogo(std::function<void()> draw);
    /// Show the remember-me checkbox (default true).
    LoginPage& WithRememberMe(bool show);
    /// Fired on submit (button or Enter) with the credentials + remember flag.
    LoginPage& WithOnSubmit(
        std::function<void(const std::string& user, const std::string& password, bool remember)>
            fn);

    // ── Live state ──────────────────────────────────────────────────────
    /// Status line under the form; `isError` renders it in the error colour
    /// and announces it assertively to assistive tech.
    void SetStatus(std::string text, bool isError = false);
    const std::string& GetStatus() const { return status_; }
    /// Disable the form while authenticating (shows "Signing in…").
    void SetBusy(bool on);
    bool IsBusy() const { return busy_; }
    /// By value: ValueWidget::GetValue() returns a copy, so a reference here
    /// would dangle.
    std::string GetUsername() const;
    void SetUsername(std::string user);
    bool GetRememberMe() const;
    const std::string& GetTitle() const { return title_; }
    /// Programmatic submit — same path as the button/Enter (no-op while busy).
    void Submit();

    void Render() override;

private:
    std::string title_ = "Sign in";
    std::function<void()> logo_;
    bool showRemember_ = true;
    std::function<void(const std::string&, const std::string&, bool)> onSubmit_;
    std::string status_;
    bool statusIsError_ = false;
    bool busy_ = false;

    LineEdit username_;
    PasswordInput password_;
    CheckBox remember_;
    Button submit_;
};

} // namespace unigui::presets
