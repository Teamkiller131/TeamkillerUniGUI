#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <utility>

namespace unigui {

template <typename T> class ValueWidget : public Widget {
public:
    using OnChange = std::function<void(T)>;

    ValueWidget(std::string name, T initial = T{})
            : Widget(std::move(name))
            , value_(std::move(initial)) {}

    T GetValue() const { return value_; }
    void SetValue(T val) { value_ = std::move(val); }

    void SetOnChange(OnChange fn) { onChange_ = std::move(fn); }

protected:
    T value_;
    OnChange onChange_;

    /// Call this in RenderImpl when value changes to fire callback
    void NotifyChange(T oldVal) {
        if (onChange_ && value_ != oldVal)
            onChange_(value_);
    }
};

} // namespace unigui
