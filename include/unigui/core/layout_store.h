#pragma once

#include <fstream>
#include <map>
#include <string>

namespace unigui {

/// LayoutStore — a tiny named string-value store for persisting UI layout state
/// (and other simple preferences) across runs. Keys map to opaque values such as
/// `MultiSplitter::SerializeLayout()` output, a theme-preset name, or a locale
/// tag. Persists as plain `name=value` lines; loading never throws on malformed
/// input (it skips bad lines), matching the project's "no throwing parsers" rule.
///
/// Header-only and dependency-free (just `<fstream>`), so it is unit-testable
/// against a temp file without any GUI/ImGui context.
///
/// Typical use:
/// ```cpp
/// LayoutStore store;
/// store.Load("layout.ini");
/// splitter.RestoreLayout(store.Get("main_split"));   // on startup
/// ...
/// store.Set("main_split", splitter.SerializeLayout());
/// store.Save("layout.ini");                          // on shutdown
/// ```
class LayoutStore {
public:
    /// Set (or replace) a value. Newlines in `value` are stripped so the line
    /// format stays intact.
    void Set(const std::string& name, const std::string& value) {
        std::string v = value;
        for (char& c : v)
            if (c == '\n' || c == '\r')
                c = ' ';
        kv_[name] = std::move(v);
    }
    /// Value for `name`, or "" if absent.
    std::string Get(const std::string& name) const {
        auto it = kv_.find(name);
        return it != kv_.end() ? it->second : std::string();
    }
    bool Has(const std::string& name) const { return kv_.find(name) != kv_.end(); }
    void Remove(const std::string& name) { kv_.erase(name); }
    void Clear() { kv_.clear(); }
    std::size_t Size() const { return kv_.size(); }
    const std::map<std::string, std::string>& Entries() const { return kv_; }

    /// Write all entries as `name=value` lines. Returns false if the file can't
    /// be opened.
    bool Save(const std::string& path) const {
        std::ofstream f(path, std::ios::binary | std::ios::trunc);
        if (!f)
            return false;
        for (const auto& [k, v] : kv_)
            f << k << '=' << v << '\n';
        return static_cast<bool>(f);
    }

    /// Load `name=value` lines, replacing the current contents. Lines without a
    /// '=' (or with an empty key) are skipped. Returns false only if the file
    /// can't be opened (a missing file is a benign false — first run).
    bool Load(const std::string& path) {
        std::ifstream f(path, std::ios::binary);
        if (!f)
            return false;
        kv_.clear();
        std::string line;
        while (std::getline(f, line)) {
            if (!line.empty() && line.back() == '\r')
                line.pop_back();
            const std::size_t eq = line.find('=');
            if (eq == std::string::npos || eq == 0)
                continue;
            kv_[line.substr(0, eq)] = line.substr(eq + 1);
        }
        return true;
    }

private:
    std::map<std::string, std::string> kv_;
};

} // namespace unigui
