#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {

class DirPath : public Widget {
public:
    DirPath(std::string name, std::string label);
    void Render() override;
    std::string GetPath() const;
    void SetPath(std::string path);
    void SetTitle(std::string title);
    void SetOnPathChanged(std::function<void(std::string)> cb);
private:
    std::string label_, path_, title_;
    char buffer_[512] = {};
    std::function<void(std::string)> on_change_;
    bool OpenNativeDialog();
};

}
