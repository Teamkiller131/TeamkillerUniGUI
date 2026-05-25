#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
namespace unigui {
class LoadingIndicator : public Widget {
public:
    LoadingIndicator(std::string name, float radius = 16.0f);
    void Render() override;
    void SetActive(bool active);
    bool IsActive() const;
private:
    float radius_; bool active_=true; float angle_=0;
};
}
