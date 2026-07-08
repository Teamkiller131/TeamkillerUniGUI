#pragma once
#include <unigui/widgets/widget_base.h>

#include <functional>
#include <string>
#include <vector>

namespace unigui {

/// Search box with filtered dropdown suggestions.
class SearchBox : public FluentWidget<SearchBox> {
public:
    SearchBox(std::string name, std::string hint = "Search...");

    void Render() override;

    void SetItems(std::vector<std::string> items) { items_ = std::move(items); }
    const std::string& GetQuery() const { return query_; }
    std::vector<std::string> GetMatches() const;

    /// Called when user selects an item from suggestions
    void SetOnSelect(std::function<void(const std::string&)> fn) { onSelect_ = std::move(fn); }
    /// Called when query changes
    void SetOnChange(std::function<void(const std::string&)> fn) { onChange_ = std::move(fn); }

    // ── Fluent (chainable) helpers — return SearchBox& via CRTP base ──────────
    SearchBox& WithItems(std::vector<std::string> items) {
        SetItems(std::move(items));
        return *this;
    }
    SearchBox& WithOnSelect(std::function<void(const std::string&)> fn) {
        SetOnSelect(std::move(fn));
        return *this;
    }
    SearchBox& WithOnChange(std::function<void(const std::string&)> fn) {
        SetOnChange(std::move(fn));
        return *this;
    }

private:
    std::string hint_;
    std::string query_;
    std::vector<std::string> items_;
    char buf_[256] = {};
    int selIdx_ = -1; // keyboard-highlighted suggestion (-1 = none)
    void Accept(const std::string& m);
    std::function<void(const std::string&)> onSelect_;
    std::function<void(const std::string&)> onChange_;
};

} // namespace unigui
