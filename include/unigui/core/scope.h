#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// UniGUI RAII Scope Helpers  (namespace unigui)
//
// Header-only move-only guards that pair Dear ImGui's Begin*/Push* calls with
// their matching End*/Pop* on scope exit — eliminating the most common source
// of ImGui bugs (forgotten or mismatched End()/PopID()).
//
//     if (unigui::WindowScope w{"Settings"}) {
//         unigui::IDScope id{"row"};
//         unigui::DisabledScope d{readOnly};
//         // ... widgets ...
//     }   // End()/PopID()/EndDisabled() run automatically, in reverse order.
//
// Mirrors the style of the existing unigui::StyleScope.
// ─────────────────────────────────────────────────────────────────────────────

#include <imgui.h>

#include <utility>

namespace unigui {

// ── WindowScope ──────────────────────────────────────────────────────────────
/// RAII for ImGui::Begin / ImGui::End. End() is always called on destruction
/// (as ImGui requires), regardless of whether the window is collapsed/clipped.
/// `operator bool` / visible() reports whether window contents should be drawn.
class WindowScope {
public:
    explicit WindowScope(const char* name, bool* open = nullptr, ImGuiWindowFlags flags = 0) {
        visible_ = ImGui::Begin(name, open, flags);
    }
    ~WindowScope() {
        if (active_)
            ImGui::End();
    }

    WindowScope(WindowScope&& o) noexcept
            : visible_(o.visible_)
            , active_(o.active_) {
        o.active_ = false;
    }
    WindowScope& operator=(WindowScope&&) = delete;
    WindowScope(const WindowScope&) = delete;
    WindowScope& operator=(const WindowScope&) = delete;

    bool visible() const { return visible_; }
    explicit operator bool() const { return visible_; }

private:
    bool visible_ = false;
    bool active_ = true; // Begin/End must always be balanced
};

// ── ChildScope ───────────────────────────────────────────────────────────────
/// RAII for ImGui::BeginChild / ImGui::EndChild. EndChild() is always called.
class ChildScope {
public:
    explicit ChildScope(const char* id, const ImVec2& size = ImVec2(0, 0),
                        ImGuiChildFlags childFlags = 0, ImGuiWindowFlags windowFlags = 0) {
        visible_ = ImGui::BeginChild(id, size, childFlags, windowFlags);
    }
    explicit ChildScope(ImGuiID id, const ImVec2& size = ImVec2(0, 0),
                        ImGuiChildFlags childFlags = 0, ImGuiWindowFlags windowFlags = 0) {
        visible_ = ImGui::BeginChild(id, size, childFlags, windowFlags);
    }
    ~ChildScope() {
        if (active_)
            ImGui::EndChild();
    }

    ChildScope(ChildScope&& o) noexcept
            : visible_(o.visible_)
            , active_(o.active_) {
        o.active_ = false;
    }
    ChildScope& operator=(ChildScope&&) = delete;
    ChildScope(const ChildScope&) = delete;
    ChildScope& operator=(const ChildScope&) = delete;

    bool visible() const { return visible_; }
    explicit operator bool() const { return visible_; }

private:
    bool visible_ = false;
    bool active_ = true;
};

// ── IDScope ──────────────────────────────────────────────────────────────────
/// RAII for ImGui::PushID / ImGui::PopID.
class IDScope {
public:
    explicit IDScope(const char* id) { ImGui::PushID(id); }
    explicit IDScope(int id) { ImGui::PushID(id); }
    explicit IDScope(const void* id) { ImGui::PushID(id); }
    ~IDScope() {
        if (active_)
            ImGui::PopID();
    }

    IDScope(IDScope&& o) noexcept
            : active_(o.active_) {
        o.active_ = false;
    }
    IDScope& operator=(IDScope&&) = delete;
    IDScope(const IDScope&) = delete;
    IDScope& operator=(const IDScope&) = delete;

private:
    bool active_ = true;
};

// ── DisabledScope ────────────────────────────────────────────────────────────
/// RAII for ImGui::BeginDisabled / ImGui::EndDisabled.
class DisabledScope {
public:
    explicit DisabledScope(bool disabled = true)
            : disabled_(disabled) {
        ImGui::BeginDisabled(disabled_);
    }
    ~DisabledScope() {
        if (active_)
            ImGui::EndDisabled();
    }

    DisabledScope(DisabledScope&& o) noexcept
            : disabled_(o.disabled_)
            , active_(o.active_) {
        o.active_ = false;
    }
    DisabledScope& operator=(DisabledScope&&) = delete;
    DisabledScope(const DisabledScope&) = delete;
    DisabledScope& operator=(const DisabledScope&) = delete;

private:
    bool disabled_ = true;
    bool active_ = true;
};

// ── GroupScope ───────────────────────────────────────────────────────────────
/// RAII for ImGui::BeginGroup / ImGui::EndGroup.
class GroupScope {
public:
    GroupScope() { ImGui::BeginGroup(); }
    ~GroupScope() {
        if (active_)
            ImGui::EndGroup();
    }

    GroupScope(GroupScope&& o) noexcept
            : active_(o.active_) {
        o.active_ = false;
    }
    GroupScope& operator=(GroupScope&&) = delete;
    GroupScope(const GroupScope&) = delete;
    GroupScope& operator=(const GroupScope&) = delete;

private:
    bool active_ = true;
};

// ── TabBarScope ──────────────────────────────────────────────────────────────
/// RAII for ImGui::BeginTabBar / ImGui::EndTabBar. EndTabBar() is called only
/// when BeginTabBar() returned true (per ImGui's API contract).
class TabBarScope {
public:
    explicit TabBarScope(const char* id, ImGuiTabBarFlags flags = 0) {
        open_ = ImGui::BeginTabBar(id, flags);
    }
    ~TabBarScope() {
        if (open_)
            ImGui::EndTabBar();
    }

    TabBarScope(TabBarScope&& o) noexcept
            : open_(o.open_) {
        o.open_ = false;
    }
    TabBarScope& operator=(TabBarScope&&) = delete;
    TabBarScope(const TabBarScope&) = delete;
    TabBarScope& operator=(const TabBarScope&) = delete;

    bool open() const { return open_; }
    explicit operator bool() const { return open_; }

private:
    bool open_ = false;
};

// ── TabItemScope ─────────────────────────────────────────────────────────────
/// RAII for ImGui::BeginTabItem / ImGui::EndTabItem. EndTabItem() is called
/// only when BeginTabItem() returned true.
class TabItemScope {
public:
    explicit TabItemScope(const char* label, bool* open = nullptr, ImGuiTabItemFlags flags = 0) {
        open_ = ImGui::BeginTabItem(label, open, flags);
    }
    ~TabItemScope() {
        if (open_)
            ImGui::EndTabItem();
    }

    TabItemScope(TabItemScope&& o) noexcept
            : open_(o.open_) {
        o.open_ = false;
    }
    TabItemScope& operator=(TabItemScope&&) = delete;
    TabItemScope(const TabItemScope&) = delete;
    TabItemScope& operator=(const TabItemScope&) = delete;

    bool selected() const { return open_; }
    explicit operator bool() const { return open_; }

private:
    bool open_ = false;
};

} // namespace unigui
