#include <unigui/widgets/image.h>
#include <imgui.h>
#include <algorithm>
namespace unigui {
Image::Image(std::string n, void* t, float w, float h):Widget(std::move(n)),tex_(t),w_(w),h_(h){}
void Image::SetTexture(void* t,float w,float h){tex_=t;w_=w;h_=h;}
void Image::SetScaleMode(ScaleMode m) { mode_ = m; }
void Image::Render() {
    if(!IsVisible()||!tex_)return;
    float rw=w_,rh=h_;
    if(mode_==Fit){auto a=ImGui::GetContentRegionAvail(); if(rw<=0)rw=a.x; if(rh<=0)rh=a.y; float s=std::min(a.x/rw,a.y/rh); rw*=s;rh*=s;}
    else if(mode_==Stretch){auto a=ImGui::GetContentRegionAvail(); rw=a.x; rh=a.y;}
    ImGui::Image((ImTextureID)tex_,ImVec2(rw,rh));
}
}
