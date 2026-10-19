#include <unigui/fonts/font_manager.h>

#include <imgui.h>

#include <algorithm>
#include <cstdlib>
#include <string>

#include <gtest/gtest.h>
using namespace unigui::fonts;

// ─────────────────────────────────────────────────────────────────────────────
// Font manager tests: registry basics, injectable probe inputs, ordered
// resolution (explicit → embedded → system) and the explicit no-font-environment
// fallback report. The CJK/body fixtures are repo fonts compiled in as
// UNIGUI_TEST_CJK_FONT_FILE / UNIGUI_TEST_FONT_FILE (see tests/CMakeLists.txt).
// ─────────────────────────────────────────────────────────────────────────────

namespace {
constexpr const char* kRepoBodyFont = UNIGUI_TEST_FONT_FILE;
constexpr const char* kRepoCJCFont = UNIGUI_TEST_CJK_FONT_FILE;
constexpr const char* kMissingFont = "Z:/unigui/no-such-font-file.ttf";

void ClearFontEnv() {
#ifdef _WIN32
    _putenv_s("UNIGUI_CJK_FONT", "");
    _putenv_s("UNIGUI_EMOJI_FONT", "");
    _putenv_s("UNIGUI_FONT_DIRS", "");
#else
    unsetenv("UNIGUI_CJK_FONT");
    unsetenv("UNIGUI_EMOJI_FONT");
    unsetenv("UNIGUI_FONT_DIRS");
#endif
}

bool Contains(const std::vector<std::string>& values, const std::string& needle) {
    return std::find(values.begin(), values.end(), needle) != values.end();
}
} // namespace

class FontTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ClearFontEnv();
        // Each test looks at a freshly-configured manager: undo whatever the
        // previous test (same process) injected into the per-context singleton.
        auto& m = Manager::Instance();
        m.Reset();
        m.ClearSearchPaths();
        m.SetCJKPath("");
        m.SetEmojiPath("");
        m.SetSystemProbeEnabled(true);
        m.SetEmbeddedFallbackEnabled(true);
    }
    void TearDown() override {
        Manager::Instance().Reset();
        ImGui::DestroyContext();
    }
};

TEST_F(FontTest, List_EmptyByDefault) {
    EXPECT_TRUE(Manager::Instance().List().empty());
}

TEST_F(FontTest, Get_Nonexistent_ReturnsNull) {
    EXPECT_EQ(Manager::Instance().Get("nonexistent"), nullptr);
}

TEST_F(FontTest, Unload_Nonexistent_ReturnsFalse) {
    EXPECT_FALSE(Manager::Instance().Unload("nope"));
}

TEST_F(FontTest, SetDefault_DoesNotCrash) {
    Manager::Instance().SetDefault("nonexistent"); // should not crash
    SUCCEED();
}

// ── Injectable probe inputs ──────────────────────────────────────────────────

TEST_F(FontTest, SearchPaths_Roundtrip) {
    auto& m = Manager::Instance();
    EXPECT_TRUE(m.SearchPaths().empty());

    m.AddSearchPath("D:/fonts/a");
    m.AddSearchPath("D:/fonts/b");
    ASSERT_EQ(m.SearchPaths().size(), 2u);
    EXPECT_EQ(m.SearchPaths()[0], "D:/fonts/a");
    EXPECT_EQ(m.SearchPaths()[1], "D:/fonts/b");

    m.SetSearchPaths({"E:/only"});
    ASSERT_EQ(m.SearchPaths().size(), 1u);
    EXPECT_EQ(m.SearchPaths()[0], "E:/only");

    m.ClearSearchPaths();
    EXPECT_TRUE(m.SearchPaths().empty());
}

TEST_F(FontTest, CJKProbePlan_ExplicitFirst_SystemCandidatesLast) {
    auto& m = Manager::Instance();
    m.SetCJKPath(kRepoCJCFont);
    m.SetSystemProbeEnabled(true);
    m.SetEmbeddedFallbackEnabled(true);

    std::vector<std::string> plan = m.CJKProbePlan();
    ASSERT_FALSE(plan.empty());
    EXPECT_EQ(plan.front(), std::string(kRepoCJCFont));

    auto indexOf = [&](const std::string& needle) {
        for (std::size_t i = 0; i < plan.size(); ++i) {
            if (plan[i].find(needle) != std::string::npos)
                return static_cast<int>(i);
        }
        return -1;
    };

    const int embeddedIdx = indexOf(kEmbeddedCJKAssetId);
    if (m.HasEmbeddedCJKFont())
        EXPECT_GE(embeddedIdx, 0);

#ifdef _WIN32
    // System probing comes last: %WINDIR%-derived Microsoft YaHei candidates.
    const int msyhIdx = indexOf("msyh.ttc");
    EXPECT_GE(msyhIdx, 0);
    EXPECT_GT(msyhIdx, 0); // never before the explicit override
    if (embeddedIdx >= 0)
        EXPECT_LT(embeddedIdx, msyhIdx); // embedded fallback before system probing
#endif
}

TEST_F(FontTest, ProbeCJK_FindsExplicitFile_NoAtlasSideEffect) {
    auto& m = Manager::Instance();
    m.SetCJKPath(kRepoCJCFont);

    FontProbe probe = m.ProbeCJK();
    ASSERT_TRUE(probe.found) << probe.error;
    EXPECT_EQ(probe.source, FontSource::Explicit);
    EXPECT_EQ(probe.path, std::string(kRepoCJCFont));
    EXPECT_TRUE(probe.error.empty());
    EXPECT_FALSE(probe.probed.empty());
    EXPECT_EQ(probe.probed.back(), std::string(kRepoCJCFont));

    // Probing resolves only — nothing was merged into the atlas yet.
    EXPECT_EQ(m.Get("cjk"), nullptr);
    // Diagnostics reports the chain's *resolvability* (the fallback report), so the
    // explicit font is visible there even though no atlas entry exists.
    FontDiagnostics diag = m.Diagnostics();
    EXPECT_TRUE(diag.cjkFound);
    EXPECT_EQ(diag.cjkSource, FontSource::Explicit);
    EXPECT_EQ(diag.cjkPath, std::string(kRepoCJCFont));
    EXPECT_FALSE(diag.bodyLoaded);
}

TEST_F(FontTest, ProbeCJK_SearchPathDirectory_ExpandsKnownFileNames) {
    auto& m = Manager::Instance();
    // A search path pointing at the repo CJK file itself is used verbatim.
    m.SetSearchPaths({kRepoCJCFont});
    m.SetSystemProbeEnabled(false);
    m.SetEmbeddedFallbackEnabled(false);

    FontProbe probe = m.ProbeCJK();
    ASSERT_TRUE(probe.found) << probe.error;
    EXPECT_EQ(probe.source, FontSource::Explicit);
    EXPECT_EQ(probe.path, std::string(kRepoCJCFont));
}

// ── Explicit failure instead of silent skipping ─────────────────────────────

TEST_F(FontTest, MissingCJK_ExplicitErrorAndNoFontReport) {
    auto& m = Manager::Instance();
    m.SetCJKPath(kMissingFont);
    m.SetSystemProbeEnabled(false);
    m.SetEmbeddedFallbackEnabled(false);

    FontProbe probe = m.ProbeCJK();
    ASSERT_FALSE(probe.found);
    EXPECT_FALSE(probe.error.empty());
    EXPECT_NE(probe.error.find(kMissingFont), std::string::npos) << probe.error;
    ASSERT_FALSE(probe.probed.empty());
    EXPECT_FALSE(probe.ToString().empty());

    // Loading reports the same explicit failure and never registers a font.
    FontProbe loaded = m.LoadSystemCJKEx(16.0f);
    EXPECT_FALSE(loaded.found);
    EXPECT_EQ(m.Get("cjk"), nullptr);

    FontDiagnostics diag = m.Diagnostics();
    EXPECT_FALSE(diag.cjkFound);
    EXPECT_FALSE(diag.cjkError.empty());
    EXPECT_FALSE(m.SystemProbeEnabled());
    EXPECT_FALSE(m.EmbeddedFallbackEnabled());
    EXPECT_NE(diag.Summary().find("cjk"), std::string::npos);
}

TEST_F(FontTest, LoadChecked_MissingFile_ReturnsExplicitError) {
    auto& m = Manager::Instance();
    FontLoadResult result = m.LoadChecked("missing", kMissingFont, 16.0f);
    EXPECT_FALSE(result.ok);
    EXPECT_EQ(result.font, nullptr);
    EXPECT_FALSE(result.error.empty());
    EXPECT_NE(result.error.find(kMissingFont), std::string::npos) << result.error;
    EXPECT_EQ(m.Get("missing"), nullptr);

    FontLoadResult emptyName = m.LoadChecked("", kRepoBodyFont, 16.0f);
    EXPECT_FALSE(emptyName.ok);
    EXPECT_FALSE(emptyName.error.empty());
}

TEST_F(FontTest, LoadChecked_RealFile_ReturnsFont) {
    auto& m = Manager::Instance();
    FontLoadResult result = m.LoadChecked("body", kRepoBodyFont, 16.0f);
    ASSERT_TRUE(result.ok) << result.error;
    ASSERT_NE(result.font, nullptr);
    EXPECT_EQ(m.Get("body"), result.font);
    EXPECT_TRUE(m.Unload("body"));
}

// ── Embedded OFL CJK subset (S0/D37) ────────────────────────────────────────

TEST_F(FontTest, EmbeddedCJK_IsUsedWhenSystemProbeDisabled) {
    auto& m = Manager::Instance();
    m.SetSystemProbeEnabled(false);
    m.SetEmbeddedFallbackEnabled(true);
    const bool embedded = m.HasEmbeddedCJKFont();

    FontProbe probe = m.ProbeCJK();
    if (embedded) {
        ASSERT_TRUE(probe.found) << probe.error;
        EXPECT_EQ(probe.source, FontSource::Embedded);
        EXPECT_EQ(probe.path, std::string(kEmbeddedCJKAssetId));
        EXPECT_TRUE(Contains(probe.probed, kEmbeddedCJKAssetId));
    } else {
        EXPECT_FALSE(probe.found);
        EXPECT_NE(probe.error.find("embedded"), std::string::npos) << probe.error;
    }

    // Disabling the fallback removes the candidate again.
    m.SetEmbeddedFallbackEnabled(false);
    EXPECT_FALSE(m.ProbeCJK().found);
}

TEST_F(FontTest, EmbeddedCJK_MergesIntoDefaultFont) {
    auto& m = Manager::Instance();
    if (!m.HasEmbeddedCJKFont()) {
        GTEST_SKIP() << "build has no embedded OFL CJK subset (documented gap)";
    }
    m.SetSystemProbeEnabled(false);
    m.SetEmbeddedFallbackEnabled(true);

    FontProbe loaded = m.LoadSystemCJKEx(16.0f);
    ASSERT_TRUE(loaded.found) << loaded.error;
    EXPECT_EQ(loaded.source, FontSource::Embedded);

    ImFont* font = m.Get("cjk");
    ASSERT_NE(font, nullptr);
    EXPECT_EQ(m.LoadSystemCJKEx(16.0f).path, std::string(kEmbeddedCJKAssetId)); // idempotent

    FontDiagnostics diag = m.Diagnostics();
    EXPECT_TRUE(diag.cjkFound);
    EXPECT_EQ(diag.cjkSource, FontSource::Embedded);
    EXPECT_TRUE(diag.embeddedCJKAvailable);
    EXPECT_FALSE(diag.fallbackOnly); // body seeded + CJK found
    EXPECT_TRUE(m.Unload("cjk"));
}

// ── Platform-aware system fallbacks (emoji + CJK) ────────────────────────────
// The system-font probes must never crash and must keep the registry consistent
// (return == Get(name)); presence is only guaranteed on Windows, where Segoe UI
// Emoji and Microsoft YaHei ship with the OS.

TEST_F(FontTest, LoadSystemEmoji_RegistryConsistent_NoCrash) {
    auto& m = Manager::Instance();
    m.LoadSystemEmoji();
    ImFont* got = m.Get("emoji");
#ifdef _WIN32
    EXPECT_NE(got, nullptr); // seguiemj.ttf ships with every supported Windows
#endif
    if (got) {
        // Idempotent: a second call must not duplicate the atlas entry.
        m.LoadSystemEmoji();
        EXPECT_EQ(m.Get("emoji"), got);
        EXPECT_TRUE(m.Unload("emoji")); // restore singleton state for later tests
    }
}

TEST_F(FontTest, LoadSystemCJK_ReturnsRegisteredFont_Idempotent) {
    auto& m = Manager::Instance();
    ImFont* ret = m.LoadSystemCJK();
    EXPECT_EQ(ret, m.Get("cjk")); // return value and registry must agree
#ifdef _WIN32
    EXPECT_NE(ret, nullptr); // msyh.ttc/simsun.ttc ship with every supported Windows
#endif
    if (ret) {
        EXPECT_EQ(m.LoadSystemCJK(), ret); // idempotent — same entry, no duplicate
        EXPECT_TRUE(m.Unload("cjk"));
    }
}
