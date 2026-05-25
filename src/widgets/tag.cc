#include <unigui/widgets/tag.h>
#include <imgui.h>
namespace unigui {
Tag::Tag(std::string n,std::string t,std::array<float,3> c):Widget(std::move(n)),text_(std::move(t)),color_(c){}
void Tag::SetText(std::string t){text_=std::move(t);} void Tag::SetColor(std::array<float,3>c){color_=c;}
void Tag::SetRemovable(bool r){removable_=r;} bool Tag::RemoveClicked()const{return removeClicked_;}
void Tag::Render(){
    if(!IsVisible())return;
    ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(color_[0],color_[1],color_[2],1));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(color_[0]*1.2f,color_[1]*1.2f,color_[2]*1.2f,1));
    ImGui::SmallButton(text_.c_str());
    removeClicked_=ImGui::IsItemClicked();
    ImGui::PopStyleColor(2);
}
}
