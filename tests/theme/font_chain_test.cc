// Font-family / font-chain tests for the theme layer (S0/D4–D5).
//
// Covers: configurable body/heading/mono families, role fallbacks, DPI- and
// font-size-driven atlas rebuilds preserving the configured chain, explicit
// fallback when a configured family is missing, and the no-font-environment
// fallback report.

#include <unigui/theme/theme.h>

#include <imgui.h>

#include <string>

#include <gtest/gtest.h>

using namespace unigui;

namespace {
constexpr const char* kRepoBodyFont = UNIGUI_TEST_FONT_FILE; // JetBrains Mono NF
constexpr const char* kRepoCJCFont = UNIGUI_TEST_CJK_FONT_FILE; // OFL CJK subset
constexpr const char* kMissingFont = "Z:/unigui/no-such-font-file.ttf";

class FontChainTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1280.0f, 720.0f);
    }
    void TearDown() override { ImGui::DestroyContext(); }
};
} // namespace

TEST_F(FontChainTest, DefaultChain_EmbeddedBodyWithRoleFallbacks) {
    ConfigureFontChain(FontChainConfig{}); // all roles default
    LoadDefaultFont(16.0f, nullptr);

    ImFont* body = GetBodyFont();
    ASSERT_NE(body, nullptr);
    EXPECT_EQ(GetHeadingFont(), body); // heading defaults to body
    EXPECT_EQ(GetMonoFont(), body);    // mono defaults to body (it is monospace)
    EXPECT_TRUE(GetFontChain().body.empty());

    std::string report = GetFontFallbackReport();
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("body=loaded"), std::string::npos) << report;
}

TEST_F(FontChainTest, ConfiguredFamilies_LoadDedicatedFonts) {
    FontChainConfig chain;
    chain.body = kRepoCJCFont;
    chain.heading = kRepoBodyFont;
    chain.mono = kRepoBodyFont;
    chain.cjk = kRepoCJCFont;
    ConfigureFontChain(chain);
    LoadDefaultFont(18.0f, nullptr);

    ImFont* body = GetBodyFont();
    ASSERT_NE(body, nullptr);
    ImFont* heading = GetHeadingFont();
    ImFont* mono = GetMonoFont();
    ASSERT_NE(heading, nullptr);
    ASSERT_NE(mono, nullptr);
    EXPECT_NE(heading, body); // dedicated heading family, not the body fallback
    EXPECT_NE(mono, body);    // dedicated mono family, not the body fallback
}

TEST_F(FontChainTest, DpiFontSizeRebuild_PreservesConfiguredFamilies) {
    ThemeConfig config;
    config.font_size = 16.0f;
    config.body_font_path = kRepoCJCFont;
    config.heading_font_path = kRepoBodyFont;
    config.mono_font_path = kRepoBodyFont;
    config.font_path = kRepoCJCFont;
    ApplyTheme(config);
    LoadDefaultFont(16.0f, nullptr);
    ASSERT_NE(GetBodyFont(), nullptr);
    ImFont* heading16 = GetHeadingFont();
    ASSERT_NE(heading16, nullptr);
    EXPECT_NE(heading16, GetBodyFont());

    // A font-size change (e.g. DPI change) queues a rebuild; the rebuild must use
    // the configured families, not silently drop back to embedded defaults.
    config.font_size = 24.0f;
    ApplyTheme(config);
    ASSERT_TRUE(HasPendingFontRebuild());
    ApplyPendingFontRebuild();
    EXPECT_FALSE(HasPendingFontRebuild());

    EXPECT_EQ(GetFontChain().body, std::string(kRepoCJCFont));
    EXPECT_EQ(GetFontChain().heading, std::string(kRepoBodyFont));
    ASSERT_NE(GetBodyFont(), nullptr);
    ImFont* heading24 = GetHeadingFont();
    ASSERT_NE(heading24, nullptr);
    EXPECT_NE(heading24, GetBodyFont());
}

TEST_F(FontChainTest, InvalidBodyFont_FallsBackToEmbedded_AndReports) {
    FontChainConfig chain;
    chain.body = kMissingFont;
    ConfigureFontChain(chain);
    LoadDefaultFont(16.0f, nullptr);

    ASSERT_NE(GetBodyFont(), nullptr); // embedded JetBrains Mono NF took over
    std::string report = GetFontFallbackReport();
    EXPECT_FALSE(report.empty());
    EXPECT_NE(report.find("no-such-font-file.ttf"), std::string::npos) << report;
    EXPECT_NE(report.find("family errors"), std::string::npos) << report;
}

TEST_F(FontChainTest, InvalidHeadingFont_FallsBackToBody_AndReports) {
    FontChainConfig chain;
    chain.heading = kMissingFont;
    ConfigureFontChain(chain);
    LoadDefaultFont(16.0f, nullptr);

    ImFont* body = GetBodyFont();
    ASSERT_NE(body, nullptr);
    EXPECT_EQ(GetHeadingFont(), body); // explicit fallback to body
    std::string report = GetFontFallbackReport();
    EXPECT_NE(report.find("heading"), std::string::npos) << report;
}

TEST_F(FontChainTest, FallbackReport_NamesCJKSourceOrGap) {
    ConfigureFontChain(FontChainConfig{});
    LoadDefaultFont(16.0f, nullptr);

    std::string report = GetFontFallbackReport();
    EXPECT_NE(report.find("cjk="), std::string::npos) << report;
    // Either an embedded/system CJK font was found, or the report says CJK text
    // will render as tofu — never a silent gap.
    const bool resolved = report.find("cjk=embedded:") != std::string::npos ||
                          report.find("cjk=system:") != std::string::npos ||
                          report.find("cjk=explicit:") != std::string::npos;
    const bool reportedGap = report.find("FALLBACK ONLY") != std::string::npos ||
                             report.find("cjk error") != std::string::npos;
    EXPECT_TRUE(resolved || reportedGap) << report;
}
