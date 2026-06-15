#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <functional>
#include <string>

namespace unigui {

/// ConfirmDialog — modal confirmation popup with optional danger styling.
/// Uses PushID/PopID for ID safety.
class ConfirmDialog : public Widget {
public:
    explicit ConfirmDialog(std::string name);

    void Render() override;

    void Open();
    /// Open carrying an on-confirm callback fired (once) when the user confirms,
    /// so callers stop maintaining a parallel pending-action id + OnTick switch.
    void Open(std::function<void()> onConfirm);
    bool WasConfirmed() const;
    /// Persistent on-confirm callback (alternative to the Open(cb) overload).
    void SetOnConfirm(std::function<void()> cb) { onConfirm_ = std::move(cb); }

    void SetTitle(std::string title) { title_ = std::move(title); }
    void SetMessage(std::string msg) { message_ = std::move(msg); }
    void SetIcon(std::string icon) { icon_ = std::move(icon); }
    void SetConfirmLabel(std::string lbl) { confirmLabel_ = std::move(lbl); }
    void SetCancelLabel(std::string lbl) { cancelLabel_ = std::move(lbl); }
    void SetConfirmColor(ImU32 color) { confirmColor_ = color; }
    void SetDangerStyle(bool on) { dangerStyle_ = on; }

    const std::string& GetTitle() const { return title_; }
    const std::string& GetMessage() const { return message_; }
    bool IsOpen() const { return open_; }

private:
    bool open_ = false;
    bool confirmed_ = false;
    bool justOpened_ = false;
    std::string title_;
    std::string message_;
    std::string icon_ = "⚠️";
    std::string confirmLabel_ = "Confirm";
    std::string cancelLabel_ = "Cancel";
    ImU32 confirmColor_ = 0xff4560e9; // RGBA (red #e94560)
    bool dangerStyle_ = false;
    std::function<void()> onConfirm_;
};

} // namespace unigui
