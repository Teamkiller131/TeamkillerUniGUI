#include <unigui/core/locale.h>
#include <vector>

namespace unigui {

std::string Locale::current_ = "en_US";

std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& Locale::Table() {
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> table;
    return table;
}

void Locale::SetCurrent(const std::string& locale) { current_ = locale; }
const std::string& Locale::GetCurrent() { return current_; }

void Locale::Set(const std::string& locale, const std::string& key, const std::string& value) {
    Table()[locale][key] = value;
}

std::string Locale::Tr(const std::string& key) {
    auto& table = Table();
    auto it = table.find(current_);
    if (it != table.end()) {
        auto kit = it->second.find(key);
        if (kit != it->second.end()) return kit->second;
    }
    return key; // fallback: return key itself
}

bool Locale::Has(const std::string& key) {
    auto& table = Table();
    auto it = table.find(current_);
    if (it != table.end()) return it->second.count(key) > 0;
    return false;
}

void Locale::Clear() { Table().clear(); }

std::vector<std::string> Locale::GetLocales() {
    std::vector<std::string> locales;
    for (auto& [code, _] : Table()) locales.push_back(code);
    return locales;
}

} // namespace unigui
