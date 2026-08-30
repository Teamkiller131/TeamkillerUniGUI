// Interaction-driven tests for the UI preset scaffolds — the test engine clicks and
// types through the composed presets, asserting the full path: driven input → composed
// widget → preset state/callback. Compiled only under UNIGUI_TEST_ENGINE (the presets
// module is ON by default, so no extra feature gating is needed beyond the engine).

#include <unigui/core/accessibility.h>
#include <unigui/im/im.h>
#include <unigui/presets/app_shell.h>
#include <unigui/presets/dashboard.h>
#include <unigui/presets/log_console.h>
#include <unigui/presets/login_page.h>
#include <unigui/presets/master_detail.h>
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

// ── MasterDetail: clicking a browser row fires WithOnSelect ──────────────────
TEST_F(PresetInteractionTest, MasterDetail_RowClick_FiresOnSelect) {
    unigui::presets::MasterDetail md("pi_md");
    md.WithItems({"alpha", "beta"}).WithDetail([](int) { ImGui::TextUnformatted("detail"); });
    int selected = -1;
    md.WithOnSelect([&](int i) { selected = i; });

    const auto status = Run(
        "preset_masterdetail_row", [&] { md.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/beta");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(selected, 1);
}

// ── Dashboard: a card body's button is reachable and fires its callback ─────
TEST_F(PresetInteractionTest, Dashboard_CardButton_FiresCallback) {
    int pings = 0;
    unigui::presets::Dashboard dash("pi_dash");
    dash.AddMetric("Total", [] { return std::string("42"); })
        .AddCard("Ops", [&] {
            if (unigui::im::Button("Ping"))
                ++pings;
        });

    const auto status = Run(
        "preset_dashboard_ping", [&] { dash.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Ping");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(pings, 1);
}

// ── LogConsole: typing a substring into the filter narrows the shown rows ────
TEST_F(PresetInteractionTest, LogConsole_FilterInput_NarrowsRows) {
    unigui::presets::LogConsole log("pi_log");
    log.Append(unigui::presets::LogConsole::Level::Info, "alpha one");
    log.Append(unigui::presets::LogConsole::Level::Info, "beta two");
    log.Append(unigui::presets::LogConsole::Level::Info, "alpha three");
    EXPECT_EQ(log.FilteredSize(), 3u);

    const auto status = Run(
        "preset_log_filter", [&] { log.Render(); },
        [](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemInputValue("**/##filter", "alpha");
        });

    EXPECT_EQ(status, ImGuiTestStatus_Success);
    EXPECT_EQ(log.GetFilter(), "alpha");
    EXPECT_EQ(log.FilteredSize(), 2u);
}
