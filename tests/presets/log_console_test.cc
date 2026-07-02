#include <unigui/core/accessibility.h>
#include <unigui/presets/log_console.h>

#include <imgui.h>

#include <gtest/gtest.h>

using unigui::presets::LogConsole;
using Level = unigui::presets::LogConsole::Level;

class LogConsoleTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(LogConsoleTest, Defaults_AreSensible) {
    LogConsole lc("log");
    EXPECT_EQ(lc.Size(), 0u);
    EXPECT_EQ(lc.GetCapacity(), 2000u);
    EXPECT_TRUE(lc.GetAutoScroll());
    EXPECT_TRUE(lc.GetFilter().empty());
    EXPECT_TRUE(lc.IsLevelVisible(Level::Debug));
    EXPECT_TRUE(lc.IsLevelVisible(Level::Info));
    EXPECT_TRUE(lc.IsLevelVisible(Level::Warn));
    EXPECT_TRUE(lc.IsLevelVisible(Level::Error));
}

TEST_F(LogConsoleTest, FluentChaining_PreservesDerivedTypeAndApplies) {
    LogConsole lc("log");
    // Base With* (CRTP) followed by preset-specific With* stays LogConsole&.
    LogConsole& same = lc.WithTooltip("recent events").WithCapacity(50).WithAutoScroll(false);
    EXPECT_EQ(&same, &lc);
    EXPECT_EQ(lc.GetCapacity(), 50u);
    EXPECT_FALSE(lc.GetAutoScroll());
}

TEST_F(LogConsoleTest, Append_GrowsSizeUpToCapacity_ThenEvictsOldest) {
    LogConsole lc("log");
    lc.WithCapacity(3);
    for (int i = 0; i < 5; ++i)
        lc.Append(Level::Info, "msg" + std::to_string(i));
    EXPECT_EQ(lc.Size(), 3u);
    // Oldest lines were evicted, newest survive.
    lc.SetFilter("msg0");
    EXPECT_EQ(lc.FilteredSize(), 0u);
    lc.SetFilter("msg4");
    EXPECT_EQ(lc.FilteredSize(), 1u);
}

TEST_F(LogConsoleTest, WithCapacity_ShrinkEvictsOldestImmediately) {
    LogConsole lc("log");
    for (int i = 0; i < 5; ++i)
        lc.Append(Level::Info, "msg" + std::to_string(i));
    lc.WithCapacity(2);
    EXPECT_EQ(lc.Size(), 2u);
    lc.SetFilter("msg3");
    EXPECT_EQ(lc.FilteredSize(), 1u);
    lc.SetFilter("msg2");
    EXPECT_EQ(lc.FilteredSize(), 0u);
}

TEST_F(LogConsoleTest, Filter_MatchesPlainSubstring) {
    LogConsole lc("log");
    lc.Append(Level::Info, "connection opened");
    lc.Append(Level::Info, "connection closed");
    lc.Append(Level::Info, "heartbeat");
    lc.SetFilter("connection");
    EXPECT_EQ(lc.GetFilter(), "connection");
    EXPECT_EQ(lc.FilteredSize(), 2u);
    lc.SetFilter(""); // empty filter passes everything
    EXPECT_EQ(lc.FilteredSize(), 3u);
}

TEST_F(LogConsoleTest, LevelToggles_HideMatchingSeverities) {
    LogConsole lc("log");
    lc.Append(Level::Debug, "d");
    lc.Append(Level::Info, "i");
    lc.Append(Level::Warn, "w");
    lc.Append(Level::Error, "e");
    lc.SetLevelVisible(Level::Debug, false);
    EXPECT_FALSE(lc.IsLevelVisible(Level::Debug));
    EXPECT_EQ(lc.FilteredSize(), 3u);
    lc.SetLevelVisible(Level::Error, false);
    EXPECT_EQ(lc.FilteredSize(), 2u);
    lc.SetLevelVisible(Level::Debug, true);
    EXPECT_EQ(lc.FilteredSize(), 3u);
}

TEST_F(LogConsoleTest, LevelName_MatchesLinePrefixTags) {
    EXPECT_STREQ(LogConsole::LevelName(Level::Debug), "DEBUG");
    EXPECT_STREQ(LogConsole::LevelName(Level::Info), "INFO");
    EXPECT_STREQ(LogConsole::LevelName(Level::Warn), "WARN");
    EXPECT_STREQ(LogConsole::LevelName(Level::Error), "ERROR");
}

// ── Accessibility ────────────────────────────────────────────────────────────
class LogConsoleA11yTest : public LogConsoleTest {
protected:
    void SetUp() override {
        LogConsoleTest::SetUp();
        unigui::a11y::SetEnabled(true);
        unigui::a11y::BeginFrame();
        unigui::a11y::DrainAnnouncements();
    }
    void TearDown() override {
        unigui::a11y::SetEnabled(false);
        LogConsoleTest::TearDown();
    }
};

TEST_F(LogConsoleA11yTest, Clear_AnnouncesOnce_ButNotWhenAlreadyEmpty) {
    LogConsole lc("log_a11y");
    lc.Append(Level::Info, "something");
    lc.Clear();
    auto msgs = unigui::a11y::DrainAnnouncements();
    ASSERT_EQ(msgs.size(), 1u);
    EXPECT_NE(msgs[0].message.find("cleared"), std::string::npos);
    EXPECT_EQ(lc.Size(), 0u);
    lc.Clear(); // no-op on an empty log — no announcement spam
    EXPECT_TRUE(unigui::a11y::DrainAnnouncements().empty());
}

TEST_F(LogConsoleA11yTest, Render_ReportsContainerWithLineCount) {
    LogConsole lc("log_a11y");
    lc.Append(Level::Info, "alpha");
    lc.Append(Level::Warn, "beta");
    lc.Render();
    bool sawContainer = false;
    for (const auto& n : unigui::a11y::Tree()) {
        if (n.role == unigui::a11y::Role::Group && n.value == "2 lines")
            sawContainer = true;
    }
    EXPECT_TRUE(sawContainer);
    // Per-line announcements would spam a screen reader — Render stays silent.
    EXPECT_TRUE(unigui::a11y::DrainAnnouncements().empty());
}

TEST_F(LogConsoleTest, Render_DoesNotCrash) {
    LogConsole lc("log_render");
    for (int i = 0; i < 100; ++i)
        lc.Append(static_cast<Level>(i % 4), "line " + std::to_string(i));
    lc.SetFilter("line 4");
    lc.SetLevelVisible(Level::Debug, false);
    lc.Render();
    LogConsole empty("log_render_empty");
    empty.Render();
}
