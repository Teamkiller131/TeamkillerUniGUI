#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

class FilePath : public FluentWidget<FilePath> {
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

    // ── Fluent (chainable) helpers — return FilePath& via CRTP base ──────────
    FilePath& WithPath(std::string path) {
        SetPath(std::move(path));
        return *this;
    }
    FilePath& WithFilter(std::string filter) {
        SetFilter(std::move(filter));
        return *this;
    }
    FilePath& WithTitle(std::string title) {
        SetTitle(std::move(title));
        return *this;
    }
    FilePath& WithMode(Mode mode) {
        SetMode(mode);
        return *this;
    }
    FilePath& WithOnPathChanged(std::function<void(std::string)> cb) {
        SetOnPathChanged(std::move(cb));
        return *this;
    }

private:
    std::string label_, path_, filter_, title_;
    Mode mode_;
    char buffer_[512] = {};
    std::function<void(std::string)> on_change_;
    bool OpenNativeDialog();
};

} // namespace unigui
