#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <functional>

namespace unigui {

class FilePath : public Widget {
public:
    enum Mode { Open, Save };
    FilePath(std::string name, std::string label, Mode mode = Open);
    void Render() override;
    std::string GetPath() const;
    void SetPath(std::string path);
    void SetFilter(std::string filter);
    void SetTitle(std::string title);
    void SetMode(Mode mode);
    void SetOnPathChanged(std::function<void(std::string)> cb);
private:
    std::string label_, path_, filter_, title_;
    Mode mode_;
    char buffer_[512] = {};
    std::function<void(std::string)> on_change_;
    bool OpenNativeDialog();
};

}
