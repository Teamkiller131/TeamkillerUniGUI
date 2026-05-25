#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>
namespace unigui {
class Breadcrumb : public Widget {
public:
    Breadcrumb(std::string name);
    void Render() override;
    void SetItems(std::vector<std::string> items);
    int GetSelected() const;
    void SetOnSelect(std::function<void(int)> cb);
private: std::vector<std::string> items_; int selected_=-1; std::function<void(int)> on_select_;
};
}
