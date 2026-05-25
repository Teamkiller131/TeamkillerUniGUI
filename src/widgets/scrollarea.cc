#include <unigui/widgets/scrollarea.h>
#include <imgui.h>
namespace unigui {
ScrollArea::ScrollArea(std::string n, float w, float h):Widget(std::move(n)),w_(w),h_(h){}
void ScrollArea::Render(){if(!IsVisible())return; ImGui::BeginChild(GetName().c_str(),ImVec2(w_,h_),ImGuiChildFlags_Borders|ImGuiChildFlags_NavFlattened); if(cb_)cb_(); ImGui::EndChild();}
void ScrollArea::SetContentCallback(std::function<void()>cb){cb_=std::move(cb);}
void ScrollArea::SetSize(float w,float h){w_=w;h_=h;}
}
