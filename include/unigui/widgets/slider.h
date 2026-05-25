#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {
template<typename T> class Slider : public Widget {
public:
    Slider(std::string name, std::string label, T value = T{}, T min = T{}, T max = T{100});
    void Render() override;
    T GetValue() const;
    void SetValue(T value);
    void SetRange(T min, T max);
    const char* GetFormat() const;
    void SetFormat(const char* fmt);
    void SetOnChange(std::function<void(T)> callback);
private:
    std::string label_;
    T value_, min_, max_;
    const char* format_ = "%.1f";
    std::function<void(T)> on_change_;
};
// Specialized for float
template<> inline void Slider<float>::Render() {
    if (!IsVisible()) return;
    float prev = value_;
    ImGui::SliderFloat(label_.c_str(), &value_, min_, max_, format_);
    if (value_ != prev && on_change_) on_change_(value_);
}
template<> inline void Slider<int>::Render() {
    if (!IsVisible()) return;
    int prev = value_;
    ImGui::SliderInt(label_.c_str(), &value_, min_, max_, format_);
    if (value_ != prev && on_change_) on_change_(value_);
}
template<typename T> Slider<T>::Slider(std::string name, std::string label, T value, T min, T max)
    : Widget(std::move(name)), label_(std::move(label)), value_(value), min_(min), max_(max) {}
template<typename T> T Slider<T>::GetValue() const { return value_; }
template<typename T> void Slider<T>::SetValue(T value) { value_ = value; }
template<typename T> void Slider<T>::SetRange(T min, T max) { min_ = min; max_ = max; }
template<typename T> const char* Slider<T>::GetFormat() const { return format_; }
template<typename T> void Slider<T>::SetFormat(const char* fmt) { format_ = fmt; }
template<typename T> void Slider<T>::SetOnChange(std::function<void(T)> callback) { on_change_ = std::move(callback); }
}
