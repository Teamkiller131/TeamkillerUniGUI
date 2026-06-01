#pragma once
#include <unigui/widgets/value_widget.h>
#include <string>

namespace unigui {
template<typename T> class Slider : public ValueWidget<T> {
public:
    using ValueWidget<T>::SetValue;
    using ValueWidget<T>::GetValue;

    Slider(std::string name, std::string label, T value = T{}, T min = T{}, T max = T{100});
    void Render() override;
    void SetRange(T min, T max);
    const char* GetFormat() const;
    void SetFormat(const char* fmt);

    // GetValue/SetValue/SetOnChange inherited from ValueWidget<T>
private:
    std::string label_;
    T min_, max_;
    const char* format_ = "%.1f";
};

// Specialized for float
template<> inline void Slider<float>::Render() {
    if (!IsVisible()) return;
    float prev = this->value_;
    ImGui::SliderFloat(label_.c_str(), &this->value_, min_, max_, format_);
    this->NotifyChange(prev);
}
template<> inline void Slider<int>::Render() {
    if (!IsVisible()) return;
    int prev = this->value_;
    ImGui::SliderInt(label_.c_str(), &this->value_, min_, max_, format_);
    this->NotifyChange(prev);
}

template<typename T>
Slider<T>::Slider(std::string name, std::string label, T value, T min, T max)
    : ValueWidget<T>(std::move(name), value), label_(std::move(label)), min_(min), max_(max) {}

template<typename T> void Slider<T>::SetRange(T min, T max) { min_ = min; max_ = max; }
template<typename T> const char* Slider<T>::GetFormat() const { return format_; }
template<typename T> void Slider<T>::SetFormat(const char* fmt) { format_ = fmt; }
}
