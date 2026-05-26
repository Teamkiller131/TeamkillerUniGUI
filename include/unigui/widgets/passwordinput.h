#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>

namespace unigui {

/// Password input with visibility toggle and strength indicator.
class PasswordInput : public Widget {
public:
    PasswordInput(std::string name, std::string label, std::string value = "");

    void Render() override;

    std::string GetValue() const { return value_; }
    void SetValue(std::string val);

    void SetShowStrength(bool on) { showStrength_ = on; }
    /// Returns 0-4: 0=empty, 1=weak, 2=fair, 3=good, 4=strong
    int GetStrengthScore() const;

private:
    int CalcStrength(const std::string& pw) const;

    std::string label_;
    std::string value_;
    bool showPassword_ = false;
    bool showStrength_ = true;
    char buf_[256] = {};
};

} // namespace unigui
