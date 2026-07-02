#pragma once
#include <unigui/widgets/menubar.h>   // MenuDef / MenuItem
#include <unigui/widgets/statusbar.h> // composed status-bar footer
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui::presets {

// ─────────────────────────────────────────────────────────────────────────────
// AppShell — flagship application-chrome preset.
//
// A prefab desktop-app scaffold: one fullscreen host window over the main
// viewport containing an optional menu bar, an optional toolbar row, a sidebar
// of navigable pages, the active page's content, and a status bar pinned at
// the bottom. Everything is a slot — a decent app UI in ~30 lines:
//
//     unigui::presets::AppShell shell("shell", "My App");
//     shell.WithMenus({{"File", {{"Quit", quit}}}})
//          .AddPage("Home",     [] { ImGui::Text("home"); })
//          .AddPage("Settings", [] { ImGui::Text("settings"); })
//          .WithStatus("Ready");
//     // per frame: shell.Render();
//
// The shell renders sensibly with nothing configured beyond the constructor
// (an empty shell shows a friendly hint instead of crashing) and coexists with
// the DockSpaceOverViewport the app loop submits.
// ─────────────────────────────────────────────────────────────────────────────
class AppShell : public FluentWidget<AppShell> {
public:
    /// `name` is the unique widget id; `title` is the app title (used as the
    /// accessible window name and the empty-shell placeholder heading).
    AppShell(std::string name, std::string title);

    // ── Chrome slots (chainable) ────────────────────────────────────────
    /// Show a menu bar with these menus (reuses MenuDef/MenuItem from MenuBar).
    AppShell& WithMenus(std::vector<MenuDef> menus);
    /// Draw a toolbar row under the menu bar (any immediate-mode content).
    AppShell& WithToolbar(std::function<void()> draw);
    /// Append a sidebar entry + its page content callback.
    AppShell& AddPage(std::string label, std::function<void()> content);
    /// Append a sidebar entry with a leading icon (e.g. a font-icon glyph).
    AppShell& AddPage(std::string icon, std::string label, std::function<void()> content);
    /// Sidebar width in pixels (default 200).
    AppShell& WithSidebarWidth(float w);
    /// Seed the status-bar text (default "Ready").
    AppShell& WithStatus(std::string text);
    /// Called with the new page index whenever the active page changes.
    AppShell& WithOnPageChange(std::function<void(int)> fn);

    // ── Live state ──────────────────────────────────────────────────────
    /// Update the status-bar text (live, e.g. from an async job).
    void SetStatus(std::string text);
    const std::string& GetStatus() const;
    /// Active page index, or -1 while the shell has no pages.
    int GetActivePage() const;
    /// Switch pages programmatically. Clamps to [0, pages-1]; announces the
    /// page to assistive tech and fires the on-page-change callback. A no-op
    /// on an empty shell or when `index` resolves to the current page.
    void SetActivePage(int index);
    int GetPageCount() const;
    float GetSidebarWidth() const;
    const std::string& GetTitle() const;

    void Render() override;

private:
    struct Page {
        std::string icon;
        std::string label;
        std::function<void()> content;
    };

    void RenderMenuBar();
    void RenderSidebar(float height);
    void RenderContent(float height);

    std::string title_;
    std::vector<MenuDef> menus_;
    std::function<void()> toolbar_;
    std::vector<Page> pages_;
    float sidebarWidth_ = 200.f;
    int activePage_ = -1;
    std::function<void(int)> onPageChange_;
    StatusBar statusBar_;
};

} // namespace unigui::presets
