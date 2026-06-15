#include <unigui/core/locale.h>

#include <cerrno>
#include <cstddef>
#include <cstdio>
#include <string_view>
#include <vector>

namespace unigui {

std::string Locale::current_ = "en_US";
std::string Locale::fallback_ = "en_US";

std::unordered_map<std::string, std::unordered_map<std::string, std::string>>& Locale::Table() {
    static std::unordered_map<std::string, std::unordered_map<std::string, std::string>> table;
    return table;
}

void Locale::SetCurrent(const std::string& locale) {
    current_ = locale;
}
const std::string& Locale::GetCurrent() {
    return current_;
}

void Locale::SetFallback(const std::string& locale) {
    fallback_ = locale;
}
const std::string& Locale::GetFallback() {
    return fallback_;
}

void Locale::Set(const std::string& locale, const std::string& key, const std::string& value) {
    Table()[locale][key] = value;
}

std::string Locale::BaseLanguage(const std::string& locale) {
    auto sep = locale.find_first_of("_-");
    return sep != std::string::npos ? locale.substr(0, sep) : std::string();
}

bool Locale::Lookup(const std::string& locale, const std::string& key, std::string& out) {
    if (locale.empty())
        return false;
    auto& table = Table();
    auto it = table.find(locale);
    if (it == table.end())
        return false;
    auto kit = it->second.find(key);
    if (kit == it->second.end())
        return false;
    out = kit->second;
    return true;
}

std::string Locale::Tr(const std::string& key) {
    std::string out;
    // current → base language of current → fallback → base language of fallback.
    if (Lookup(current_, key, out))
        return out;
    if (Lookup(BaseLanguage(current_), key, out))
        return out;
    if (Lookup(fallback_, key, out))
        return out;
    if (Lookup(BaseLanguage(fallback_), key, out))
        return out;
    return key;
}

std::string Locale::Tr(const std::string& key, const std::vector<std::string>& args) {
    std::string s = Tr(key);
    for (std::size_t i = 0; i < args.size(); ++i) {
        const std::string token = "{" + std::to_string(i) + "}";
        for (std::size_t pos = s.find(token); pos != std::string::npos;
             pos = s.find(token, pos + args[i].size())) {
            s.replace(pos, token.size(), args[i]);
        }
    }
    return s;
}

bool Locale::Has(const std::string& key) {
    std::string out;
    return Lookup(current_, key, out);
}

bool Locale::IsRTL() {
    return IsRTL(current_);
}

bool Locale::IsRTL(const std::string& locale) {
    const std::string base = BaseLanguage(locale);
    const std::string lang = base.empty() ? locale : base;
    return lang == "ar" || lang == "he" || lang == "fa" || lang == "ur";
}

void Locale::Clear() {
    Table().clear();
}

bool Locale::LoadFromFile(const std::string& path) {
    FILE* f = fopen(path.c_str(), "r");
    if (!f)
        return false;
    fseek(f, 0, SEEK_END);
    long sz = ftell(f);
    fseek(f, 0, SEEK_SET);
    if (sz <= 0) {
        fclose(f);
        return false;
    }
    std::string json(sz, '\0');
    fread(&json[0], 1, sz, f);
    fclose(f);

    // Determine locale from filename: "zh_CN.json" -> "zh_CN"
    std::string locale = "en_US";
    auto lastSlash = path.find_last_of("/\\");
    auto base = (lastSlash != std::string::npos) ? path.substr(lastSlash + 1) : path;
    auto dot = base.find('.');
    if (dot != std::string::npos)
        locale = base.substr(0, dot);

    // Parse JSON {"key":"value","key2":"value2"}.
    //
    // Hand-written scanner rather than std::regex: MSVC's std::regex enforces a
    // backtracking-complexity governor (libstdc++/libc++ do not), so a large or
    // malformed locale file made the old pattern throw std::regex_error and the
    // load crash on Windows. This linear scan never throws on bad input — it
    // simply stops once no further "key":"value" pair is found.
    std::string_view sv(json);
    std::size_t p = 0;
    while (true) {
        // Opening quote of the key.
        const std::size_t kq = sv.find('"', p);
        if (kq == std::string_view::npos)
            break;
        const std::size_t ke = sv.find('"', kq + 1);
        if (ke == std::string_view::npos)
            break;
        std::string_view k = sv.substr(kq + 1, ke - kq - 1);

        // ':' separator (allow surrounding spaces/tabs).
        std::size_t c = ke + 1;
        while (c < sv.size() && (sv[c] == ' ' || sv[c] == '\t' || sv[c] == '\n' || sv[c] == '\r'))
            ++c;
        if (c >= sv.size() || sv[c] != ':') {
            // Not a "key":… pair — resume scanning after the key's close quote.
            p = ke + 1;
            continue;
        }
        ++c;
        while (c < sv.size() && (sv[c] == ' ' || sv[c] == '\t' || sv[c] == '\n' || sv[c] == '\r'))
            ++c;
        // Opening quote of the value.
        if (c >= sv.size() || sv[c] != '"') {
            p = ke + 1;
            continue;
        }
        const std::size_t vq = c;
        const std::size_t ve = sv.find('"', vq + 1);
        if (ve == std::string_view::npos)
            break;
        std::string_view v = sv.substr(vq + 1, ve - vq - 1);

        // A key must be non-empty to match the old ([^"]+) class.
        if (!k.empty())
            Set(locale, std::string(k), std::string(v));
        p = ve + 1;
    }
    return true;
}

void Locale::LoadBuiltin() {
    // English (default — keys are already English)
    Set("en_US", "app.title", "UniGUI Application");
    Set("en_US", "menu.file", "File");
    Set("en_US", "menu.edit", "Edit");
    Set("en_US", "menu.help", "Help");
    Set("en_US", "menu.exit", "Exit");
    Set("en_US", "btn.ok", "OK");
    Set("en_US", "btn.cancel", "Cancel");
    Set("en_US", "btn.apply", "Apply");
    Set("en_US", "btn.close", "Close");

    // Chinese
    Set("zh_CN", "app.title", "UniGUI 应用程序");
    Set("zh_CN", "menu.file", "文件");
    Set("zh_CN", "menu.edit", "编辑");
    Set("zh_CN", "menu.help", "帮助");
    Set("zh_CN", "menu.exit", "退出");
    Set("zh_CN", "btn.ok", "确定");
    Set("zh_CN", "btn.cancel", "取消");
    Set("zh_CN", "btn.apply", "应用");
    Set("zh_CN", "btn.close", "关闭");

    // Japanese
    Set("ja_JP", "app.title", "UniGUI アプリケーション");
    Set("ja_JP", "menu.file", "ファイル");
    Set("ja_JP", "menu.edit", "編集");
    Set("ja_JP", "menu.help", "ヘルプ");
    Set("ja_JP", "menu.exit", "終了");
    Set("ja_JP", "btn.ok", "OK");
    Set("ja_JP", "btn.cancel", "キャンセル");
}

std::vector<std::string> Locale::GetLocales() {
    std::vector<std::string> locales;
    for (auto& [code, _] : Table())
        locales.push_back(code);
    return locales;
}

} // namespace unigui
