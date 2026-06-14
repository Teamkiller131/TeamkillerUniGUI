#include <unigui/core/log.h>
#include <unigui/widgets/trayicon.h>
#ifdef _WIN32
#include <windows.h>

#include <shellapi.h>
#define WM_TRAYICON (WM_USER + 1)
static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_TRAYICON) {
        auto* self = (unigui::TrayIcon*) GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (self && lp == WM_RBUTTONUP)
            self->ShowContextMenu();
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}
#endif
namespace unigui {
TrayIcon::TrayIcon(std::string name, std::string title, int iconId)
        : Widget(std::move(name))
        , title_(std::move(title))
        , iconId_(iconId) {
#ifdef _WIN32
    HINSTANCE hi = GetModuleHandle(nullptr);
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = hi;
    wc.lpszClassName = L"UniGUITray";
    RegisterClassExW(&wc);
    hwnd_ = CreateWindowExW(0, L"UniGUITray", L"", 0, 0, 0, 0, 0, nullptr, nullptr, hi, nullptr);
    SetWindowLongPtr((HWND) hwnd_, GWLP_USERDATA, (LONG_PTR) this);
#endif
}
TrayIcon::~TrayIcon() {
    Hide();
}
void TrayIcon::SetMenu(std::vector<TrayMenuItem> items) {
    menu_ = std::move(items);
}
bool TrayIcon::Show() {
    if (visible_)
        return true;
#ifdef _WIN32
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND) hwnd_;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = iconId_ ? LoadIcon(GetModuleHandle(nullptr), MAKEINTRESOURCE(iconId_))
                        : LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nid.szTip, 128, L"UniGUI");
    if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
        UNIGUI_LOG_WARN("TrayIcon NIM_ADD failed");
        return false;
    }
    visible_ = true;
    return true;
#else
    return false;
#endif
}
void TrayIcon::Hide() {
    if (!visible_)
        return;
#ifdef _WIN32
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND) hwnd_;
    nid.uID = 1;
    Shell_NotifyIconW(NIM_DELETE, &nid);
    if (hwnd_) {
        DestroyWindow((HWND) hwnd_);
        hwnd_ = nullptr;
    }
#endif
    visible_ = false;
}
void TrayIcon::UpdateTooltip(std::string title) {
    title_ = std::move(title);
#ifdef _WIN32
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND) hwnd_;
    nid.uID = 1;
    nid.uFlags = NIF_TIP;
    int tl = (int) title_.size();
    if (tl > 127)
        tl = 127;
    MultiByteToWideChar(CP_UTF8, 0, title_.c_str(), tl, nid.szTip, 128);
    nid.szTip[tl] = 0;
    Shell_NotifyIconW(NIM_MODIFY, &nid);
#endif
}
void TrayIcon::ShowNotification([[maybe_unused]] std::string title, [[maybe_unused]] std::string msg,
                                [[maybe_unused]] NotifyType type) {
#ifdef _WIN32
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND) hwnd_;
    nid.uID = 1;
    nid.uFlags = NIF_INFO;
    switch (type) {
    case NotifyType::Warning:
        nid.dwInfoFlags = NIIF_WARNING;
        break;
    case NotifyType::Error:
        nid.dwInfoFlags = NIIF_ERROR;
        break;
    default:
        nid.dwInfoFlags = NIIF_INFO;
        break;
    }
    int tl = (int) title.size();
    if (tl > 63)
        tl = 63;
    int ml = (int) msg.size();
    if (ml > 255)
        ml = 255;
    MultiByteToWideChar(CP_UTF8, 0, title.c_str(), tl, nid.szInfoTitle, 64);
    nid.szInfoTitle[tl] = 0;
    MultiByteToWideChar(CP_UTF8, 0, msg.c_str(), ml, nid.szInfo, 256);
    nid.szInfo[ml] = 0;
    Shell_NotifyIconW(NIM_MODIFY, &nid);
#endif
}
#ifdef _WIN32
void TrayIcon::BuildContextMenu(HMENU hMenu, std::vector<TrayMenuItem>& items, int& idCounter) {
    for (auto& item : items) {
        if (item.isSeparator) {
            AppendMenuA(hMenu, MF_SEPARATOR, 0, nullptr);
            continue;
        }
        if (!item.children.empty()) {
            HMENU hSub = CreatePopupMenu();
            int subId = idCounter;
            BuildContextMenu(hSub, item.children, subId);
            AppendMenuA(hMenu, MF_STRING | MF_POPUP, (UINT_PTR) hSub, item.label.c_str());
        } else {
            AppendMenuA(hMenu, MF_STRING, ++idCounter, item.label.c_str());
        }
    }
}
void TrayIcon::ShowContextMenu() {
    if (menu_.empty())
        return;
    HMENU hMenu = CreatePopupMenu();
    int idCounter = 0;
    BuildContextMenu(hMenu, menu_, idCounter);
    POINT pt;
    GetCursorPos(&pt);
    SetForegroundWindow((HWND) hwnd_);
    int cmd =
        TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, (HWND) hwnd_, nullptr);
    if (cmd > 0 && cmd <= idCounter) {
        // Flattened search: find item by ID in recursive structure
        std::function<TrayMenuItem*(std::vector<TrayMenuItem>&)> find =
            [&](auto& its) -> TrayMenuItem* {
            int curId = 0;
            for (auto& it : its) {
                if (it.isSeparator)
                    continue;
                if (!it.children.empty()) {
                    auto* found = find(it.children);
                    if (found)
                        return found;
                }
                curId++;
                if (curId == cmd)
                    return &it;
            }
            return nullptr;
        };
        int curId = 0;
        for (auto& it : menu_) {
            if (it.isSeparator)
                continue;
            curId++;
            if (curId == cmd) {
                if (it.action)
                    it.action();
                if (onExit_ && it.label == "Exit")
                    onExit_();
                break;
            }
        }
    }
    DestroyMenu(hMenu);
}
#endif
} // namespace unigui
