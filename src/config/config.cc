#include <unigui/config/config.h>
#include <unigui/core/log.h>
#include <unigui/core/strutil.h>

// Third-party parser backends — kept out of the public config.h so they do not leak
// into every consumer translation unit.
#include <cpptoml.h>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <nlohmann/json.hpp>
#include <sstream>

namespace unigui::config {

Store& Store::Instance() {
    static Store c;
    return c;
}

// ── Internal ────────────────────────────────────────────────────────────────
void Store::SetValue(const std::string& key, const std::string& value) {
    data_[key] = value;
}
std::string Store::GetValue(const std::string& key, const std::string& defaultVal) const {
    auto it = data_.find(key);
    return it != data_.end() ? it->second : defaultVal;
}
void Store::Clear() {
    data_.clear();
}
bool Store::Has(const std::string& key) const {
    return data_.count(key) > 0;
}
std::vector<std::string> Store::Keys() const {
    std::vector<std::string> ks;
    for (auto& [k, _] : data_)
        ks.push_back(k);
    return ks;
}
void Store::Merge(const Store& other) {
    for (auto& [k, v] : other.data_)
        data_[k] = v;
}

// ── Typed access ────────────────────────────────────────────────────────────
std::string Store::GetString(const std::string& k, const std::string& d) const {
    return GetValue(k, d);
}
void Store::SetString(const std::string& k, const std::string& v) {
    SetValue(k, v);
}
int Store::GetInt(const std::string& k, int d) const {
    return ToIntOr(GetValue(k), d);
}
void Store::SetInt(const std::string& k, int v) {
    SetValue(k, std::to_string(v));
}
double Store::GetDouble(const std::string& k, double d) const {
    return ToDoubleOr(GetValue(k), d);
}
void Store::SetDouble(const std::string& k, double v) {
    SetValue(k, std::to_string(v));
}
bool Store::GetBool(const std::string& k, bool d) const {
    auto v = GetValue(k);
    return v.empty() ? d : (v == "true" || v == "1");
}
void Store::SetBool(const std::string& k, bool v) {
    SetValue(k, v ? "true" : "false");
}

// ── TOML ────────────────────────────────────────────────────────────────────
Result<void> Store::LoadTOML(const std::string& path) {
    {
        std::ifstream probe(path);
        if (!probe)
            return Err(ErrorCode::FileNotFound);
    }
    try {
        auto tbl = cpptoml::parse_file(path);
        for (auto it = tbl->begin(); it != tbl->end(); ++it) {
            std::string key = it->first;
            if (auto v = it->second->as<std::string>())
                SetValue(key, v->get());
            else if (auto v = it->second->as<int64_t>())
                SetValue(key, std::to_string(v->get()));
            else if (auto v = it->second->as<double>())
                SetValue(key, std::to_string(v->get()));
            else if (auto v = it->second->as<bool>())
                SetValue(key, v->get() ? "true" : "false");
        }
        UNIGUI_LOG_INFO("Store: loaded TOML {} ({} keys)", path, (int) data_.size());
        return {};
    } catch (...) {
        UNIGUI_LOG_WARN("Store: TOML parse failed: {}", path);
        return Err(ErrorCode::ParseFailed);
    }
}

bool Store::SaveTOML(const std::string& path) const {
    FILE* f = fopen(path.c_str(), "w");
    if (!f)
        return false;
    for (auto& [k, v] : data_) {
        if (v == "true" || v == "false")
            std::fprintf(f, "%s = %s\n", k.c_str(), v.c_str());
        else {
            char* end;
            std::strtod(v.c_str(), &end);
            if (*end == 0)
                std::fprintf(f, "%s = %s\n", k.c_str(), v.c_str());
            else
                std::fprintf(f, "%s = \"%s\"\n", k.c_str(), v.c_str());
        }
    }
    fclose(f);
    return true;
}

// ── JSON ────────────────────────────────────────────────────────────────────
Result<void> Store::LoadJSON(const std::string& path) {
    std::ifstream f(path);
    if (!f)
        return Err(ErrorCode::FileNotFound);
    try {
        nlohmann::json j;
        f >> j;
        for (auto& [k, v] : j.items()) {
            if (v.is_string())
                SetValue(k, v.get<std::string>());
            else if (v.is_number_integer())
                SetValue(k, std::to_string(v.get<int64_t>()));
            else if (v.is_number_float())
                SetValue(k, std::to_string(v.get<double>()));
            else if (v.is_boolean())
                SetValue(k, v.get<bool>() ? "true" : "false");
        }
        UNIGUI_LOG_INFO("Store: loaded JSON {} ({} keys)", path, (int) data_.size());
        return {};
    } catch (...) {
        UNIGUI_LOG_WARN("Store: JSON parse failed: {}", path);
        return Err(ErrorCode::ParseFailed);
    }
}

bool Store::SaveJSON(const std::string& path) const {
    nlohmann::json j;
    for (auto& [k, v] : data_) {
        if (v == "true")
            j[k] = true;
        else if (v == "false")
            j[k] = false;
        else {
            char* end = nullptr;
            long ival = std::strtol(v.c_str(), &end, 10);
            if (end != v.c_str() && *end == 0) {
                j[k] = static_cast<int>(ival);
            } else {
                double dval = std::strtod(v.c_str(), &end);
                if (end != v.c_str() && *end == 0)
                    j[k] = dval;
                else
                    j[k] = v;
            }
        }
    }
    std::ofstream o(path);
    o << j.dump(2);
    return true;
}

// ── INI ─────────────────────────────────────────────────────────────────────
Result<void> Store::LoadINI(const std::string& path) {
    FILE* f = fopen(path.c_str(), "r");
    if (!f)
        return Err(ErrorCode::FileNotFound);
    char buf[4096];
    while (fgets(buf, sizeof(buf), f)) {
        std::string line(buf);
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
            line.pop_back();
        auto eq = line.find('=');
        if (eq != std::string::npos)
            data_[line.substr(0, eq)] = line.substr(eq + 1);
    }
    fclose(f);
    UNIGUI_LOG_INFO("Store: loaded INI {} ({} keys)", path, (int) data_.size());
    return {};
}

} // namespace unigui::config
