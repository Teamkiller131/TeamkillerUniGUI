#include <unigui/core/settings.h>
#include <cstdio>
#include <cstdlib>
#include <algorithm>

namespace unigui {

namespace {
// strtol/strtof never throw, unlike std::stoi/std::stof — fall back to the
// caller's default on empty/garbage input instead of crashing.
int SafeToInt(const std::string& s, int def) {
    if (s.empty()) return def;
    char* end = nullptr;
    long v = std::strtol(s.c_str(), &end, 10);
    return end == s.c_str() ? def : static_cast<int>(v);
}
float SafeToFloat(const std::string& s, float def) {
    if (s.empty()) return def;
    char* end = nullptr;
    float v = std::strtof(s.c_str(), &end);
    return end == s.c_str() ? def : v;
}
} // namespace

bool Settings::autoSaveEnabled_ = false;
std::string Settings::autoSavePathStatic_;

Settings& Settings::Instance() { static Settings s; return s; }
void Settings::Set(const std::string& key, const std::string& value) { data_[key] = value; }
std::string Settings::Get(const std::string& key, const std::string& defaultVal) const {
    auto it = data_.find(key); return it != data_.end() ? it->second : defaultVal;
}
void Settings::SetInt(const std::string& key, int value) { Set(key, std::to_string(value)); }
int Settings::GetInt(const std::string& key, int defaultVal) const {
    auto it = data_.find(key); return it != data_.end() ? SafeToInt(it->second, defaultVal) : defaultVal;
}
void Settings::SetFloat(const std::string& key, float value) { Set(key, std::to_string(value)); }
float Settings::GetFloat(const std::string& key, float defaultVal) const {
    auto it = data_.find(key); return it != data_.end() ? SafeToFloat(it->second, defaultVal) : defaultVal;
}
void Settings::SetBool(const std::string& key, bool value) { Set(key, value ? "1" : "0"); }
bool Settings::GetBool(const std::string& key, bool defaultVal) const {
    auto it = data_.find(key); return it != data_.end() ? (it->second == "1" || it->second == "true") : defaultVal;
}
bool Settings::Has(const std::string& key) const { return data_.count(key) > 0; }
void Settings::Clear() { data_.clear(); }

bool Settings::Save(const std::string& path) {
    FILE* f = fopen(path.c_str(), "w"); if (!f) return false;
    for (auto& [k, v] : data_) std::fprintf(f, "%s=%s\n", k.c_str(), v.c_str());
    fclose(f); return true;
}
bool Settings::Load(const std::string& path) {
    FILE* f = fopen(path.c_str(), "r"); if (!f) return false;
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        std::string line(buf);
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
        auto eq = line.find('=');
        if (eq != std::string::npos) data_[line.substr(0, eq)] = line.substr(eq + 1);
    }
    fclose(f); return true;
}
void Settings::EnableAutoSave(const std::string& path) { autoSaveEnabled_ = true; autoSavePathStatic_ = path; }
void Settings::Shutdown() { if (autoSaveEnabled_) Instance().Save(autoSavePathStatic_); }

void Settings::AddRecentFile(const std::string& path, int max) {
    auto files = GetRecentFiles();
    files.erase(std::remove(files.begin(), files.end(), path), files.end());
    files.insert(files.begin(), path);
    if ((int)files.size() > max) files.resize(max);
    for (int i = 0; i < (int)files.size(); i++) data_["recent." + std::to_string(i)] = files[i];
}
std::vector<std::string> Settings::GetRecentFiles() const {
    std::vector<std::string> files;
    for (int i = 0; ; i++) {
        auto it = data_.find("recent." + std::to_string(i));
        if (it == data_.end()) break;
        files.push_back(it->second);
    }
    return files;
}
void Settings::ClearRecentFiles() {
    for (int i = 99; i >= 0; i--) data_.erase("recent." + std::to_string(i));
}

} // namespace unigui
