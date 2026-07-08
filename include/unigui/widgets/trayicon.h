#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>
namespace unigui {
struct TrayMenuItem {
    std::string label;
    std::function<void()> action;
    bool isSeparator = false;
    std::vector<TrayMenuItem> children; // submenu
};
enum class NotifyType { Info, Warning, Error };
class TrayIcon : public FluentWidget<TrayIcon> {
public:
    TrayIcon(std::string name, std::string title = "UniGUI", int iconId = 0);
    ~TrayIcon();
    void Render() override {}
    bool Show();
    void Hide();
    void SetMenu(std::vector<TrayMenuItem> items);
    void UpdateTooltip(std::string title);
    void ShowNotification(std::string title, std::string msg, NotifyType type = NotifyType::Info);
    void SetOnExit(std::function<void()> cb) { onExit_ = std::move(cb); }

    // ── Fluent (chainable) helpers — return TrayIcon& via CRTP base ──────────
    TrayIcon& WithMenu(std::vector<TrayMenuItem> items) {
        SetMenu(std::move(items));
        return *this;
    }
    TrayIcon& WithOnExit(std::function<void()> cb) {
        SetOnExit(std::move(cb));
        return *this;
    }
#ifdef _WIN32
    void ShowContextMenu();
#endif
private:
    std::string title_;
    std::vector<TrayMenuItem> menu_;
    std::function<void()> onExit_;
    int iconId_;
    bool visible_ = false;
#ifdef _WIN32
    void* nid_ = nullptr;
    void* hwnd_ = nullptr;
    void CreateWin32();
    void DestroyWin32();
    // hMenu is an HMENU, type-erased to void* so this public header needs no
    // <windows.h> (cast back at the definition in trayicon.cc).
    void BuildContextMenu(void* hMenu, std::vector<TrayMenuItem>& items, int& idCounter);
#endif
};
} // namespace unigui
