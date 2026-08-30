#include <unigui/core/log.h>
#include <unigui/core/strutil.h>
#include <unigui/widgets/form.h>

#include <imgui.h>

#include <algorithm>
#include <cctype>
#include <regex>
#include <sstream>

#include "detail/combo_chevron.h"

namespace unigui {

namespace {
// Upper bound on the value length a regex validator will run against. Form fields hold
// short inputs (names, emails, codes), so anything longer is rejected without invoking the
// matcher — this caps polynomial backtracking to sub-millisecond and prevents pathological
// time/memory on huge inputs. (Exponential catastrophic backtracking from a host-authored
// pattern is a host trust-boundary concern; see SetFieldValidatorRegex.)
constexpr std::size_t kMaxRegexInputLen = 4096;

std::string EscapeJsonString(const std::string& value) {
    std::string out;
    out.reserve(value.size() + 8);
    for (char ch : value) {
        switch (ch) {
        case '\\':
            out += "\\\\";
            break;
        case '"':
            out += "\\\"";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += ch;
            break;
        }
    }
    return out;
}

bool ParseJsonString(const std::string& json, size_t& pos, std::string& out) {
    if (pos >= json.size() || json[pos] != '"')
        return false;
    pos++;
    out.clear();
    while (pos < json.size()) {
        char ch = json[pos++];
        if (ch == '"')
            return true;
        if (ch != '\\') {
            out += ch;
            continue;
        }
        if (pos >= json.size())
            return false;
        char esc = json[pos++];
        switch (esc) {
        case '"':
            out += '"';
            break;
        case '\\':
            out += '\\';
            break;
        case 'n':
            out += '\n';
            break;
        case 'r':
            out += '\r';
            break;
        case 't':
            out += '\t';
            break;
        default:
            out += esc;
            break;
        }
    }
    return false;
}

void SkipJsonWhitespace(const std::string& json, size_t& pos) {
    while (pos < json.size() && std::isspace(static_cast<unsigned char>(json[pos])))
        pos++;
}
} // namespace

Form::Form(std::string name, std::string title)
        : FluentWidget<Form>(std::move(name))
        , title_(std::move(title)) {}

void Form::AddTextField(std::string name, std::string label, bool required) {
    fields_.push_back(
        {std::move(name), std::move(label), FormField::Type::Text, required, "", {}, 0, 100});
}

void Form::AddCheckbox(std::string name, std::string label) {
    fields_.push_back(
        {std::move(name), std::move(label), FormField::Type::Checkbox, false, "0", {}, 0, 100});
}
void Form::AddComboField(std::string name, std::string label, std::vector<std::string> options) {
    std::string initial = options.empty() ? "" : options.front();
    fields_.push_back({std::move(name), std::move(label), FormField::Type::Combo, false,
                       std::move(initial), std::move(options), 0, 100});
}
void Form::AddSliderField(std::string name, std::string label, float min, float max) {
    auto fieldName = name;
    fields_.push_back({std::move(name),
                       std::move(label),
                       FormField::Type::Slider,
                       false,
                       std::to_string(min),
                       {},
                       min,
                       max});
    SetFieldMinMax(fieldName, min, max);
}
void Form::AddNumberField(std::string name, std::string label, int min, int max) {
    auto fieldName = name;
    fields_.push_back({std::move(name),
                       std::move(label),
                       FormField::Type::Number,
                       false,
                       std::to_string(min),
                       {},
                       static_cast<double>(min),
                       static_cast<double>(max)});
    SetFieldMinMax(fieldName, min, max);
}

std::string Form::GetFieldValue(const std::string& name) const {
    for (auto& f : fields_) {
        if (f.name == name)
            return f.value;
    }
    return "";
}

void Form::SetFieldValue(const std::string& name, std::string value) {
    for (auto& f : fields_) {
        if (f.name == name) {
            f.value = std::move(value);
            return;
        }
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
            if (it->second.hasRegex && it->second.compiled && !f.value.empty()) {
                // Match against the regex compiled once at set-time (no per-call recompile).
                // std::regex has no complexity governor, so bound the matched length: form
                // field values aren't documents, and capping the input keeps polynomial
                // backtracking sub-millisecond and rejects implausibly long inputs outright.
                if (f.value.size() > kMaxRegexInputLen) {
                    errors.push_back({f.name, it->second.errorMsg});
                } else {
                    bool matched = false;
                    try {
                        matched = std::regex_match(f.value, *it->second.compiled);
                    } catch (const std::exception& e) {
                        // MSVC's std::regex raises error_complexity when backtracking gets
                        // too deep; treat that as a failed match rather than propagating.
                        UNIGUI_LOG_WARN("Form '{}' field '{}': regex match aborted: {}", GetName(),
                                        f.name, e.what());
                    }
                    if (!matched)
                        errors.push_back({f.name, it->second.errorMsg});
                }
            }
            if (it->second.hasRange &&
                (f.type == FormField::Type::Number || f.type == FormField::Type::Slider)) {
                // Non-throwing parse (std::stod is banned project-wide): distinguish a
                // genuine parse failure from an out-of-range value.
                double val = 0.0;
                if (!TryToDouble(f.value, val)) {
                    errors.push_back({f.name, "Not a valid number"});
                } else if (val < it->second.min || val > it->second.max) {
                    errors.push_back({f.name, "Value out of range [" +
                                                  std::to_string(it->second.min) + ", " +
                                                  std::to_string(it->second.max) + "]"});
                }
            }
        }
    }
    return errors;
}

void Form::SetFieldValidatorRegex(const std::string& name, std::string pattern,
                                  std::string errorMsg) {
    auto& v = validators_[name];
    v.pattern = std::move(pattern);
    v.errorMsg = std::move(errorMsg);
    // Compile the pattern ONCE here rather than on every Validate() call. An invalid
    // pattern disables the validator (hasRegex stays false) instead of being recompiled
    // and logged every frame. NOTE: the pattern is a host-controlled trust boundary —
    // std::regex on libstdc++/libc++ has no backtracking governor, so a host sourcing a
    // pattern from untrusted input must pre-vet it (Validate() additionally bounds the
    // matched value length, which caps polynomial blow-up and huge inputs).
    try {
        v.compiled = std::make_shared<const std::regex>(v.pattern, std::regex::ECMAScript |
                                                                       std::regex::optimize);
        v.hasRegex = true;
    } catch (const std::exception& e) {
        v.compiled.reset();
        v.hasRegex = false;
        UNIGUI_LOG_WARN("Form '{}' field '{}': invalid validator regex '{}': {}", GetName(), name,
                        v.pattern, e.what());
    }
}
void Form::SetFieldMinMax(const std::string& name, double min, double max) {
    auto& v = validators_[name];
    v.min = min;
    v.max = max;
    v.hasRange = true;
}

void Form::SetOnSubmit(std::function<void()> callback) {
    on_submit_ = std::move(callback);
}

void Form::Render() {
    if (!IsVisible())
        return;
    ImGui::PushID(GetName().c_str());
    ImGui::Begin(title_.c_str());

    for (auto& f : fields_) {
        char buf[256] = {};
        if (f.value.size() < 256)
            f.value.copy(buf, f.value.size());

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
            int idx = 0;
            for (int i = 0; i < (int) f.options.size(); ++i) {
                if (f.value == f.options[i]) {
                    idx = i;
                    break;
                }
            }
            if (!f.value.empty() &&
                std::all_of(f.value.begin(), f.value.end(),
                            [](unsigned char ch) { return std::isdigit(ch) != 0; })) {
                int parsed = ToIntOr(f.value);
                if (parsed >= 0 && parsed < (int) f.options.size())
                    idx = parsed;
            }
            const char* preview = f.options.empty() ? "" : f.options[idx].c_str();
            const auto comboFrame = detail::CaptureComboFrame();
            const bool comboOpen =
                ImGui::BeginCombo(f.label.c_str(), preview, ImGuiComboFlags_NoArrowButton);
            detail::DrawComboChevron(comboFrame, comboOpen || ImGui::IsItemHovered());
            if (comboOpen) {
                for (int i = 0; i < (int) f.options.size(); ++i) {
                    bool selected = (i == idx);
                    if (ImGui::Selectable(f.options[i].c_str(), selected))
                        f.value = f.options[i];
                    if (selected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }
            break;
        }
        case FormField::Type::Slider: {
            float val = ToFloatOr(f.value);
            if (ImGui::SliderFloat(f.label.c_str(), &val, (float) f.min, (float) f.max, "%.1f"))
                f.value = std::to_string(val);
            break;
        }
        case FormField::Type::Number: {
            int val = ToIntOr(f.value, static_cast<int>(f.min));
            if (ImGui::InputInt(f.label.c_str(), &val))
                f.value = std::to_string(val);
            break;
        }
        }
    }

    if (ImGui::Button("Submit")) {
        auto errors = Validate();
        if (errors.empty()) {
            last_errors_.clear();
            if (on_submit_)
                on_submit_();
        } else {
            last_errors_ = std::move(errors);
        }
    }

    // Display errors
    for (auto& e : last_errors_) {
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s: %s", e.field_name.c_str(),
                           e.message.c_str());
    }

    ImGui::End();
    ImGui::PopID();
}

std::string Form::Serialize() const {
    std::ostringstream ss;
    ss << "{";
    for (size_t i = 0; i < fields_.size(); i++) {
        if (i > 0)
            ss << ",";
        ss << "\"" << EscapeJsonString(fields_[i].name) << "\":\""
           << EscapeJsonString(fields_[i].value) << "\"";
    }
    ss << "}";
    return ss.str();
}

bool Form::Deserialize(const std::string& json) {
    size_t pos = 0;
    SkipJsonWhitespace(json, pos);
    if (pos >= json.size() || json[pos] != '{')
        return false;
    ++pos; // consume '{' (kept out of the condition: mutating pos inside a && chain is a
           // bugprone-inc-dec-in-conditions footgun even though short-circuit makes it safe)
    SkipJsonWhitespace(json, pos);
    bool any = false;
    if (pos < json.size() && json[pos] == '}')
        return true;
    while (pos < json.size()) {
        std::string key, value;
        if (!ParseJsonString(json, pos, key))
            return false;
        SkipJsonWhitespace(json, pos);
        if (pos >= json.size() || json[pos] != ':')
            return false;
        ++pos; // consume ':' (see the '{' note above)
        SkipJsonWhitespace(json, pos);
        if (!ParseJsonString(json, pos, value))
            return false;
        SetFieldValue(key, value);
        any = true;
        SkipJsonWhitespace(json, pos);
        if (pos < json.size() && json[pos] == ',') {
            pos++;
            SkipJsonWhitespace(json, pos);
            continue;
        }
        if (pos < json.size() && json[pos] == '}')
            return any;
        return false;
    }
    return false;
}

} // namespace unigui
