#pragma once
#include <unigui/widgets/widget_base.h>

#include <imgui.h>

#include <functional>
#include <string>
#include <variant>
#include <vector>

namespace unigui {

using PropValue = std::variant<bool, int, float, std::string>;

enum class PropType { Bool, Int, Float, String, Color, Combo };

struct PropertyDef {
    std::string name;
    std::string label;
    PropType type = PropType::String;
    PropValue value;
    // For Combo type: list of options
    std::vector<std::string> options;
    // For Int/Float: min/max range
    float minVal = 0, maxVal = 100;
    // Read-only flag
    bool readOnly = false;
};

/// Property grid for editing key-value properties (like VS Properties window).
class PropertyGrid : public Widget {
public:
    PropertyGrid(std::string name);

    void Render() override;

    void AddProperty(PropertyDef prop);
    void Clear();

    template <typename T> T GetValue(const std::string& name, T defaultVal = T{}) const;
    void SetValue(const std::string& name, PropValue val);

    void SetOnChange(std::function<void(const std::string& name, const PropValue& val)> fn) {
        onChange_ = std::move(fn);
    }

    const std::vector<PropertyDef>& GetProperties() const { return props_; }

private:
    void RenderProp(PropertyDef& prop);

    std::vector<PropertyDef> props_;
    std::function<void(const std::string&, const PropValue&)> onChange_;
};

} // namespace unigui
