#include <unigui/widgets/passwordinput.h>
#include <imgui.h>
#include <algorithm>
#include <cctype>
#include <cstring>

namespace unigui {

PasswordInput::PasswordInput(std::string name, std::string label, std::string value)
    : ValueWidget<std::string>(std::move(name), std::move(value)), label_(std::move(label)) {
    SetValue(value_);
}

void PasswordInput::SetValue(std::string val) {
    value_ = std::move(val);
    size_t n = std::min(value_.size(), sizeof(buf_) - 1);
    std::copy_n(value_.data(), n, buf_); buf_[n] = 0;
}

int PasswordInput::CalcStrength(const std::string& pw) const {
    if (pw.empty()) return 0;
    int score = 1;
    if (pw.size() >= 8) score++;
    if (pw.size() >= 12) score++;
    bool hasUpper = false, hasLower = false, hasDigit = false, hasSpecial = false;
    for (char c : pw) {
        if (isupper(c)) hasUpper = true;
        else if (islower(c)) hasLower = true;
        else if (isdigit(c)) hasDigit = true;
        else hasSpecial = true;
    }
    int types = hasUpper + hasLower + hasDigit + hasSpecial;
    if (types >= 3) score++;
    if (types >= 4) score++;
    return std::min(score, 4);
}

int PasswordInput::GetStrengthScore() const { return CalcStrength(value_); }

void PasswordInput::Render() {
    if (!IsVisible()) return;
    ImGui::PushID(GetName().c_str());
    ImGuiInputTextFlags flags = showPassword_ ? 0 : ImGuiInputTextFlags_Password;
    ImGui::SetNextItemWidth(-1);
    std::string oldValue = value_;
    if (ImGui::InputText(label_.c_str(), buf_, sizeof(buf_), flags)) {
        value_ = buf_;
        NotifyChange(oldValue);
    }
    ImGui::SameLine();
    if (ImGui::Button(showPassword_ ? "Hide" : "Show")) showPassword_ = !showPassword_;

    if (showStrength_ && !value_.empty()) {
        int s = CalcStrength(value_);
        const char* labels[] = {"", "Weak", "Fair", "Good", "Strong"};
        ImVec4 colors[] = {{1,0,0,1}, {1,0.5f,0,1}, {1,1,0,1}, {0,1,0,1}};
        if (s > 0) {
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text, colors[s-1]);
            ImGui::TextUnformatted(labels[s]);
            ImGui::PopStyleColor();
        }
    }
    ImGui::PopID();
}

} // namespace unigui
