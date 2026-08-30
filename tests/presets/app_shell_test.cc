#include <unigui/core/accessibility.h>
#include <unigui/presets/app_shell.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

class AppShellTest : public ::testing::Test {
protected:
    void SetUp() override {
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

// ── Defaults: decent with nothing configured beyond the ctor ────────────────
TEST_F(AppShellTest, Defaults) {
    unigui::presets::AppShell shell("shell", "My App");
    EXPECT_EQ(shell.GetTitle(), "My App");
    EXPECT_EQ(shell.GetPageCount(), 0);
    EXPECT_EQ(shell.GetActivePage(), -1); // no pages yet
    EXPECT_FLOAT_EQ(shell.GetSidebarWidth(), 200.f);
    EXPECT_EQ(shell.GetStatus(), "Ready");
}

// ── Fluent chaining preserves the derived type and applies every setter ─────
TEST_F(AppShellTest, FluentChaining_AppliesConfiguration) {
    unigui::presets::AppShell shell("shell", "My App");
    unigui::presets::AppShell& chained =
        shell.WithMenus({{"File", {{"Quit", nullptr}}}})
            .WithToolbar([] {})
            .AddPage("Home", [] {})
            .AddPage("G", "Settings", [] {})
            .WithSidebarWidth(240.f)
            .WithStatus("Loaded")
            .WithOnPageChange([](int) {})
            .WithTooltip("shell tooltip"); // base helper stays AppShell&
    EXPECT_EQ(&chained, &shell);
    EXPECT_EQ(shell.GetPageCount(), 2);
    EXPECT_EQ(shell.GetActivePage(), 0); // first AddPage activates page 0
    EXPECT_FLOAT_EQ(shell.GetSidebarWidth(), 240.f);
    EXPECT_EQ(shell.GetStatus(), "Loaded");
}

// ── Status bar: live updates through SetStatus ──────────────────────────────
TEST_F(AppShellTest, SetStatus_UpdatesLiveText) {
    unigui::presets::AppShell shell("shell", "My App");
    shell.WithStatus("Seeded");
    EXPECT_EQ(shell.GetStatus(), "Seeded");
    shell.SetStatus("3 items saved");
    EXPECT_EQ(shell.GetStatus(), "3 items saved");
}

// ── SetActivePage clamps and no-ops on an empty shell ───────────────────────
TEST_F(AppShellTest, SetActivePage_Clamps) {
    unigui::presets::AppShell shell("shell", "My App");
    shell.SetActivePage(5); // empty shell: stays -1, no crash
    EXPECT_EQ(shell.GetActivePage(), -1);

    shell.AddPage("A", [] {}).AddPage("B", [] {}).AddPage("C", [] {});
    shell.SetActivePage(99);
    EXPECT_EQ(shell.GetActivePage(), 2); // clamped to last
    shell.SetActivePage(-7);
    EXPECT_EQ(shell.GetActivePage(), 0); // clamped to first
    shell.SetActivePage(1);
    EXPECT_EQ(shell.GetActivePage(), 1);
}

// ── Page-change callback fires with the new index, only on real changes ─────
TEST_F(AppShellTest, OnPageChange_FiresOnChangeOnly) {
    unigui::presets::AppShell shell("shell", "My App");
    std::vector<int> seen;
    shell.AddPage("Home", [] {}).AddPage("Settings", [] {}).WithOnPageChange([&](int idx) {
        seen.push_back(idx);
    });
    shell.SetActivePage(1);
    shell.SetActivePage(1); // same page: no second callback
    shell.SetActivePage(0);
    EXPECT_EQ(seen, (std::vector<int>{1, 0}));
}

// ── A11y: a programmatic page switch announces "<label> page" ───────────────
TEST_F(AppShellTest, SetActivePage_AnnouncesPage) {
    unigui::a11y::SetEnabled(true);
    unigui::a11y::BeginFrame();
    unigui::a11y::DrainAnnouncements();

    unigui::presets::AppShell shell("shell", "My App");
    shell.AddPage("Home", [] {}).AddPage("Settings", [] {});
    shell.SetActivePage(1);

    bool announced = false;
    for (const auto& a : unigui::a11y::DrainAnnouncements())
        if (a.message == "Settings page")
            announced = true;
    EXPECT_TRUE(announced);
    unigui::a11y::SetEnabled(false);
}

// ── Render draws only the active page's content ──────────────────────────────
TEST_F(AppShellTest, Render_InvokesActivePageOnly) {
    unigui::presets::AppShell shell("shell", "My App");
    bool home = false, settings = false;
    shell.AddPage("Home", [&] { home = true; }).AddPage("Settings", [&] { settings = true; });
    shell.SetActivePage(1);
    shell.Render();
    EXPECT_FALSE(home);
    EXPECT_TRUE(settings);
}

// ── Render: empty shell shows the hint instead of crashing ──────────────────
TEST_F(AppShellTest, Render_EmptyShell_DoesNotCrash) {
    unigui::presets::AppShell shell("shell", "My App");
    shell.Render();
    EXPECT_EQ(shell.GetActivePage(), -1);
}

// ── Command palette integration ──────────────────────────────────────────────
TEST_F(AppShellTest, Palette_DisabledByDefault) {
    unigui::presets::AppShell shell("shell", "App");
    EXPECT_FALSE(shell.HasCommandPalette());
    EXPECT_FALSE(shell.IsCommandPaletteOpen());
    shell.OpenCommandPalette(); // no-op while disabled
    EXPECT_FALSE(shell.IsCommandPaletteOpen());
}

TEST_F(AppShellTest, Palette_EnableAndOpen) {
    unigui::presets::AppShell shell("shell", "App");
    shell.WithCommandPalette();
    EXPECT_TRUE(shell.HasCommandPalette());
    shell.OpenCommandPalette();
    EXPECT_TRUE(shell.IsCommandPaletteOpen());
}

TEST_F(AppShellTest, Palette_AddCommand_ImpliesEnable) {
    unigui::presets::AppShell shell("shell", "App");
    shell.AddCommand("app.save", "Save workspace", [] {});
    EXPECT_TRUE(shell.HasCommandPalette());
}

TEST_F(AppShellTest, Palette_RenderWithOpenPalette_DoesNotCrash) {
    unigui::presets::AppShell shell("shell", "App");
    shell.WithCommandPalette()
        .AddPage("Home", [] { ImGui::TextUnformatted("home"); })
        .AddPage("Logs", [] { ImGui::TextUnformatted("logs"); });
    shell.AddCommand("app.quit", "Quit", [] {});
    shell.OpenCommandPalette();
    shell.Render();
    EXPECT_TRUE(shell.IsCommandPaletteOpen());
}

// ── Render: fully-loaded shell does not crash ────────────────────────────────
TEST_F(AppShellTest, Render_FullChrome_DoesNotCrash) {
    unigui::presets::AppShell shell("shell", "My App");
    bool toolbarDrawn = false;
    shell
        .WithMenus(
            {{"File", {{"New", nullptr}, {"Quit", nullptr}}}, {"Help", {{"About", nullptr}}}})
        .WithToolbar([&] {
            toolbarDrawn = true;
            ImGui::SmallButton("Run");
        })
        .AddPage("H", "Home", [] { ImGui::TextUnformatted("home"); })
        .AddPage("Settings", [] { ImGui::TextUnformatted("settings"); })
        .WithSidebarWidth(160.f)
        .WithStatus("OK");
    shell.Render();
    EXPECT_TRUE(toolbarDrawn);
}
