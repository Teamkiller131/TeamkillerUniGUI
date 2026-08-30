#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {
class Dialog : public FluentWidget<Dialog> {
public:
    Dialog(std::string name, std::string title, std::string message);
    void Render() override;
    void Open();
    void Close();
    bool IsOpen() const;
    void SetButtons(std::string okText, std::string cancelText = "");
    void SetOnOk(std::function<void()> callback);
    void SetOnCancel(std::function<void()> callback);
    bool WasOkClicked() const;

    // ── Fluent (chainable) helpers — return Dialog& via CRTP base ──────────
    Dialog& WithButtons(std::string okText, std::string cancelText = "") {
        SetButtons(std::move(okText), std::move(cancelText));
        return *this;
    }
    Dialog& WithOnOk(std::function<void()> callback) {
        SetOnOk(std::move(callback));
        return *this;
    }
    Dialog& WithOnCancel(std::function<void()> callback) {
        SetOnCancel(std::move(callback));
        return *this;
    }

private:
    std::string title_, message_, okText_ = "OK", cancelText_;
    bool open_ = false, okClicked_ = false;
    std::function<void()> onOk_, onCancel_;
};
} // namespace unigui
