#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

class FilePath : public Widget {
public:
    enum Mode { Open, Save };
    FilePath(std::string name, std::string label, Mode mode = Open);
    void Render() override;
    // 直接弹出系统文件选择框（不渲染内嵌输入框）；选中后触发 OnPathChanged。
    // 返回 true 表示用户确认了选择。
    bool OpenDialog();
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

} // namespace unigui
