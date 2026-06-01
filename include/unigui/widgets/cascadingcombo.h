#pragma once
#include <unigui/widgets/widget_base.h>
#include <string>
#include <vector>
#include <functional>

namespace unigui {

class CascadingCombo : public Widget {
public:
    struct Level {
        std::string label;
        std::vector<std::string> options;
        int selectedIndex = 0;
    };

    CascadingCombo(std::string name, std::vector<Level> levels = {});
    void Render() override;
    void SetLevels(std::vector<Level> levels);
    void SetOptions(int level, std::vector<std::string> options);
    int GetSelectedIndex(int level) const;
    std::string GetSelectedText(int level) const;

    using OnChanged = std::function<void(int level, int index)>;
    void SetOnChanged(OnChanged fn);

private:
    std::vector<Level> levels_;
    OnChanged onChanged_;
};

} // namespace unigui
