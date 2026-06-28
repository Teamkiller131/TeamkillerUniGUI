#pragma once
// NOTE: the TOML/JSON backends (cpptoml, nlohmann/json) are implementation details
// and are included in src/config/config.cc, not here — they must not leak into every
// consumer that includes this public header.
#include <unigui/core/error.h>

#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

namespace unigui::config {

/// Unified config system: TOML + JSON + INI, layered merging.
class Store {
public:
    static Store& Instance();

    // ── Load ────────────────────────────────────────────────────────────────
    // Each returns a value result on success, or an error result:
    // Err(ErrorCode::FileNotFound) if the path can't be opened, or
    // Err(ErrorCode::ParseFailed) if the contents are malformed.
    Result<void> LoadTOML(const std::string& path);
    Result<void> LoadJSON(const std::string& path);
    Result<void> LoadINI(const std::string& path);

    // ── Save ────────────────────────────────────────────────────────────────
    bool SaveTOML(const std::string& path) const;
    bool SaveJSON(const std::string& path) const;

    // ── Typed access ────────────────────────────────────────────────────────
    template <typename T> T Get(const std::string& key, T defaultVal = T{}) const;
    template <typename T> void Set(const std::string& key, const T& value);

    // String specialization
    std::string GetString(const std::string& key, const std::string& defaultVal = "") const;
    void SetString(const std::string& key, const std::string& value);

    int GetInt(const std::string& key, int defaultVal = 0) const;
    void SetInt(const std::string& key, int value);

    double GetDouble(const std::string& key, double defaultVal = 0.0) const;
    void SetDouble(const std::string& key, double value);

    bool GetBool(const std::string& key, bool defaultVal = false) const;
    void SetBool(const std::string& key, bool value);

    // ── Keys ────────────────────────────────────────────────────────────────
    bool Has(const std::string& key) const;
    std::vector<std::string> Keys() const;

    // ── Layering ────────────────────────────────────────────────────────────
    /// Merge another config on top (higher priority overwrites).
    void Merge(const Store& other);

    /// Clear all values.
    void Clear();

private:
    Store() = default;
    std::unordered_map<std::string, std::string> data_;
    void SetValue(const std::string& key, const std::string& value);
    std::string GetValue(const std::string& key, const std::string& defaultVal = "") const;
};

} // namespace unigui::config
