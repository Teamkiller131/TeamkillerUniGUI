#pragma once
#include <unigui/widgets/widget_base.h>

#include <chrono>
#include <deque>
#include <string>
namespace unigui {
struct NotificationMsg {
    std::string title, message;
    float duration = 3.0f;
    float elapsed = 0;
};
class Notification : public FluentWidget<Notification> {
public:
    Notification(std::string name);
    void Render() override;
    void Show(std::string title, std::string msg, float duration = 3.0f);
    size_t PendingCount() const;

private:
    std::deque<NotificationMsg> queue_;
};
} // namespace unigui
