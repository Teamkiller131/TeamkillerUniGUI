#include <unigui/presets/app_shell.h>
#include <unigui/theme/color_tokens.h>

#include <imgui.h>

#include <algorithm>

namespace unigui::presets {

AppShell::AppShell(std::string name, std::string title)
        : FluentWidget<AppShell>(std::move(name))
        , title_(std::move(title))
        , statusBar_(GetName() + ".status", "Ready") {}

// ── Chrome slots ─────────────────────────────────────────────────────────────

AppShell& AppShell::WithMenus(std::vector<MenuDef> menus) {
    menus_ = std::move(menus);
    return *this;
}

AppShell& AppShell::WithToolbar(std::function<void()> draw) {
    toolbar_ = std::move(draw);
    return *this;
}

AppShell& AppShell::AddPage(std::string label, std::function<void()> content) {
    return AddPage("", std::move(label), std::move(content));
}

AppShell& AppShell::AddPage(std::string icon, std::string label, std::function<void()> content) {
    pages_.push_back(Page{std::move(icon), std::move(label), std::move(content)});
    if (activePage_ < 0)
        activePage_ = 0; // first page becomes active (no announcement: initial state)
    return *this;
}

AppShell& AppShell::WithSidebarWidth(float w) {
    sidebarWidth_ = w;
    return *this;
}

AppShell& AppShell::WithStatus(std::string text) {
    statusBar_.SetText(std::move(text));
    return *this;
}

AppShell& AppShell::WithOnPageChange(std::function<void(int)> fn) {
    onPageChange_ = std::move(fn);
    return *this;
}

// ── Live state ───────────────────────────────────────────────────────────────

void AppShell::SetStatus(std::string text) {
    statusBar_.SetText(std::move(text));
}

const std::string& AppShell::GetStatus() const {
    return statusBar_.GetText();
}

int AppShell::GetActivePage() const {
    return activePage_;
}

void AppShell::SetActivePage(int index) {
    if (pages_.empty())
        return;
    index = std::clamp(index, 0, (int) pages_.size() - 1);
    if (index == activePage_)
        return;
    activePage_ = index;
    a11y::Announce(pages_[(size_t) activePage_].label + " page");
    if (onPageChange_)
        onPageChange_(activePage_);
}

int AppShell::GetPageCount() const {
    return (int) pages_.size();
}

float AppShell::GetSidebarWidth() const {
    return sidebarWidth_;
}

const std::string& AppShell::GetTitle() const {
    return title_;
}

// ── Render ───────────────────────────────────────────────────────────────────

void AppShell::Render() {
    if (!IsVisible())
        return;

    // Fullscreen host over the main viewport (WorkPos/WorkSize respect a global
    // main-menu bar). NoDocking + NoBringToFrontOnFocus keep the shell a plain
    // backdrop that coexists with the DockSpaceOverViewport(PassthruCentralNode)
    // the app loop already submits.
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->WorkPos);
    ImGui::SetNextWindowSize(vp->WorkSize);
    ImGui::SetNextWindowViewport(vp->ID);
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                             ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBringToFrontOnFocus |
                             ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoNavFocus |
                             ImGuiWindowFlags_NoDocking;
    if (!menus_.empty())
        flags |= ImGuiWindowFlags_MenuBar;

    ImGui::PushID(GetName().c_str());
    if (ImGui::Begin(("##" + GetName()).c_str(), nullptr, flags)) {
        ReportAccessible(a11y::Role::Window, ImGui::IsItemFocused(), title_);
        RenderMenuBar();
        if (toolbar_) {
            toolbar_();
            ImGui::Separator();
        }

        // Reserve one text line (+ the Separator the StatusBar draws) at the
        // bottom so the status bar stays pinned under the body row.
        const ImGuiStyle& style = ImGui::GetStyle();
        const float statusH = ImGui::GetTextLineHeightWithSpacing() + style.ItemSpacing.y + 2.f;
        const float bodyH = std::max(ImGui::GetContentRegionAvail().y - statusH, 1.f);

        RenderSidebar(bodyH);
        ImGui::SameLine();
        RenderContent(bodyH);
        statusBar_.Render();
    }
    ImGui::End();
    ImGui::PopID();
}

void AppShell::RenderMenuBar() {
    // The shell hosts its menus in its own window bar (the MenuBar widget
    // targets the global BeginMainMenuBar, which would escape the shell).
    if (menus_.empty() || !ImGui::BeginMenuBar())
        return;
    ReportAccessible(a11y::Role::Menu, ImGui::IsItemFocused(), "");
    for (auto& menu : menus_) {
        if (ImGui::BeginMenu(menu.label.c_str())) {
            for (auto& item : menu.items) {
                if (ImGui::MenuItem(item.label.c_str()) && item.action)
                    item.action();
            }
            ImGui::EndMenu();
        }
    }
    ImGui::EndMenuBar();
}

void AppShell::RenderSidebar(float height) {
    const float width = std::max(sidebarWidth_, 1.f);
    ImGui::BeginChild("##sidebar", ImVec2(width, height), ImGuiChildFlags_Borders);
    // Tint the active entry with the theme accent so the highlight follows the
    // active theme instead of the stock Header grey.
    const ImVec4 accent = theme::GetSemanticColor(theme::Semantic::Accent);
    ImGui::PushStyleColor(ImGuiCol_Header, theme::WithAlpha(accent, 0.35f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, theme::WithAlpha(accent, 0.25f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, theme::WithAlpha(accent, 0.45f));
    for (int i = 0; i < (int) pages_.size(); ++i) {
        const Page& page = pages_[(size_t) i];
        const std::string entry = page.icon.empty() ? page.label : page.icon + "  " + page.label;
        if (ImGui::Selectable(entry.c_str(), i == activePage_))
            SetActivePage(i); // announces + fires the page-change callback
        ReportAccessible(a11y::Role::ListItem, ImGui::IsItemFocused(), page.label);
    }
    ImGui::PopStyleColor(3);
    ImGui::EndChild();
}

void AppShell::RenderContent(float height) {
    ImGui::BeginChild("##content", ImVec2(0, height));
    if (pages_.empty()) {
        // Friendly empty state instead of a blank (or crashing) shell.
        ImGui::TextDisabled("%s", title_.c_str());
        ImGui::TextDisabled("No pages yet — add one with AddPage(label, content).");
    } else if (activePage_ >= 0 && activePage_ < (int) pages_.size() &&
               pages_[(size_t) activePage_].content) {
        pages_[(size_t) activePage_].content();
    }
    ImGui::EndChild();
}

} // namespace unigui::presets
