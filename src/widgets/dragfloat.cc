#include <unigui/widgets/dragfloat.h>
#include <imgui.h>
namespace unigui {
DragFloat::DragFloat(std::string n, std::string l, float v, float s, float mn, float mx):Widget(std::move(n)),label_(std::move(l)),value_(v),speed_(s),min_(mn),max_(mx),changed_(false){}
void DragFloat::Render(){if(!IsVisible())return;ImGui::PushID(GetName().c_str());changed_=ImGui::DragFloat(label_.c_str(),&value_,speed_,min_,max_,"%.3f");ImGui::PopID();if(value_<min_)value_=min_;if(value_>max_)value_=max_;}
float DragFloat::GetValue()const{return value_;} void DragFloat::SetValue(float v){value_=v;} bool DragFloat::WasChanged()const{return changed_;}
}
