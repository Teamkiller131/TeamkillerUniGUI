#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
namespace unigui {
struct TrayMenuItem { std::string label; std::function<void()> action; };
class TrayIcon : public Widget {
public:
    TrayIcon(std::string name, std::string title = "UniGUI");
    ~TrayIcon();
    void Render() override {}
    bool Show();
    void Hide();
    void SetMenu(std::vector<TrayMenuItem> items);
    void ShowNotification(std::string title, std::string msg);
private:
    std::string title_;
    std::vector<TrayMenuItem> menu_;
    bool visible_ = false;
#ifdef _WIN32
    void* nid_ = nullptr;
    void* hwnd_ = nullptr;
    void CreateWin32();
    void DestroyWin32();
#endif
};
} // namespace unigui
