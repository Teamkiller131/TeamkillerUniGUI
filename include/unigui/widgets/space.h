#pragma once
#include <unigui/widgets/widget_base.h>

#include <string>
namespace unigui {
class DockSpace : public FluentWidget<DockSpace> {
public:
    DockSpace(std::string name);
    void Render() override;

private:
    ImGuiID dock_id_ = 0;
};
} // namespace unigui
