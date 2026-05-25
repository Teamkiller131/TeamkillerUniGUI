#include <unigui/widgets/image.h>
#include <imgui.h>
namespace unigui {
Image::Image(std::string n, void* t, float w, float h):Widget(std::move(n)),tex_(t),w_(w),h_(h){}
void Image::SetTexture(void* t,float w,float h){tex_=t;w_=w;h_=h;}
void Image::Render(){if(!IsVisible()||!tex_)return; ImGui::Image((ImTextureID)tex_,ImVec2(w_>0?w_:ImGui::GetContentRegionAvail().x,h_>0?h_:ImGui::GetContentRegionAvail().y));}
}
