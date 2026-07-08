#include <unigui/fonts/font_manager.h>

#include <imgui.h>

#include <gtest/gtest.h>
using namespace unigui::fonts;

class FontTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); }
    void TearDown() override { ImGui::DestroyContext(); }
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
