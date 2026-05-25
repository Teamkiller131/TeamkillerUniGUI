#include <unigui/widgets/toggleswitch.h>
#include <imgui.h>
namespace unigui {
ToggleSwitch::ToggleSwitch(std::string n, std::string l, bool on):Widget(std::move(n)),label_(std::move(l)),on_(on){}
void ToggleSwitch::Render(){if(!IsVisible())return; bool prev=on_; ImGui::Checkbox(label_.c_str(),&on_); if(on_!=prev&&on_change_)on_change_(on_);}
bool ToggleSwitch::IsOn()const{return on_;}
void ToggleSwitch::SetOn(){on_=true;} void ToggleSwitch::SetOff(){on_=false;} void ToggleSwitch::Toggle(){on_=!on_;}
void ToggleSwitch::SetOnChange(std::function<void(bool)> cb){on_change_=std::move(cb);}
}
