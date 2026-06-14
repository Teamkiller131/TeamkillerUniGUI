/// Fuzz target for the CSS style engine parser — no GL context required.
/// Feed random/malformed CSS and verify Engine::Parse never crashes/throws.

#include <unigui/styling/style_engine.h>

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

// Random string biased toward CSS punctuation so the parser's structural paths
// (braces, selectors, declarations, at-rules) get exercised, not just garbage.
std::string RandomCss(std::mt19937& rng, int maxLen) {
    static const char alphabet[] = "{}();:,.#@%-/* \n\twidth heightcolorhoverButton0123456789";
    std::uniform_int_distribution<int> lenDist(0, maxLen);
    std::uniform_int_distribution<int> idxDist(0, static_cast<int>(sizeof(alphabet) - 2));
    int len = lenDist(rng);
    std::string s;
    s.reserve(len);
    for (int i = 0; i < len; ++i)
        s.push_back(alphabet[idxDist(rng)]);
    return s;
}

} // namespace

TEST(FuzzCSS, RandomBytes_1000Iterations_NoThrow) {
    std::mt19937 rng(2024);
    auto& engine = unigui::styling::Engine::Instance();
    for (int i = 0; i < 1000; ++i) {
        std::string css = RandomString(rng, 512);
        EXPECT_NO_THROW(engine.Parse(css)) << "iteration " << i;
    }
}

TEST(FuzzCSS, CssLikeTokens_1000Iterations_NoThrow) {
    std::mt19937 rng(7);
    auto& engine = unigui::styling::Engine::Instance();
    for (int i = 0; i < 1000; ++i) {
        std::string css = RandomCss(rng, 512);
        EXPECT_NO_THROW(engine.Parse(css)) << "iteration " << i;
    }
}

TEST(FuzzCSS, EdgeCases_NoThrow) {
    auto& engine = unigui::styling::Engine::Instance();
    // Empty / whitespace only
    EXPECT_NO_THROW(engine.Parse(""));
    EXPECT_NO_THROW(engine.Parse("   \n\t  "));
    // Unbalanced braces
    EXPECT_NO_THROW(engine.Parse("Button {"));
    EXPECT_NO_THROW(engine.Parse("}"));
    EXPECT_NO_THROW(engine.Parse("Button { color: red"));
    EXPECT_NO_THROW(engine.Parse("{{{{{{"));
    EXPECT_NO_THROW(engine.Parse("}}}}}}"));
    // Declaration without value / colon
    EXPECT_NO_THROW(engine.Parse("Button { color }"));
    EXPECT_NO_THROW(engine.Parse("Button { : ; ; : }"));
    EXPECT_NO_THROW(engine.Parse("Button { color: ; }"));
    // Selectors with every supported decoration but no body
    EXPECT_NO_THROW(engine.Parse("Button.primary:hover#id"));
    EXPECT_NO_THROW(engine.Parse(".:#"));
    // At-rules / media
    EXPECT_NO_THROW(engine.Parse("@media (min-width: 800px) { Button { color: #fff } }"));
    EXPECT_NO_THROW(engine.Parse("@media {"));
    EXPECT_NO_THROW(engine.Parse("@"));
    // linear-gradient and var() value parsers
    EXPECT_NO_THROW(engine.Parse("Panel { background: linear-gradient(}"));
    EXPECT_NO_THROW(engine.Parse("Panel { color: var(--x) }"));
    EXPECT_NO_THROW(engine.Parse("Panel { color: var( }"));
    // Binary garbage embedded in a rule
    std::string garbage = "Button { color: ";
    for (int i = 0; i < 256; ++i)
        garbage.push_back(static_cast<char>(i));
    garbage += " }";
    EXPECT_NO_THROW(engine.Parse(garbage));
    // Very long declaration
    EXPECT_NO_THROW(engine.Parse("Button { color: " + std::string(100000, 'a') + " }"));
}
