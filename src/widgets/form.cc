#include <unigui/widgets/form.h>
#include <imgui.h>
#include <sstream>
#include <regex>

namespace unigui {

Form::Form(std::string name, std::string title)
    : Widget(std::move(name)), title_(std::move(title)) {
}

void Form::AddTextField(std::string name, std::string label, bool required) {
    fields_.push_back({std::move(name), std::move(label), FormField::Type::Text, required, ""});
}

void Form::AddCheckbox(std::string name, std::string label) {
    fields_.push_back({std::move(name), std::move(label), FormField::Type::Checkbox, false, "0"});
}
void Form::AddComboField(std::string name, std::string label, std::vector<std::string>) {
    fields_.push_back({std::move(name), std::move(label), FormField::Type::Combo, false, "0"});
}
void Form::AddSliderField(std::string name, std::string label, float, float) {
    fields_.push_back({std::move(name), std::move(label), FormField::Type::Slider, false, "0"});
}
void Form::AddNumberField(std::string name, std::string label, int, int) {
    fields_.push_back({std::move(name), std::move(label), FormField::Type::Number, false, "0"});
}

std::string Form::GetFieldValue(const std::string& name) const {
    for (auto& f : fields_) {
        if (f.name == name) return f.value;
    }
    return "";
}

void Form::SetFieldValue(const std::string& name, std::string value) {
    for (auto& f : fields_) {
        if (f.name == name) { f.value = std::move(value); return; }
    }
}

std::vector<FormError> Form::Validate() const {
    std::vector<FormError> errors;
    for (auto& f : fields_) {
        if (f.required && f.value.empty()) {
            errors.push_back({f.name, "Required field"});
        }
        auto it = validators_.find(f.name);
        if (it != validators_.end()) {
            if (it->second.hasRegex && !f.value.empty()) {
                try {
                    std::regex re(it->second.pattern);
                    if (!std::regex_match(f.value, re)) {
                        errors.push_back({f.name, it->second.errorMsg});
                    }
                } catch (...) {}
            }
            if (it->second.hasRange && (f.type == FormField::Type::Number || f.type == FormField::Type::Slider)) {
                try {
                    double val = std::stod(f.value);
                    if (val < it->second.min || val > it->second.max) {
                        errors.push_back({f.name, "Value out of range [" +
                            std::to_string(it->second.min) + ", " + std::to_string(it->second.max) + "]"});
                    }
                } catch (...) {}
            }
        }
    }
    return errors;
}

void Form::SetFieldValidatorRegex(const std::string& name, std::string pattern, std::string errorMsg) {
    auto& v = validators_[name];
    v.pattern = std::move(pattern);
    v.errorMsg = std::move(errorMsg);
    v.hasRegex = true;
}
void Form::SetFieldMinMax(const std::string& name, double min, double max) {
    auto& v = validators_[name];
    v.min = min; v.max = max;
    v.hasRange = true;
}

void Form::SetOnSubmit(std::function<void()> callback) { on_submit_ = std::move(callback); }

void Form::Render() {
    if (!IsVisible()) return;
    ImGui::Begin(title_.c_str());

    for (auto& f : fields_) {
        char buf[256] = {};
        if (f.value.size() < 256) f.value.copy(buf, f.value.size());

        switch (f.type) {
        case FormField::Type::Text:
            if (ImGui::InputText(f.label.c_str(), buf, sizeof(buf))) {
                f.value = buf;
            }
            break;
        case FormField::Type::Checkbox: {
            bool checked = (f.value == "1");
            if (ImGui::Checkbox(f.label.c_str(), &checked)) {
                f.value = checked ? "1" : "0";
            }
            break;
        }
        case FormField::Type::Combo: {
            int idx = f.value.empty() ? 0 : std::stoi(f.value);
            if (ImGui::Combo(f.label.c_str(), &idx, "Item 0\0Item 1\0Item 2\0")) f.value = std::to_string(idx);
            break;
        }
        case FormField::Type::Slider: {
            float val = f.value.empty() ? 0.0f : std::stof(f.value);
            if (ImGui::SliderFloat(f.label.c_str(), &val, 0, 100, "%.1f")) f.value = std::to_string(val);
            break;
        }
        case FormField::Type::Number: {
            int val = f.value.empty() ? 0 : std::stoi(f.value);
            if (ImGui::InputInt(f.label.c_str(), &val)) f.value = std::to_string(val);
            break;
        }
        }
    }

    if (ImGui::Button("Submit")) {
        auto errors = Validate();
        if (errors.empty()) {
            last_errors_.clear();
            if (on_submit_) on_submit_();
        } else {
            last_errors_ = std::move(errors);
        }
    }

    // Display errors
    for (auto& e : last_errors_) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s: %s", e.field_name.c_str(), e.message.c_str());
    }

    ImGui::End();
}

std::string Form::Serialize() const {
    std::ostringstream ss;
    ss << "{";
    for (size_t i = 0; i < fields_.size(); i++) {
        if (i > 0) ss << ",";
        ss << "\"" << fields_[i].name << "\":\"" << fields_[i].value << "\"";
    }
    ss << "}";
    return ss.str();
}

bool Form::Deserialize(const std::string& json) {
    // Parse simple JSON: {"key1":"val1","key2":"val2"}
    std::regex re("\"([^\"]+)\"\\s*:\\s*\"([^\"]*)\"");
    std::smatch m;
    std::string s = json;
    bool any = false;
    while (std::regex_search(s, m, re)) {
        SetFieldValue(m[1].str(), m[2].str());
        s = m.suffix().str();
        any = true;
    }
    return any || json == "{}";
}

} // namespace unigui
