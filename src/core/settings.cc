#include <unigui/core/settings.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <algorithm>

namespace unigui {

Settings& Settings::Instance() {
    static Settings s;
    return s;
}

void Settings::Set(const std::string& key, const std::string& value) { data_[key] = value; }
std::string Settings::Get(const std::string& key, const std::string& defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? it->second : defaultVal;
}

void Settings::SetInt(const std::string& key, int value) { Set(key, std::to_string(value)); }
int Settings::GetInt(const std::string& key, int defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? std::stoi(it->second) : defaultVal;
}

void Settings::SetFloat(const std::string& key, float value) { Set(key, std::to_string(value)); }
float Settings::GetFloat(const std::string& key, float defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? std::stof(it->second) : defaultVal;
}

void Settings::SetBool(const std::string& key, bool value) { Set(key, value ? "1" : "0"); }
bool Settings::GetBool(const std::string& key, bool defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? (it->second == "1" || it->second == "true") : defaultVal;
}

bool Settings::Has(const std::string& key) const { return data_.count(key) > 0; }
void Settings::Clear() { data_.clear(); }

bool Settings::Save(const std::string& path) {
    FILE* f = fopen(path.c_str(), "w");
    if (!f) return false;
    for (auto& [k, v] : data_) std::fprintf(f, "%s=%s\n", k.c_str(), v.c_str());
    fclose(f); return true;
}

bool Settings::Load(const std::string& path) {
    FILE* f = fopen(path.c_str(), "r");
    if (!f) return false;
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        std::string line(buf);
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r')) line.pop_back();
        auto eq = line.find('=');
        if (eq != std::string::npos) data_[line.substr(0, eq)] = line.substr(eq + 1);
    }
    fclose(f); return true;
}

} // namespace unigui
