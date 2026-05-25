#include <unigui/widgets/separator.h>
#include <imgui.h>
namespace unigui {
Separator::Separator(std::string n, std::string l):Widget(std::move(n)),label_(std::move(l)){}
void Separator::Render(){if(!IsVisible())return; if(label_.empty())ImGui::Separator(); else ImGui::SeparatorText(label_.c_str());}
void Separator::SetLabel(std::string l){label_=std::move(l);}
}
