// preset_demo — a complete, decent-looking application assembled ONLY from the UI
// preset scaffolds (unigui::presets). This is the module's pitch made runnable: an
// app shell with four pages — dashboard, data browser, settings, live log — in
// roughly sixty lines of application code. Headless-friendly: --frames N renders N
// frames and exits (the CI smoke idiom shared by every example).

#include <unigui/unigui.h>

#include <cstdlib>
#include <string>
#include <string_view>

using namespace unigui;

// App state the settings page binds to (getter/setter pairs — no config dep needed).
static bool g_notifications = true;
static int g_refreshMs = 250;
static int g_themeIdx = 0;
static std::string g_username = "trader1";

// The presets — identity objects, constructed once and configured on first use.
static presets::LogConsole g_logs("logs");
static presets::Dashboard g_dash("dash");
static presets::MasterDetail g_data("data");
static presets::SettingsPage g_settings("settings");
static presets::AppShell g_shell("shell", "Preset Demo");
static presets::LoginPage g_login("login");
static presets::WizardFlow g_setup("setup");
static bool g_signedIn = false;     // the login gate flips this on submit
static bool g_setup_agreed = false; // the setup wizard's step-2 gate

static void BuildOnce() {
    g_dash.AddMetric("Throughput", [] { return std::to_string(1200 + rand() % 100) + " msg/s"; })
        .AddMetric(
            "P&L", [] { return "+$12,408"; }, [] { return 0.8; })
        .AddCard("Notes", [] {
            ImGui::TextWrapped("Every card on this page is one AddMetric/AddCard call. "
                               "The grid re-wraps as the window resizes.");
        });

    g_data.WithItems({"EURUSD", "XAUUSD", "BTCUSDT", "AAPL", "NVDA"})
        .WithDetail([](int i) {
            ImGui::Text("Instrument #%d", i);
            ImGui::Separator();
            ImGui::TextWrapped("Detail pane: render anything here — charts, blotters, forms.");
        })
        .WithOnSelect([](int i) {
            g_logs.Append(presets::LogConsole::Level::Info,
                          "Selected instrument " + std::to_string(i));
        });

    g_settings.AddSection("General")
        .AddToggle(
            "Notifications", [] { return g_notifications; }, [](bool v) { g_notifications = v; })
        .AddInt(
            "Refresh (ms)", [] { return g_refreshMs; }, [](int v) { g_refreshMs = v; }, 50, 2000)
        .AddText(
            "Username", [] { return g_username; }, [](const std::string& v) { g_username = v; })
        .AddSection("Appearance")
        .AddCombo(
            "Theme", {"Dark", "Light"}, [] { return g_themeIdx; }, [](int v) { g_themeIdx = v; })
        .AddAction("Reset to defaults", [] {
            g_notifications = true;
            g_refreshMs = 250;
            g_themeIdx = 0;
            g_username = "trader1";
            g_logs.Append(presets::LogConsole::Level::Warn, "Settings reset to defaults");
        });

    // A login gate: submitting any non-empty username signs in and reveals the shell.
    g_login.WithTitle("Sign in to Preset Demo")
        .WithOnSubmit([](const std::string& user, const std::string&, bool) {
            if (user.empty()) {
                g_login.SetStatus("Enter a username", /*isError=*/true);
                return;
            }
            g_signedIn = true;
            g_logs.Append(presets::LogConsole::Level::Info, "Signed in as " + user);
        });

    // A first-run setup wizard, shown as a page.
    g_setup.AddStep("Welcome", [] { ImGui::TextWrapped("This wizard has validation gating."); })
        .AddStep(
            "Confirm",
            [] {
                static bool agreed = false;
                ImGui::Checkbox("I agree to the terms", &agreed);
                g_setup_agreed = agreed; // captured by the gate below
            },
            [] { return g_setup_agreed; })
        .AddStep("Done", [] { ImGui::TextUnformatted("All set!"); })
        .WithOnFinish([] { g_logs.Append(presets::LogConsole::Level::Info, "Setup complete"); });

    g_shell
        .WithMenus({{"File", {{"Clear logs", [] { g_logs.Clear(); }}}},
                    {"Help", {{"About", [] { g_shell.SetStatus("UniGUI preset demo"); }}}}})
        .WithCommandPalette() // Ctrl+P out of the box; pages auto-register as "Go to …"
        .AddCommand("logs.clear", "Clear logs", [] { g_logs.Clear(); })
        .AddPage("Dashboard", [] { g_dash.Render(); })
        .AddPage("Data", [] { g_data.Render(); })
        .AddPage("Setup", [] { g_setup.Render(); })
        .AddPage("Settings", [] { g_settings.Render(); })
        .AddPage("Logs", [] { g_logs.Render(); })
        .WithOnPageChange([](int) { g_shell.SetStatus("Ready"); });
}

static void Draw() {
    static bool built = false;
    if (!built) {
        BuildOnce();
        built = true;
    }
    static int frame = 0;
    ++frame;

    // Login gate: the LoginPage owns the screen until a username is submitted,
    // then the full AppShell takes over. In a headless --frames run there's no
    // user to type, so auto-fill + submit after a few frames so the smoke still
    // exercises the shell (and the demo self-drives when you just watch it).
    if (!g_signedIn) {
        if (frame == 3) {
            g_login.SetUsername("trader1");
            g_login.Submit();
        }
        ImGui::SetNextWindowPos(ImGui::GetMainViewport()->WorkPos);
        ImGui::SetNextWindowSize(ImGui::GetMainViewport()->WorkSize);
        ImGui::Begin("##login_host", nullptr,
                     ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus);
        g_login.Render();
        ImGui::End();
        return;
    }

    // A heartbeat so the log page is alive.
    if (frame % 120 == 0)
        g_logs.Append(presets::LogConsole::Level::Debug,
                      "heartbeat frame " + std::to_string(frame));

    g_shell.Render();
}

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        std::string_view a = argv[i];
        if (a == "--frames" && i + 1 < argc)
            max_frames = std::atoi(argv[++i]);
    }
    AppConfig cfg;
    cfg.title = "UniGUI Preset Demo";
    return RunApp(cfg, [] { Draw(); }, max_frames);
}
