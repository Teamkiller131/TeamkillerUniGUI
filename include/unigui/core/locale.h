#pragma once
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace unigui {

/// Simple i18n locale system with translation table.
/// Usage: Locale::SetCurrent("zh_CN"); std::string s = Locale::Tr("hello");
///
/// Lookup follows a fallback chain so a partially-translated locale degrades
/// gracefully instead of leaking raw keys: current locale → its base language
/// (e.g. "zh_CN" → "zh") → the configured fallback locale (default "en_US") →
/// the key itself.
class Locale {
public:
    /// Set the current active locale (e.g., "en_US", "zh_CN", "ja_JP").
    static void SetCurrent(const std::string& locale);
    static const std::string& GetCurrent();

    /// Add or update a translation for a key in a specific locale.
    static void Set(const std::string& locale, const std::string& key, const std::string& value);

    /// Set the fallback locale used when a key is missing from the current
    /// locale and its base language (default "en_US").
    static void SetFallback(const std::string& locale);
    static const std::string& GetFallback();

    /// Translate a key, following the fallback chain. Returns the key itself if
    /// no translation is found anywhere.
    static std::string Tr(const std::string& key);

    /// Translate `key`, then substitute positional placeholders {0}, {1}, … with
    /// the supplied arguments (left as-is if an index is out of range).
    static std::string Tr(const std::string& key, const std::vector<std::string>& args);

    /// Check if a translation exists for the current locale (no fallback).
    static bool Has(const std::string& key);

    /// Whether a locale (default: the current one) is written right-to-left
    /// (Arabic / Hebrew / Persian / Urdu). RTL *layout mirroring* is a separate,
    /// larger effort; this is the detection primitive widgets/layout can branch on.
    static bool IsRTL();
    static bool IsRTL(const std::string& locale);

    /// Clear all translations.
    static void Clear();

    /// Load translations from a JSON file {"key":"value",...}. Returns true on success.
    static bool LoadFromFile(const std::string& path);

    /// Load built-in translations for common locales.
    static void LoadBuiltin();

    /// Get all available locale codes.
    static std::vector<std::string> GetLocales();

private:
    /// Look up `key` in exactly `locale` (no fallback). Returns true + writes
    /// `out` on hit.
    static bool Lookup(const std::string& locale, const std::string& key, std::string& out);
    /// Base language of a locale code ("zh_CN" → "zh"; "" if none).
    static std::string BaseLanguage(const std::string& locale);
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& Table();
    static std::string current_;
    static std::string fallback_;
};

} // namespace unigui
