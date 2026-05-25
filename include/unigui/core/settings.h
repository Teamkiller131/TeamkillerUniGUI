#pragma once
#include <string>
#include <unordered_map>
#include <functional>

namespace unigui {

/// Simple key-value settings store with INI persistence.
/// Usage: Settings::Instance().Set("window.x", "100"); auto v = Settings::Instance().Get("window.x");
class Settings {
public:
    static Settings& Instance();

    /// Set a string value.
    void Set(const std::string& key, const std::string& value);
    /// Get a string value. Returns defaultVal if key not found.
    std::string Get(const std::string& key, const std::string& defaultVal = "") const;

    /// Typed convenience: int
    void SetInt(const std::string& key, int value);
    int GetInt(const std::string& key, int defaultVal = 0) const;

    /// Typed convenience: float
    void SetFloat(const std::string& key, float value);
    float GetFloat(const std::string& key, float defaultVal = 0.0f) const;

    /// Typed convenience: bool
    void SetBool(const std::string& key, bool value);
    bool GetBool(const std::string& key, bool defaultVal = false) const;

    /// Check if a key exists.
    bool Has(const std::string& key) const;

    /// Save all settings to an INI file. Returns success.
    bool Save(const std::string& path);
    /// Load settings from an INI file. Returns success.
    bool Load(const std::string& path);

    /// Clear all settings.
    void Clear();

private:
    Settings() = default;
    std::unordered_map<std::string, std::string> data_;
};

} // namespace unigui
