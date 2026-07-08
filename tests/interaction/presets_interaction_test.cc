// Interaction-driven tests for the UI preset scaffolds — the test engine clicks and
// types through the composed presets, asserting the full path: driven input → composed
// widget → preset state/callback. Compiled only under UNIGUI_TEST_ENGINE (the presets
// module is ON by default, so no extra feature gating is needed beyond the engine).

#include <unigui/core/accessibility.h>
#include <unigui/presets/app_shell.h>
#include <unigui/presets/login_page.h>
#include <unigui/presets/settings_page.h>
#include <unigui/presets/wizard_flow.h>

#include <string>

#include "interaction_harness.h"

class PresetInteractionTest : public itest::InteractionFixture {};

// ── AppShell: clicking a sidebar entry switches the page ─────────────────────
TEST_F(PresetInteractionTest, AppShell_SidebarClick_SwitchesPage) {
    unigui::presets::AppShell shell("pi_shell", "App");
    shell.AddPage("Home", [] { ImGui::TextUnformatted("home"); }).AddPage("Reports", [] {
        ImGui::TextUnformatted("reports");
    });
    int changedTo = -1;
    shell.WithOnPageChange([&](int i) { changedTo = i; });

    // The shell renders its own fullscreen host window (not the harness's "TW"),
    // so address the sidebar entry through the wildcard from the root.
    const auto status = Run(
        "preset_shell_sidebar", [&] { shell.Render(); },
        [](ImGuiTestContext* ctx) { ctx->ItemClick("//**/Reports"); });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(shell.GetActivePage(), 1);
    EXPECT_EQ(changedTo, 1);
}

// ── SettingsPage: clicking a toggle row fires the setter ─────────────────────
TEST_F(PresetInteractionTest, SettingsPage_ToggleClick_FiresSetter) {
    bool dark = false;
    unigui::presets::SettingsPage page("pi_settings");
    page.AddToggle("Dark mode", [&] { return dark; }, [&](bool v) { dark = v; });

    const auto status = Run(
        "preset_settings_toggle", [&] { page.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/##toggle"); // Checkbox("##toggle") under the per-row PushID
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_TRUE(dark);
}

// ── LoginPage: type a username, click Sign in → OnSubmit carries the value ───
TEST_F(PresetInteractionTest, LoginPage_TypeAndSubmit_FiresCallback) {
    unigui::presets::LoginPage login("pi_login");
    std::string user;
    bool fired = false;
    login.WithOnSubmit([&](const std::string& u, const std::string&, bool) {
        user = u;
        fired = true;
    });

    const auto status = Run(
        "preset_login_submit", [&] { login.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemInputValue("**/Username", "carol");
            ctx->ItemClick("**/Sign in");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_TRUE(fired);
    EXPECT_EQ(user, "carol");
}

// ── WizardFlow: Next advances; a failing gate blocks it ──────────────────────
TEST_F(PresetInteractionTest, WizardFlow_NextClick_AdvancesAndRespectsGate) {
    bool gateOpen = false;
    unigui::presets::WizardFlow wiz("pi_wiz");
    wiz.AddStep("Welcome", [] { ImGui::TextUnformatted("hi"); })
        .AddStep(
            "Details", [] { ImGui::TextUnformatted("details"); }, [&] { return gateOpen; })
        .AddStep("Done", [] { ImGui::TextUnformatted("done"); });

    const auto status = Run(
        "preset_wizard_next", [&] { wiz.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Next"); // step 0 -> 1 (ungated)
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(wiz.GetCurrentStep(), 1);
    // Step 1's gate is closed: programmatic Next must refuse to advance.
    wiz.Next();
    EXPECT_EQ(wiz.GetCurrentStep(), 1);
    gateOpen = true;
    wiz.Next();
    EXPECT_EQ(wiz.GetCurrentStep(), 2);
}
