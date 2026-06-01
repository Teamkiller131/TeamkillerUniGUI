#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>

namespace unigui {

/// Password input with visibility toggle and strength indicator.
class PasswordInput : public ValueWidget<std::string> {
public:
    PasswordInput(std::string name, std::string label, std::string value = "");

    void Render() override;

    void SetValue(std::string val);

    void SetShowStrength(bool on) { showStrength_ = on; }
    /// Returns 0-4: 0=empty, 1=weak, 2=fair, 3=good, 4=strong
    int GetStrengthScore() const;

private:
    int CalcStrength(const std::string& pw) const;

    std::string label_;
    bool showPassword_ = false;
    bool showStrength_ = true;
    char buf_[256] = {};
};

} // namespace unigui
