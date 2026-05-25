#pragma once
#include <string>
#include <unordered_map>
#include <functional>

namespace unigui {

/// Simple i18n locale system with translation table.
/// Usage: Locale::SetCurrent("zh_CN"); std::string s = Locale::Tr("hello");
class Locale {
public:
    /// Set the current active locale (e.g., "en_US", "zh_CN", "ja_JP").
    static void SetCurrent(const std::string& locale);
    static const std::string& GetCurrent();

    /// Add or update a translation for a key in a specific locale.
    static void Set(const std::string& locale, const std::string& key, const std::string& value);

    /// Translate a key to the current locale. Returns key if not found.
    static std::string Tr(const std::string& key);

    /// Check if a translation exists for the current locale.
    static bool Has(const std::string& key);

    /// Clear all translations.
    static void Clear();

    /// Get all available locale codes.
    static std::vector<std::string> GetLocales();

private:
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& Table();
    static std::string current_;
};

} // namespace unigui
