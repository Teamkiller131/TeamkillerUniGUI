#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {
class Dialog : public Widget {
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
private:
    std::string title_, message_, okText_ = "OK", cancelText_;
    bool open_ = false, okClicked_ = false;
    std::function<void()> onOk_, onCancel_;
};
}
