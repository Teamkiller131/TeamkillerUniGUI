#pragma once

#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {

struct FormField {
    std::string name;
    std::string label;
    enum class Type { Text, Checkbox } type = Type::Text;
    bool required = false;
    std::string value;
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
    std::string GetFieldValue(const std::string& name) const;
    void SetFieldValue(const std::string& name, std::string value);

    std::vector<FormError> Validate() const;
    void SetOnSubmit(std::function<void()> callback);
    const std::vector<FormError>& GetErrors() const { return last_errors_; }

private:
    std::string title_;
    std::vector<FormField> fields_;
    std::function<void()> on_submit_;
    std::vector<FormError> last_errors_;
    bool submitted_ = false;
};

} // namespace unigui
