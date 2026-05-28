#include <unigui/widgets/inputfloat.h>
#include <imgui.h>
namespace unigui {
InputFloat::InputFloat(std::string n, std::string l, float v, float mn, float mx):Widget(std::move(n)),label_(std::move(l)),val_(v),min_(mn),max_(mx){}
void InputFloat::Render(){if(!IsVisible())return;float prev=val_;ImGui::PushID(GetName().c_str());ImGui::InputFloat(label_.c_str(),&val_,0,0,fmt_);if(!suffix_.empty()){ImGui::SameLine();ImGui::TextUnformatted(suffix_.c_str());}ImGui::PopID();if(val_<min_)val_=min_;if(val_>max_)val_=max_;if(val_!=prev&&on_change_)on_change_(val_);} 
float InputFloat::GetValue()const{return val_;} void InputFloat::SetValue(float v){val_=v;} void InputFloat::SetRange(float mn,float mx){min_=mn;max_=mx;} void InputFloat::SetFormat(const char*f){fmt_=f;} void InputFloat::SetOnChange(std::function<void(float)>cb){on_change_=std::move(cb);} void InputFloat::SetSuffix(std::string s){suffix_=std::move(s);} 
}
