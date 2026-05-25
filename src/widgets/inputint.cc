#include <unigui/widgets/inputint.h>
#include <imgui.h>
namespace unigui {
InputInt::InputInt(std::string n, std::string l, int v, int mn, int mx):Widget(std::move(n)),label_(std::move(l)),val_(v),min_(mn),max_(mx){}
void InputInt::Render(){if(!IsVisible())return;int prev=val_; ImGui::InputInt(label_.c_str(),&val_); if(val_<min_)val_=min_;if(val_>max_)val_=max_; if(val_!=prev&&on_change_)on_change_(val_);}
int InputInt::GetValue()const{return val_;} void InputInt::SetValue(int v){val_=v;} void InputInt::SetRange(int mn,int mx){min_=mn;max_=mx;} void InputInt::SetOnChange(std::function<void(int)>cb){on_change_=std::move(cb);}
}
