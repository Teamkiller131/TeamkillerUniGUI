#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>

namespace unigui {

class DirPath : public FluentWidget<DirPath> {
public:
    DirPath(std::string name, std::string label);
    void Render() override;
    std::string GetPath() const;
    void SetPath(std::string path);
    void SetTitle(std::string title);
    void SetOnPathChanged(std::function<void(std::string)> cb);

    // ── Fluent (chainable) helpers — return DirPath& via CRTP base ─────────
    DirPath& WithPath(std::string path) {
        SetPath(std::move(path));
        return *this;
    }
    DirPath& WithTitle(std::string title) {
        SetTitle(std::move(title));
        return *this;
    }
    DirPath& WithOnPathChanged(std::function<void(std::string)> cb) {
        SetOnPathChanged(std::move(cb));
        return *this;
    }

private:
    std::string label_, path_, title_;
    char buffer_[512] = {};
    std::function<void(std::string)> on_change_;
    bool OpenNativeDialog();
};

} // namespace unigui
