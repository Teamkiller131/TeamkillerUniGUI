#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {
class LineEdit : public Widget {
public:
    LineEdit(std::string name, std::string label, std::string value = "");
    void Render() override;
    std::string GetValue() const;
    void SetValue(std::string value);
    void SetPlaceholder(std::string text);
    void SetValidator(std::function<bool(const std::string&)> fn);
    bool HasError() const;
    void SetPasswordMode(bool on);
    void SetMultiline(bool on);
    void SetReadOnly(bool on);
    void SetMaxLength(int maxLen);
private:
    std::string label_;
    std::string value_;
    std::string placeholder_;
    std::function<bool(const std::string&)> validator_;
    bool has_error_ = false;
    bool password_ = false;
    bool multiline_ = false;
    bool read_only_ = false;
    int max_length_ = 256;
    char buffer_[1024] = {};
};
}
