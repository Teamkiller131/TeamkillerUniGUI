#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>
namespace unigui {
template<typename T> class SpinBox : public Widget {
public:
    SpinBox(std::string name, std::string label, T val = T{}, T mn = T{}, T mx = T{100}, T step = T{1});
    void Render() override;
    T GetValue() const; void SetValue(T v);
    void SetRange(T min, T max); void SetStep(T step);
    void SetOnChange(std::function<void(T)> cb);
private: std::string label_; T val_, min_, max_, step_; std::function<void(T)> on_change_;
};
template<> inline void SpinBox<int>::Render() { if(!IsVisible())return; int prev=val_; ImGui::InputInt(label_.c_str(),&val_); if(val_<min_)val_=min_; if(val_>max_)val_=max_; if(val_!=prev&&on_change_)on_change_(val_); }
template<> inline void SpinBox<float>::Render() { if(!IsVisible())return; float prev=val_; ImGui::InputFloat(label_.c_str(),&val_); if(val_<min_)val_=min_; if(val_>max_)val_=max_; if(val_!=prev&&on_change_)on_change_(val_); }
template<typename T> SpinBox<T>::SpinBox(std::string n, std::string l, T v, T mn, T mx, T s):Widget(std::move(n)),label_(std::move(l)),val_(v),min_(mn),max_(mx),step_(s){}
template<typename T> T SpinBox<T>::GetValue()const{return val_;}
template<typename T> void SpinBox<T>::SetValue(T v){val_=v;}
template<typename T> void SpinBox<T>::SetRange(T mn,T mx){min_=mn;max_=mx;}
template<typename T> void SpinBox<T>::SetStep(T s){step_=s;}
template<typename T> void SpinBox<T>::SetOnChange(std::function<void(T)> cb){on_change_=std::move(cb);}
}
