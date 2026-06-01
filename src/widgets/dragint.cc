#include <unigui/widgets/dragint.h>
#include <imgui.h>
namespace unigui {
DragInt::DragInt(std::string n, std::string l, int v, float s, int mn, int mx):ValueWidget<int>(std::move(n), v),label_(std::move(l)),speed_(s),min_(mn),max_(mx),changed_(false){}
void DragInt::Render(){if(!IsVisible())return;int prev=value_;ImGui::PushID(GetName().c_str());changed_=ImGui::DragInt(label_.c_str(),&value_,speed_,min_,max_,"%d");ImGui::PopID();if(value_<min_)value_=min_;if(value_>max_)value_=max_; NotifyChange(prev);}
bool DragInt::WasChanged()const{return changed_;}
}
