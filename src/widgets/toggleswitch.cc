#include <unigui/widgets/toggleswitch.h>
#include <imgui.h>
namespace unigui {
ToggleSwitch::ToggleSwitch(std::string n, std::string l, bool on):Widget(std::move(n)),label_(std::move(l)),on_(on){}
void ToggleSwitch::Render(){
    if(!IsVisible())return;
    bool prev=on_;

    // Animated transition
    float target = on_ ? 1.f : 0.f;
    if (anim_.progress != target && !anim_.IsPlaying())
        anim_.Play(0.2f, fx::EasingCurve::EaseOut);
    float t = anim_.Update(ImGui::GetIO().DeltaTime);

    // Push alpha for smooth on/off feel
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, 0.7f + 0.3f * t);
    ImGui::PushID(GetName().c_str());
    ImGui::Checkbox(label_.c_str(),&on_);
    ImGui::PopID();
    ImGui::PopStyleVar();

    if(on_!=prev&&on_change_)on_change_(on_);
}
bool ToggleSwitch::IsOn()const{return on_;}
void ToggleSwitch::SetOn(){on_=true;}void ToggleSwitch::SetOff(){on_=false;}void ToggleSwitch::Toggle(){on_=!on_;}
void ToggleSwitch::SetOnChange(std::function<void(bool)> cb){on_change_=std::move(cb);}
}
