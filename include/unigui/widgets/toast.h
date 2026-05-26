#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <deque>
#include <chrono>
#include <imgui.h>

namespace unigui {

enum class ToastType { Info, Success, Warning, Error };

struct ToastMessage {
    std::string text;
    ToastType type = ToastType::Info;
    std::chrono::steady_clock::time_point showTime;
    float duration = 3.0f;
};

/// Singleton notification popup system. Call Show() from anywhere.
class Toast : public Widget {
public:
    static Toast& Instance();
    Toast(std::string name = "toast");
    void Render() override;
    void Show(std::string msg, ToastType type = ToastType::Info, float duration = 3.0f);
    static void Info(std::string msg) { Instance().Show(std::move(msg), ToastType::Info); }
    static void Success(std::string msg) { Instance().Show(std::move(msg), ToastType::Success); }
    static void Warn(std::string msg) { Instance().Show(std::move(msg), ToastType::Warning); }
    static void Error(std::string msg) { Instance().Show(std::move(msg), ToastType::Error); }

    /// v1.9: Set anchor position. 0=top-left, 1=top-right, 2=bottom-right(default), 3=bottom-left
    void SetPosition(int anchor, float offsetX=10, float offsetY=10);

private:
    std::deque<ToastMessage> queue_;
    int anchor_ = 2;
    float offX_ = 10, offY_ = 10;
};

} // namespace unigui
