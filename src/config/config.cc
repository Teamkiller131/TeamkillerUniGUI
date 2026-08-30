#include <unigui/config/config.h>
#include <unigui/core/log.h>
#include <unigui/core/strutil.h>

// Third-party parser backends — kept out of the public config.h so they do not leak
// into every consumer translation unit.
#include <cctype>
#include <cerrno>
#include <cpptoml.h>
#include <cstdint>
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

namespace {
// Escape a string as the body of a TOML basic string (no surrounding quotes).
std::string TomlEscape(const std::string& s) {
    std::string out;
    out.reserve(s.size() + 2);
    for (char c : s) {
        switch (c) {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

// A TOML key: emitted bare when it is a non-empty run of [A-Za-z0-9_-], otherwise as a
// quoted+escaped basic string. Keys with spaces/'='/'"'/newlines reach here via LoadINI
// (everything left of the first '=') and LoadJSON, and would otherwise produce
// unparseable output that fails the whole LoadTOML on round-trip.
std::string TomlKey(const std::string& k) {
    bool bare = !k.empty();
    for (char c : k) {
        if (!(std::isalnum(static_cast<unsigned char>(c)) || c == '_' || c == '-')) {
            bare = false;
            break;
        }
    }
    return bare ? k : ("\"" + TomlEscape(k) + "\"");
}
} // namespace

bool Store::SaveTOML(const std::string& path) const {
    FILE* f = fopen(path.c_str(), "w");
    if (!f)
        return false;
    for (auto& [k, v] : data_) {
        const std::string key = TomlKey(k);
        if (v == "true" || v == "false") {
            std::fprintf(f, "%s = %s\n", key.c_str(), v.c_str());
        } else {
            // Emit a value bare ONLY when it is a canonical integer that re-parses to the
            // IDENTICAL text — i.e. cpptoml reads it back as the same string. strtod's
            // grammar is far wider than TOML's bare-number grammar: it accepts "007", ".5",
            // "+5", "0x10", "1e9999" etc., which either make cpptoml THROW on reload (losing
            // the WHOLE file — `Store::Load*` wraps the parse in one try/catch) or silently
            // change the value. Floats also don't round-trip (LoadTOML re-stringifies a
            // double via std::to_string → "3.14" becomes "3.140000"). So bare emission is
            // restricted to integers whose std::to_string round-trips exactly; everything
            // else is quoted+escaped, which always reloads as the identical string.
            errno = 0;
            char* end = nullptr;
            const long long iv = std::strtoll(v.c_str(), &end, 10);
            const bool bareInt = !v.empty() && errno == 0 && *end == 0 && std::to_string(iv) == v;
            if (bareInt)
                std::fprintf(f, "%s = %s\n", key.c_str(), v.c_str());
            else
                std::fprintf(f, "%s = \"%s\"\n", key.c_str(), TomlEscape(v).c_str());
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
            // Integer first, at full int64 width with errno range-checking, so a large
            // but valid value (e.g. "3000000000") is not silently truncated to a wrong,
            // possibly negative int. LoadJSON round-trips integers as int64, so this keeps
            // Save/Load lossless. strtoll/strtod are the non-throwing C functions (the
            // banned forms are std::stoi/stoll/stod, which throw).
            errno = 0;
            char* end = nullptr;
            long long ival = std::strtoll(v.c_str(), &end, 10);
            if (errno == 0 && end != v.c_str() && *end == 0) {
                j[k] = static_cast<std::int64_t>(ival);
            } else {
                errno = 0;
                double dval = std::strtod(v.c_str(), &end);
                if (errno == 0 && end != v.c_str() && *end == 0)
                    j[k] = dval;
                else
                    j[k] = v; // overflow / non-numeric: preserve the original string verbatim
            }
        }
    }
    std::ofstream o(path);
    if (!o)
        return false; // open failed (bad dir / read-only / etc.)
    o << j.dump(2);
    return static_cast<bool>(o); // false if the write/flush failed (disk full / I/O error)
}

// ── INI ─────────────────────────────────────────────────────────────────────
Result<void> Store::LoadINI(const std::string& path) {
    FILE* f = fopen(path.c_str(), "r");
    if (!f)
        return Err(ErrorCode::FileNotFound);
    char buf[4096];
    std::string section; // current [section]; keys become "section.key"
    while (fgets(buf, sizeof(buf), f)) {
        std::string line(buf);
        while (!line.empty() && (line.back() == '\n' || line.back() == '\r'))
            line.pop_back();
        line = Trim(line);
        // Skip blanks and comment lines (';' or '#' — both common INI conventions).
        if (line.empty() || line[0] == ';' || line[0] == '#')
            continue;
        // [section] header: subsequent keys are stored as "section.key".
        if (line.front() == '[' && line.back() == ']') {
            section = Trim(line.substr(1, line.size() - 2));
            continue;
        }
        const auto eq = line.find('=');
        if (eq == std::string::npos)
            continue; // not a key=value line
        std::string key = Trim(line.substr(0, eq));
        std::string value = Trim(line.substr(eq + 1));
        if (key.empty())
            continue;
        if (!section.empty())
            key = section + "." + key;
        data_[key] = std::move(value);
    }
    fclose(f);
    UNIGUI_LOG_INFO("Store: loaded INI {} ({} keys)", path, (int) data_.size());
    return {};
}

} // namespace unigui::config
