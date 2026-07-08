#include <unigui/core/accessibility.h>
#include <unigui/presets/login_page.h>
#include <unigui/theme/theme.h>

#include <imgui.h>

#include <algorithm>

namespace unigui::presets {

LoginPage::LoginPage(std::string name)
        : FluentWidget<LoginPage>(std::move(name))
        , username_(GetName() + ".user", "Username")
        , password_(GetName() + ".pass", "Password")
        , remember_(GetName() + ".remember", "Remember me", false)
        , submit_(GetName() + ".submit", "Sign in") {
    submit_.SetOnClick([this] { Submit(); });
    submit_.SetColorVariant(Button::Primary);
    submit_.SetSize(Button::Large);
}

// ── Configuration ─────────────────────────────────────────────────────────────

LoginPage& LoginPage::WithTitle(std::string title) {
    title_ = std::move(title);
    return *this;
}

LoginPage& LoginPage::WithLogo(std::function<void()> draw) {
    logo_ = std::move(draw);
    return *this;
}

LoginPage& LoginPage::WithRememberMe(bool show) {
    showRemember_ = show;
    return *this;
}

LoginPage& LoginPage::WithOnSubmit(
    std::function<void(const std::string&, const std::string&, bool)> fn) {
    onSubmit_ = std::move(fn);
    return *this;
}

// ── Live state ────────────────────────────────────────────────────────────────

void LoginPage::SetStatus(std::string text, bool isError) {
    status_ = std::move(text);
    statusIsError_ = isError;
    if (!status_.empty())
        a11y::Announce(status_, isError ? a11y::Live::Assertive : a11y::Live::Polite);
}

void LoginPage::SetBusy(bool on) {
    busy_ = on;
}

std::string LoginPage::GetUsername() const {
    return username_.GetValue();
}

void LoginPage::SetUsername(std::string user) {
    username_.SetValue(std::move(user));
}

bool LoginPage::GetRememberMe() const {
    return remember_.IsChecked();
}

void LoginPage::Submit() {
    if (busy_)
        return; // an auth attempt is already in flight
    // Announce the attempt — never the password (a11y sees presence only).
    a11y::Announce("Signing in");
    if (onSubmit_)
        onSubmit_(username_.GetValue(), password_.GetValue(), remember_.IsChecked());
}

// ── Render ────────────────────────────────────────────────────────────────────

void LoginPage::Render() {
    if (!IsVisible())
        return;

    ImGui::PushID(GetName().c_str());

    // Centre a fixed-width card in the available region (top-third bias reads
    // better than exact vertical centring on tall windows).
    const float cardW = 380.f;
    const ImVec2 avail = ImGui::GetContentRegionAvail();
    const float padX = std::max((avail.x - cardW) * 0.5f, 0.f);
    const float padY = std::max(avail.y * 0.18f, 0.f);
    // Centre via real items (Dummy + SameLine): teleporting the cursor with
    // SetCursorPosX past the parent's boundary asserts in imgui unless an item
    // grows the boundary first.
    if (padY > 0.f)
        ImGui::Dummy(ImVec2(1.f, padY));
    if (padX > 0.f) {
        ImGui::Dummy(ImVec2(padX, 1.f));
        ImGui::SameLine(0.f, 0.f);
    }

    ImGui::BeginChild("##card", ImVec2(cardW, 0.f),
                      ImGuiChildFlags_Borders | ImGuiChildFlags_AutoResizeY);

    if (logo_) {
        logo_();
        ImGui::Spacing();
    }
    ImGui::TextUnformatted(title_.c_str());
    ImGui::Separator();
    ImGui::Spacing();

    if (busy_)
        ImGui::BeginDisabled();

    ImGui::PushItemWidth(-1.f);
    username_.Render();
    password_.Render();
    ImGui::PopItemWidth();
    if (showRemember_)
        remember_.Render();

    ImGui::Spacing();
    submit_.SetLabel(busy_ ? "Signing in…" : "Sign in");
    submit_.Render();

    if (busy_)
        ImGui::EndDisabled();

    // Enter submits from anywhere in the form (standard login-page behaviour).
    // Checked window-locally so an Enter in another window doesn't fire it.
    if (!busy_ && ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows) &&
        ImGui::IsKeyPressed(ImGuiKey_Enter, false))
        Submit();

    if (!status_.empty()) {
        ImGui::Spacing();
        if (statusIsError_) {
            ImGui::PushStyleColor(
                ImGuiCol_Text, ImGui::GetColorU32(theme::GetSemanticColor(theme::Semantic::Down)));
            ImGui::TextWrapped("%s", status_.c_str());
            ImGui::PopStyleColor();
        } else {
            ImGui::TextWrapped("%s", status_.c_str());
        }
    }

    ImGui::EndChild();

    // Container node: the card + its state, never the password value.
    ReportAccessible(a11y::Role::Group, ImGui::IsItemFocused(),
                     busy_ ? title_ + " (signing in)" : title_);

    ImGui::PopID();
}

} // namespace unigui::presets
