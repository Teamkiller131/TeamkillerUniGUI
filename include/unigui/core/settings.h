#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include <functional>

namespace unigui {

class Settings {
public:
    static Settings& Instance();

    void Set(const std::string& key, const std::string& value);
    std::string Get(const std::string& key, const std::string& defaultVal = "") const;
    void SetInt(const std::string& key, int value);
    int GetInt(const std::string& key, int defaultVal = 0) const;
    void SetFloat(const std::string& key, float value);
    float GetFloat(const std::string& key, float defaultVal = 0.0f) const;
    void SetBool(const std::string& key, bool value);
    bool GetBool(const std::string& key, bool defaultVal = false) const;
    bool Has(const std::string& key) const;

    bool Save(const std::string& path);
    bool Load(const std::string& path);
    void Clear();

    /// Enable auto-save on program exit. Call Shutdown() to trigger save.
    void EnableAutoSave(const std::string& path);
    static void Shutdown();

    /// MRU (Most Recently Used) file list
    void AddRecentFile(const std::string& path, int max = 10);
    std::vector<std::string> GetRecentFiles() const;
    void ClearRecentFiles();

private:
    Settings() = default;
    std::unordered_map<std::string, std::string> data_;
    std::string autoSavePath_;
    static bool autoSaveEnabled_;
    static std::string autoSavePathStatic_;
};

} // namespace unigui
