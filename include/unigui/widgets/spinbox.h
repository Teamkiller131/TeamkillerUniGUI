#pragma once
#include <unigui/widgets/value_widget.h>

#include <string>
namespace unigui {
template <typename T> class SpinBox : public ValueWidget<T> {
public:
    using ValueWidget<T>::SetValue;
    using ValueWidget<T>::GetValue;
    SpinBox(std::string name, std::string label, T val = T{}, T mn = T{}, T mx = T{100},
            T step = T{1});
    void Render() override;
    void SetRange(T min, T max);
    void SetStep(T step);

private:
    std::string label_;
    T min_, max_, step_;
};
template <> inline void SpinBox<int>::Render() {
    if (!IsVisible())
        return;
    int prev = this->value_;
    ImGui::InputInt(label_.c_str(), &this->value_);
    if (this->value_ < min_)
        this->value_ = min_;
    if (this->value_ > max_)
        this->value_ = max_;
    this->NotifyChange(prev);
}
template <> inline void SpinBox<float>::Render() {
    if (!IsVisible())
        return;
    float prev = this->value_;
    ImGui::InputFloat(label_.c_str(), &this->value_);
    if (this->value_ < min_)
        this->value_ = min_;
    if (this->value_ > max_)
        this->value_ = max_;
    this->NotifyChange(prev);
}
template <typename T>
SpinBox<T>::SpinBox(std::string n, std::string l, T v, T mn, T mx, T s)
        : ValueWidget<T>(std::move(n), v)
        , label_(std::move(l))
        , min_(mn)
        , max_(mx)
        , step_(s) {}
template <typename T> void SpinBox<T>::SetRange(T mn, T mx) {
    min_ = mn;
    max_ = mx;
}
template <typename T> void SpinBox<T>::SetStep(T s) {
    step_ = s;
}
} // namespace unigui
