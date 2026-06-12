/// Fuzz targets for CSV and JSON parsers — no GL context required.
/// Feed random/malformed input and verify no crash/UB.

#include <unigui/widgets/form.h>
#include <unigui/widgets/table.h>

#include <gtest/gtest.h>
#include <random>
#include <string>
#include <vector>

namespace {

std::string RandomString(std::mt19937& rng, int maxLen) {
    std::uniform_int_distribution<int> lenDist(0, maxLen);
    std::uniform_int_distribution<int> chDist(0, 255);
    int len = lenDist(rng);
    std::string s(len, '\0');
    for (auto& c : s)
        static_cast<char&>(c) = static_cast<char>(chDist(rng));
    return s;
}

} // namespace

// ── CSV Import fuzz ──────────────────────────────────────────────────────────

TEST(FuzzCSV, RandomInput_1000Iterations_NoCrash) {
    std::mt19937 rng(42);
    unigui::Table tbl("fuzz_csv", {"A", "B", "C"});

    for (int i = 0; i < 1000; i++) {
        std::string csv = RandomString(rng, 512);
        EXPECT_NO_THROW(tbl.ImportCSV(csv)) << "iteration " << i;
    }
}

TEST(FuzzCSV, EdgeCases_NoCrash) {
    unigui::Table tbl("fuzz_csv2", {"X"});
    // Empty
    EXPECT_NO_THROW(tbl.ImportCSV(""));
    // Only header
    EXPECT_NO_THROW(tbl.ImportCSV("A,B,C\n"));
    // Unclosed quote
    EXPECT_NO_THROW(tbl.ImportCSV("A,B\n\"unclosed\n"));
    // Deeply nested quotes
    EXPECT_NO_THROW(tbl.ImportCSV("A,B\n\"\"\"\"\"\"\",x\n"));
    // Binary garbage
    std::string garbage(256, '\0');
    for (int i = 0; i < 256; i++)
        garbage[i] = static_cast<char>(i);
    EXPECT_NO_THROW(tbl.ImportCSV(garbage));
    // Very long line
    std::string longLine(100000, 'x');
    EXPECT_NO_THROW(tbl.ImportCSV(longLine));
}

// ── Form Deserialize (JSON) fuzz ─────────────────────────────────────────────

TEST(FuzzJSON, RandomInput_1000Iterations_NoCrash) {
    std::mt19937 rng(123);
    unigui::Form form("fuzz_form", "Fuzz");
    form.AddTextField("name", "Name");
    form.AddTextField("value", "Value");
    form.AddCheckbox("flag", "Flag");

    for (int i = 0; i < 1000; i++) {
        std::string json = RandomString(rng, 512);
        EXPECT_NO_THROW(form.Deserialize(json)) << "iteration " << i;
    }
}

TEST(FuzzJSON, EdgeCases_NoCrash) {
    unigui::Form form("fuzz_form2", "Fuzz2");
    form.AddTextField("k", "V");
    // Empty
    EXPECT_NO_THROW(form.Deserialize(""));
    // Just braces
    EXPECT_NO_THROW(form.Deserialize("{}"));
    EXPECT_NO_THROW(form.Deserialize("{"));
    EXPECT_NO_THROW(form.Deserialize("}"));
    // Malformed JSON
    EXPECT_NO_THROW(form.Deserialize("{key: value}"));
    EXPECT_NO_THROW(form.Deserialize("{\"k\": }"));
    EXPECT_NO_THROW(form.Deserialize("{\"k\": \"v\",}"));
    // Unclosed string
    EXPECT_NO_THROW(form.Deserialize("{\"k\": \"unclosed"));
    // Binary garbage
    std::string garbage(256, '\0');
    for (int i = 0; i < 256; i++)
        garbage[i] = static_cast<char>(i);
    EXPECT_NO_THROW(form.Deserialize(garbage));
    // Deeply nested
    EXPECT_NO_THROW(form.Deserialize("{\"k\": \"\\\"\\\\\\n\\t\"}"));
}
