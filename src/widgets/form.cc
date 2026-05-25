#include <unigui/widgets/form.h>
#include <imgui.h>

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
    }
    return errors;
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

} // namespace unigui
