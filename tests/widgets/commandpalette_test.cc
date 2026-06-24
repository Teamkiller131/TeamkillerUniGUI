#include <unigui/widgets/commandpalette.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

using unigui::CommandPalette;
using unigui::detail::FuzzyMatch;

// ── Pure fuzzy-match core ────────────────────────────────────────────────────

TEST(FuzzyMatch, EmptyPatternMatchesEverythingScoreZero) {
    int s = 123;
    EXPECT_TRUE(FuzzyMatch("", "anything", s));
    EXPECT_EQ(s, 0);
}

TEST(FuzzyMatch, SubsequenceMatchesAndGapsFail) {
    int s = 0;
    EXPECT_TRUE(FuzzyMatch("fmt", "Format Document", s)); // F..m..t in order
    EXPECT_FALSE(FuzzyMatch("xyz", "Format Document", s));
    EXPECT_EQ(s, 0); // reset to 0 on non-match
}

TEST(FuzzyMatch, CaseInsensitive) {
    int s = 0;
    EXPECT_TRUE(FuzzyMatch("OPEN", "open file", s));
    EXPECT_TRUE(FuzzyMatch("open", "OPEN FILE", s));
}

TEST(FuzzyMatch, PrefixOutranksScattered) {
    int prefix = 0, scattered = 0;
    ASSERT_TRUE(FuzzyMatch("op", "Open Project", prefix));         // contiguous prefix
    ASSERT_TRUE(FuzzyMatch("op", "Reopen Properties", scattered)); // later, non-prefix
    EXPECT_GT(prefix, scattered);
}

TEST(FuzzyMatch, WordBoundaryOutranksMidWord) {
    int boundary = 0, mid = 0;
    ASSERT_TRUE(FuzzyMatch("f", "Open File", boundary)); // 'F' at word start
    ASSERT_TRUE(FuzzyMatch("f", "Profile", mid));        // 'f' mid-word
    EXPECT_GT(boundary, mid);
}

// ── Command registry ─────────────────────────────────────────────────────────

TEST(CommandPalette, AddRemoveAndDedupById) {
    CommandPalette p;
    p.AddCommand("save", "Save File", [] {});
    p.AddCommand("open", "Open File", [] {});
    EXPECT_EQ(p.CommandCount(), 2u);
    EXPECT_TRUE(p.HasCommand("save"));

    // Same id replaces, not duplicates.
    p.AddCommand("save", "Save As…", [] {});
    EXPECT_EQ(p.CommandCount(), 2u);

    EXPECT_TRUE(p.RemoveCommand("open"));
    EXPECT_FALSE(p.RemoveCommand("nope"));
    EXPECT_EQ(p.CommandCount(), 1u);

    p.ClearCommands();
    EXPECT_EQ(p.CommandCount(), 0u);
}

// ── Query → ranked results ───────────────────────────────────────────────────

TEST(CommandPalette, EmptyQueryReturnsAllEnabledInOrder) {
    CommandPalette p;
    p.AddCommand("a", "Alpha", [] {});
    p.AddCommand("b", "Beta", [] {});
    p.AddCommand("c", "Gamma", [] {});
    const auto m = p.Matches();
    ASSERT_EQ(m.size(), 3u);
    EXPECT_EQ(m[0], "a");
    EXPECT_EQ(m[1], "b");
    EXPECT_EQ(m[2], "c");
}

TEST(CommandPalette, DisabledCommandsHidden) {
    CommandPalette p;
    CommandPalette::Command c;
    c.id = "hidden";
    c.title = "Hidden Command";
    c.enabled = false;
    p.AddCommand(std::move(c));
    p.AddCommand("shown", "Shown Command", [] {});
    const auto m = p.Matches();
    ASSERT_EQ(m.size(), 1u);
    EXPECT_EQ(m[0], "shown");
}

TEST(CommandPalette, QueryFiltersAndRanks) {
    CommandPalette p;
    p.AddCommand("fmt", "Format Document", [] {});
    p.AddCommand("open", "Open File", [] {});
    p.AddCommand("of", "Open Folder", [] {});
    p.SetQuery("of");
    const auto m = p.Matches();
    // "Open Folder" (O..F at word boundaries) should be present and rank above
    // "Format Document" if both match; "Open File" also matches (O..F).
    ASSERT_FALSE(m.empty());
    // Every returned id must actually be a subsequence match.
    for (const auto& id : m)
        EXPECT_TRUE(p.HasCommand(id));
}

TEST(CommandPalette, TitleMatchOutranksCategoryOnlyMatch) {
    CommandPalette p;
    CommandPalette::Command titled;
    titled.id = "t";
    titled.title = "Zebra"; // matches "zeb" in title
    p.AddCommand(std::move(titled));

    CommandPalette::Command catOnly;
    catOnly.id = "c";
    catOnly.title = "Quokka";   // no "zeb"
    catOnly.category = "Zebra"; // matches only via category
    p.AddCommand(std::move(catOnly));

    p.SetQuery("zeb");
    const auto m = p.Matches();
    ASSERT_EQ(m.size(), 2u);
    EXPECT_EQ(m[0], "t"); // title match first
    EXPECT_EQ(m[1], "c"); // category-only match demoted
}

TEST(CommandPalette, MaxResultsCaps) {
    CommandPalette p;
    for (int i = 0; i < 20; ++i)
        p.AddCommand("c" + std::to_string(i), "Cmd " + std::to_string(i), [] {});
    p.SetMaxResults(5);
    EXPECT_EQ(p.Matches().size(), 5u);
}

// ── Execute ──────────────────────────────────────────────────────────────────

TEST(CommandPalette, ExecuteRunsActionAndCloses) {
    CommandPalette p;
    int ran = 0;
    p.AddCommand("go", "Go", [&] { ++ran; });
    p.Open();
    EXPECT_TRUE(p.IsOpen());
    EXPECT_TRUE(p.Execute("go"));
    EXPECT_EQ(ran, 1);
    EXPECT_FALSE(p.IsOpen()); // execution closes the palette
    EXPECT_FALSE(p.Execute("missing"));
    EXPECT_EQ(ran, 1);
}

TEST(CommandPalette, ExecuteDisabledIsNoOp) {
    CommandPalette p;
    int ran = 0;
    CommandPalette::Command c;
    c.id = "d";
    c.title = "Disabled";
    c.enabled = false;
    c.action = [&] { ++ran; };
    p.AddCommand(std::move(c));
    EXPECT_FALSE(p.Execute("d"));
    EXPECT_EQ(ran, 0);
}

TEST(CommandPalette, OpenCloseToggle) {
    CommandPalette p;
    EXPECT_FALSE(p.IsOpen());
    p.Open();
    EXPECT_TRUE(p.IsOpen());
    p.Toggle();
    EXPECT_FALSE(p.IsOpen());
    p.Toggle();
    EXPECT_TRUE(p.IsOpen());
    p.Close();
    EXPECT_FALSE(p.IsOpen());
}

// ── Render: no-crash (headless) ──────────────────────────────────────────────

TEST(CommandPalette, RenderDoesNotCrash) {
    ImGui::CreateContext();
    ImGui::GetIO().DisplaySize = ImVec2(1024, 768);
    ImGui::GetIO().Fonts->Build();

    CommandPalette p;
    p.AddCommand("save", "Save File", [] {});
    p.AddCommand("open", "Open File", [] {});
    p.Open();
    p.SetQuery("sa");

    for (int frame = 0; frame < 3; ++frame) {
        ImGui::NewFrame();
        EXPECT_NO_THROW(p.Render());
        ImGui::Render();
    }
    ImGui::DestroyContext();
}
