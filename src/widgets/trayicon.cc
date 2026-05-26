#include <unigui/widgets/trayicon.h>
#include <unigui/core/log.h>
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#define WM_TRAYICON (WM_USER + 1)
static LRESULT CALLBACK TrayWndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_TRAYICON && lp == WM_RBUTTONUP) {
        // Right-click: show context menu
        auto* self = (unigui::TrayIcon*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        if (self) { /* menu handled by caller */ }
    }
    return DefWindowProc(hwnd, msg, wp, lp);
}
#endif

namespace unigui {

TrayIcon::TrayIcon(std::string name, std::string title)
    : Widget(std::move(name)), title_(std::move(title)) {
#ifdef _WIN32
    CreateWin32();
#endif
}

TrayIcon::~TrayIcon() { Hide(); }

void TrayIcon::SetMenu(std::vector<TrayMenuItem> items) { menu_ = std::move(items); }

bool TrayIcon::Show() {
    if (visible_) return true;
#ifdef _WIN32
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND)hwnd_;
    nid.uID = 1;
    nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    nid.uCallbackMessage = WM_TRAYICON;
    nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wcscpy_s(nid.szTip, 128, L"UniGUI");
    if (!Shell_NotifyIconW(NIM_ADD, &nid)) {
        UNIGUI_LOG_WARN("TrayIcon: Shell_NotifyIcon(NIM_ADD) failed");
        return false;
    }
    visible_ = true;
    UNIGUI_LOG_INFO("TrayIcon: added to system tray");
    return true;
#else
    UNIGUI_LOG_WARN("TrayIcon: not supported on this platform");
    return false;
#endif
}

void TrayIcon::Hide() {
    if (!visible_) return;
#ifdef _WIN32
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND)hwnd_;
    nid.uID = 1;
    Shell_NotifyIconW(NIM_DELETE, &nid);
    DestroyWin32();
#endif
    visible_ = false;
}

void TrayIcon::ShowNotification(std::string title, std::string msg) {
#ifdef _WIN32
    NOTIFYICONDATAW nid = {};
    nid.cbSize = sizeof(nid);
    nid.hWnd = (HWND)hwnd_;
    nid.uID = 1;
    nid.uFlags = NIF_INFO;
    nid.dwInfoFlags = NIIF_INFO;
    wcscpy_s(nid.szInfoTitle, 64, L"UniGUI");
    wcscpy_s(nid.szInfo, 256, L"Notification");
    Shell_NotifyIconW(NIM_MODIFY, &nid);
#endif
}

#ifdef _WIN32
void TrayIcon::CreateWin32() {
    HINSTANCE hi = GetModuleHandle(nullptr);
    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(wc);
    wc.lpfnWndProc = TrayWndProc;
    wc.hInstance = hi;
    wc.lpszClassName = L"UniGUITrayClass";
    RegisterClassExW(&wc);
    hwnd_ = CreateWindowExW(0, L"UniGUITrayClass", L"", 0, 0, 0, 0, 0, nullptr, nullptr, hi, nullptr);
    SetWindowLongPtr((HWND)hwnd_, GWLP_USERDATA, (LONG_PTR)this);
}
void TrayIcon::DestroyWin32() {
    if (hwnd_) { DestroyWindow((HWND)hwnd_); hwnd_ = nullptr; }
}
#endif

} // namespace unigui
