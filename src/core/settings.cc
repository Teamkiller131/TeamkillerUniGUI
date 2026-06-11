#include <unigui/core/settings.h>
#include <unigui/core/strutil.h>

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>

#ifdef _WIN32
#include <filesystem>
#include <windows.h>
namespace {
std::wstring PathToWide(const std::string& s) {
    // std::filesystem::path handles the native encoding correctly on Windows
    return std::filesystem::path(s).wstring();
}
} // namespace
#endif

namespace unigui {

namespace {

std::string EscapeSetting(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (char ch : s) {
        switch (ch) {
        case '\\': out += "\\\\"; break;
        case '\n': out += "\\n"; break;
        case '\r': out += "\\r"; break;
        case '=':  out += "\\="; break;
        default:   out += ch; break;
        }
    }
    return out;
}

std::string UnescapeSetting(const std::string& s) {
    std::string out;
    out.reserve(s.size());
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            switch (s[i + 1]) {
            case '\\': out += '\\'; ++i; break;
            case 'n':  out += '\n'; ++i; break;
            case 'r':  out += '\r'; ++i; break;
            case '=':  out += '=';  ++i; break;
            default:   out += s[i]; break;
            }
        } else {
            out += s[i];
        }
    }
    return out;
}

// Find the first '=' that is NOT escaped as "\=" — i.e. the real key/value
// separator. Scanning while honoring escapes is required because EscapeSetting
// turns any '=' inside a key or value into "\=".
size_t FindSeparator(const std::string& line) {
    for (size_t i = 0; i < line.size(); ++i) {
        if (line[i] == '\\') {
            ++i; // skip the escaped character
            continue;
        }
        if (line[i] == '=')
            return i;
    }
    return std::string::npos;
}
} // namespace

bool Settings::autoSaveEnabled_ = false;
std::string Settings::autoSavePathStatic_;

Settings& Settings::Instance() {
    static Settings s;
    return s;
}
void Settings::Set(const std::string& key, const std::string& value) {
    data_[key] = value;
}
std::string Settings::Get(const std::string& key, const std::string& defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? it->second : defaultVal;
}
void Settings::SetInt(const std::string& key, int value) {
    Set(key, std::to_string(value));
}
int Settings::GetInt(const std::string& key, int defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? ToIntOr(it->second, defaultVal) : defaultVal;
}
void Settings::SetFloat(const std::string& key, float value) {
    Set(key, std::to_string(value));
}
float Settings::GetFloat(const std::string& key, float defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? ToFloatOr(it->second, defaultVal) : defaultVal;
}
void Settings::SetBool(const std::string& key, bool value) {
    Set(key, value ? "1" : "0");
}
bool Settings::GetBool(const std::string& key, bool defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? (it->second == "1" || it->second == "true") : defaultVal;
}
bool Settings::Has(const std::string& key) const {
    return data_.count(key) > 0;
}
void Settings::Erase(const std::string& key) {
    data_.erase(key);
}
std::vector<std::string> Settings::Keys(const std::string& prefix) const {
    std::vector<std::string> result;
    for (auto& [k, _] : data_) {
        if (prefix.empty() || k.compare(0, prefix.size(), prefix) == 0)
            result.push_back(k);
    }
    return result;
}
void Settings::Clear() {
    data_.clear();
}

bool Settings::Save(const std::string& path) {
    std::string tmpPath = path + ".tmp";
#ifdef _WIN32
    auto wtmp = PathToWide(tmpPath);
    auto wpath = PathToWide(path);
    FILE* f = _wfopen(wtmp.c_str(), L"w");
#else
    FILE* f = fopen(tmpPath.c_str(), "w");
#endif
    if (!f)
        return false;
    for (auto& [k, v] : data_)
        std::fprintf(f, "%s=%s\n", EscapeSetting(k).c_str(), EscapeSetting(v).c_str());
    fclose(f);
#ifdef _WIN32
    if (!MoveFileExW(wtmp.c_str(), wpath.c_str(), MOVEFILE_REPLACE_EXISTING)) {
        _wremove(wtmp.c_str());
        return false;
    }
#else
    if (std::rename(tmpPath.c_str(), path.c_str()) != 0) {
        std::remove(tmpPath.c_str());
        return false;
    }
#endif
    return true;
}
bool Settings::Load(const std::string& path) {
    data_.clear();
#ifdef _WIN32
    auto wpath = PathToWide(path);
    FILE* f = _wfopen(wpath.c_str(), L"r");
#else
    FILE* f = fopen(path.c_str(), "r");
#endif
    if (!f)
        return false;
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        std::string line(buf);
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n'))
            line.pop_back();
        if (line.empty() || line[0] == '#' || line[0] == ';')
            continue;
        auto eq = FindSeparator(line);
        if (eq == std::string::npos)
            continue;
        std::string key = UnescapeSetting(line.substr(0, eq));
        TrimInPlace(key);
        if (key.empty())
            continue;
        data_[key] = UnescapeSetting(line.substr(eq + 1));
    }
    fclose(f);
    return true;
}
void Settings::EnableAutoSave(const std::string& path) {
    autoSaveEnabled_ = true;
    autoSavePathStatic_ = path;
}
void Settings::Shutdown() {
    if (autoSaveEnabled_)
        Instance().Save(autoSavePathStatic_);
}

void Settings::AddRecentFile(const std::string& path, int max) {
    auto files = GetRecentFiles();
    files.erase(std::remove(files.begin(), files.end(), path), files.end());
    files.insert(files.begin(), path);
    if ((int) files.size() > max)
        files.resize(max);
    for (int i = 0; i < (int) files.size(); i++)
        data_["recent." + std::to_string(i)] = files[i];
}
std::vector<std::string> Settings::GetRecentFiles() const {
    std::vector<std::string> files;
    for (int i = 0;; i++) {
        auto it = data_.find("recent." + std::to_string(i));
        if (it == data_.end())
            break;
        files.push_back(it->second);
    }
    return files;
}
void Settings::ClearRecentFiles() {
    for (auto& k : Keys("recent."))
        data_.erase(k);
}

} // namespace unigui
