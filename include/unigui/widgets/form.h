#pragma once

#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <regex>

namespace unigui {

struct FormField {
    std::string name;
    std::string label;
    enum class Type { Text, Checkbox, Combo, Slider, Number } type = Type::Text;
    bool required = false;
    std::string value;
    std::vector<std::string> options;
    double min = 0;
    double max = 100;
};

struct FormError {
    std::string field_name;
    std::string message;
};

class Form : public Widget {
public:
    Form(std::string name, std::string title);
    void Render() override;

    void AddTextField(std::string name, std::string label, bool required = false);
    void AddCheckbox(std::string name, std::string label);
    void AddComboField(std::string name, std::string label, std::vector<std::string> options);
    void AddSliderField(std::string name, std::string label, float min = 0, float max = 100);
    void AddNumberField(std::string name, std::string label, int min = 0, int max = 100);
    std::string GetFieldValue(const std::string& name) const;
    void SetFieldValue(const std::string& name, std::string value);

    std::vector<FormError> Validate() const;
    void SetOnSubmit(std::function<void()> callback);
    const std::vector<FormError>& GetErrors() const { return last_errors_; }

    /// Set a regex validator for a text field.
    void SetFieldValidatorRegex(const std::string& name, std::string pattern, std::string errorMsg);
    /// Set min/max range for a number or slider field.
    void SetFieldMinMax(const std::string& name, double min, double max);

    /// Serialize form state to a simple JSON string.
    std::string Serialize() const;
    /// Deserialize form state from a simple JSON string.
    bool Deserialize(const std::string& json);

private:
    struct FieldValidator {
        std::string pattern;
        std::string errorMsg;
        double min = 0, max = 0;
        bool hasRange = false;
        bool hasRegex = false;
    };

    std::string title_;
    std::vector<FormField> fields_;
    std::function<void()> on_submit_;
    std::vector<FormError> last_errors_;
    bool submitted_ = false;
    std::unordered_map<std::string, FieldValidator> validators_;
};

} // namespace unigui
