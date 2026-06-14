#include <unigui/core/locale.h>
#include <unigui/unigui.h>

#include <gtest/gtest.h>
class LocaleTest : public ::testing::Test {
protected:
    void TearDown() override {
        unigui::Locale::Clear();
        unigui::Locale::SetCurrent("en_US");
        unigui::Locale::SetFallback("en_US");
    }
};
TEST_F(LocaleTest, DefaultLocaleIsEnUS) {
    EXPECT_EQ(unigui::Locale::GetCurrent(), "en_US");
}
TEST_F(LocaleTest, SetAndTranslate) {
    unigui::Locale::Set("en_US", "hello", "Hello");
    unigui::Locale::Set("zh_CN", "hello", "\344\275\240\345\245\275");
    unigui::Locale::SetCurrent("zh_CN");
    EXPECT_EQ(unigui::Locale::Tr("hello"), "\344\275\240\345\245\275");
    EXPECT_TRUE(unigui::Locale::Has("hello"));
}

// ── Fallback chain (Horizon 5 i18n) ─────────────────────────────────────────

TEST_F(LocaleTest, Tr_FallsBackToFallbackLocale) {
    unigui::Locale::Set("en_US", "btn.ok", "OK");
    unigui::Locale::SetCurrent("ja_JP"); // no translation for this key
    // Falls back to en_US instead of leaking the raw key.
    EXPECT_EQ(unigui::Locale::Tr("btn.ok"), "OK");
}

TEST_F(LocaleTest, Tr_FallsBackToBaseLanguage) {
    unigui::Locale::Set("zh", "greeting", "base-zh");
    unigui::Locale::SetCurrent("zh_CN"); // region-specific not present
    EXPECT_EQ(unigui::Locale::Tr("greeting"), "base-zh");
}

TEST_F(LocaleTest, Tr_RegionOverridesBaseLanguage) {
    unigui::Locale::Set("zh", "greeting", "base-zh");
    unigui::Locale::Set("zh_CN", "greeting", "cn-specific");
    unigui::Locale::SetCurrent("zh_CN");
    EXPECT_EQ(unigui::Locale::Tr("greeting"), "cn-specific");
}

TEST_F(LocaleTest, Tr_UnknownKeyReturnsKey) {
    unigui::Locale::SetCurrent("fr_FR");
    EXPECT_EQ(unigui::Locale::Tr("does.not.exist"), "does.not.exist");
}

TEST_F(LocaleTest, CustomFallbackLocale) {
    unigui::Locale::Set("eo", "x", "eo-value");
    unigui::Locale::SetFallback("eo");
    unigui::Locale::SetCurrent("ja_JP");
    EXPECT_EQ(unigui::Locale::Tr("x"), "eo-value");
}

// ── Positional argument substitution ────────────────────────────────────────

TEST_F(LocaleTest, Tr_SubstitutesPositionalArgs) {
    unigui::Locale::Set("en_US", "welcome", "Hi {0}, you have {1} messages");
    EXPECT_EQ(unigui::Locale::Tr("welcome", {"Sam", "3"}), "Hi Sam, you have 3 messages");
}

TEST_F(LocaleTest, Tr_RepeatedPlaceholderAllReplaced) {
    unigui::Locale::Set("en_US", "echo", "{0}-{0}");
    EXPECT_EQ(unigui::Locale::Tr("echo", {"x"}), "x-x");
}

TEST_F(LocaleTest, Tr_MissingArgLeftAsIs) {
    unigui::Locale::Set("en_US", "t", "{0} and {1}");
    EXPECT_EQ(unigui::Locale::Tr("t", {"only"}), "only and {1}");
}

// ── RTL detection ───────────────────────────────────────────────────────────

TEST_F(LocaleTest, IsRTL_DetectsRtlLanguages) {
    EXPECT_TRUE(unigui::Locale::IsRTL("ar_SA"));
    EXPECT_TRUE(unigui::Locale::IsRTL("he"));
    EXPECT_TRUE(unigui::Locale::IsRTL("fa_IR"));
    EXPECT_FALSE(unigui::Locale::IsRTL("en_US"));
    EXPECT_FALSE(unigui::Locale::IsRTL("zh_CN"));
}

TEST_F(LocaleTest, IsRTL_UsesCurrentLocale) {
    unigui::Locale::SetCurrent("ar_EG");
    EXPECT_TRUE(unigui::Locale::IsRTL());
    unigui::Locale::SetCurrent("en_US");
    EXPECT_FALSE(unigui::Locale::IsRTL());
}

// ── Built-in catalog degrades gracefully ────────────────────────────────────

TEST_F(LocaleTest, LoadBuiltin_JapaneseFallsBackForMissingKeys) {
    unigui::Locale::LoadBuiltin();
    unigui::Locale::SetCurrent("ja_JP");
    EXPECT_EQ(unigui::Locale::Tr("menu.file"), "\343\203\225\343\202\241\343\202\244\343\203\253");
    // ja_JP lacks btn.apply in the builtin catalog → falls back to en_US.
    EXPECT_EQ(unigui::Locale::Tr("btn.apply"), "Apply");
}
