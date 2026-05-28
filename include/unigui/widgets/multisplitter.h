#pragma once
#include <unigui/widgets/widget_base.h>
#include <vector>
#include <functional>

namespace unigui {

/// MultiSplitter — N-panel resizable layout with drag handles.
/// Panels are added via AddPanel(ratio, content). Ratios sum to 1.0.
class MultiSplitter : public Widget {
public:
    enum Orientation { Horizontal, Vertical };

    MultiSplitter(std::string name, Orientation ori = Horizontal);

    void Render() override;
    void AddPanel(float ratio, std::function<void()> content);
    std::vector<float> GetRatios() const;
    void SetRatios(const std::vector<float>& ratios);

private:
    Orientation ori_;
    struct Panel { float ratio; std::function<void()> content; };
    std::vector<Panel> panels_;
    int dragIndex_ = -1;
};

} // namespace unigui
