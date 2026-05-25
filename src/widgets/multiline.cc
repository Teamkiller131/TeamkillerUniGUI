#include <unigui/widgets/multiline.h>
#include <imgui.h>
#include <algorithm>
namespace unigui {
MultiLine::MultiLine(std::string n,std::string t,int ml):Widget(std::move(n)),text_(std::move(t)),maxLines_(ml){
    size_t sz=std::min(text_.size(),sizeof(buf_)-1); std::copy_n(text_.data(),sz,buf_); buf_[sz]=0;
}
void MultiLine::SetText(std::string t){text_=std::move(t);size_t sz=std::min(text_.size(),sizeof(buf_)-1);std::copy_n(text_.data(),sz,buf_);buf_[sz]=0;}
std::string MultiLine::GetText()const{return text_;} void MultiLine::SetMaxLines(int n){maxLines_=n;}
void MultiLine::Render(){
    if(!IsVisible())return;
    float h=ImGui::GetTextLineHeight()*maxLines_;
    ImGui::InputTextMultiline(GetName().c_str(),buf_,sizeof(buf_),ImVec2(-1,h),ImGuiInputTextFlags_ReadOnly);
}
}
