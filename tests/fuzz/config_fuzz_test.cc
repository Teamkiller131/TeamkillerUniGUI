/// Fuzz targets for the config parsers (TOML / JSON / INI) — no GL context.
/// The loaders are file-based, so each input is written to a temp file and then
/// loaded; the property under test is that loading never crashes/throws,
/// regardless of how malformed the file is.

#include <unigui/config/config.h>

#include <atomic>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
#include <random>
#include <string>

namespace {

std::string RandomString(std::mt19937& rng, int maxLen) {
    std::uniform_int_distribution<int> lenDist(0, maxLen);
    std::uniform_int_distribution<int> chDist(0, 255);
    int len = lenDist(rng);
    std::string s(len, '\0');
    for (auto& c : s)
        c = static_cast<char>(chDist(rng));
    return s;
}

// RAII temp file seeded with arbitrary bytes; removed on destruction.
struct TempFile {
    std::filesystem::path path;
    explicit TempFile(const std::string& content) {
        static std::atomic<unsigned> counter{0};
        path = std::filesystem::temp_directory_path() /
               ("unigui_cfg_fuzz_" + std::to_string(counter++) + ".tmp");
        std::ofstream f(path, std::ios::binary);
        f.write(content.data(), static_cast<std::streamsize>(content.size()));
    }
    ~TempFile() {
        std::error_code ec;
        std::filesystem::remove(path, ec);
    }
    std::string str() const { return path.string(); }
};

} // namespace

TEST(FuzzConfig, RandomBytes_AllFormats_NoThrow) {
    std::mt19937 rng(99);
    for (int i = 0; i < 500; ++i) {
        std::string content = RandomString(rng, 512);
        TempFile tf(content);
        auto& store = unigui::config::Store::Instance();
        EXPECT_NO_THROW(store.LoadTOML(tf.str())) << "TOML iteration " << i;
        EXPECT_NO_THROW(store.LoadJSON(tf.str())) << "JSON iteration " << i;
        EXPECT_NO_THROW(store.LoadINI(tf.str())) << "INI iteration " << i;
    }
}

TEST(FuzzConfig, TomlLikeTokens_NoThrow) {
    static const char alphabet[] = "[]{}=\"'.,#-+ \n\ttruefalse0123456789keyvalue";
    std::mt19937 rng(11);
    std::uniform_int_distribution<int> lenDist(0, 256);
    std::uniform_int_distribution<int> idxDist(0, static_cast<int>(sizeof(alphabet) - 2));
    for (int i = 0; i < 500; ++i) {
        int len = lenDist(rng);
        std::string content;
        content.reserve(len);
        for (int j = 0; j < len; ++j)
            content.push_back(alphabet[idxDist(rng)]);
        TempFile tf(content);
        auto& store = unigui::config::Store::Instance();
        EXPECT_NO_THROW(store.LoadTOML(tf.str())) << "iteration " << i;
        EXPECT_NO_THROW(store.LoadINI(tf.str())) << "iteration " << i;
    }
}

TEST(FuzzConfig, EdgeCases_NoThrow) {
    auto loadAll = [](const std::string& content) {
        TempFile tf(content);
        auto& store = unigui::config::Store::Instance();
        EXPECT_NO_THROW(store.LoadTOML(tf.str()));
        EXPECT_NO_THROW(store.LoadJSON(tf.str()));
        EXPECT_NO_THROW(store.LoadINI(tf.str()));
    };
    loadAll("");                                     // empty
    loadAll("\n\n\n");                               // blank lines
    loadAll("=");                                    // INI: empty key and value
    loadAll("=value");                               // INI: empty key
    loadAll("key=");                                 // INI: empty value
    loadAll("key = \"unterminated");                 // unterminated string
    loadAll("[section\nkey = 1");                    // unclosed TOML table header
    loadAll("{ broken json ");                       // malformed JSON
    loadAll("{}");                                   // empty JSON object
    loadAll("[1, 2, 3]");                            // JSON array at top level
    loadAll("key = 999999999999999999999999999999"); // out-of-range number
    // Binary garbage
    std::string garbage(256, '\0');
    for (int i = 0; i < 256; ++i)
        garbage[i] = static_cast<char>(i);
    loadAll(garbage);
    // Loading a non-existent path must be safe and return false.
    auto& store = unigui::config::Store::Instance();
    EXPECT_NO_THROW(store.LoadTOML("/nonexistent/unigui_fuzz/does_not_exist.toml"));
    EXPECT_NO_THROW(store.LoadJSON("/nonexistent/unigui_fuzz/does_not_exist.json"));
    EXPECT_NO_THROW(store.LoadINI("/nonexistent/unigui_fuzz/does_not_exist.ini"));
}
