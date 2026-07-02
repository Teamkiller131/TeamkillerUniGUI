#include <unigui/core/accessibility.h>
#include <unigui/widgets/datatable.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

using unigui::detail::CompareSortCells;
using unigui::detail::ParseNumericSortKey;

// detail::ParseNumericSortKey used to parse with std::stod inside a try/catch — a banned
// throwing parser that threw (as control flow) on every non-numeric cell during a sort. It
// now uses the non-throwing strutil TryToDouble; these tests pin the leading-prefix +
// no-throw behaviour and the numeric-aware comparison.

TEST(DataTableSort, ParsesLeadingNumericPrefix) {
    double v = 0;
    EXPECT_TRUE(ParseNumericSortKey("1194", v));
    EXPECT_DOUBLE_EQ(v, 1194.0);
    EXPECT_TRUE(ParseNumericSortKey("100手", v)); // trailing non-numeric ignored (like strtod)
    EXPECT_DOUBLE_EQ(v, 100.0);
    EXPECT_TRUE(ParseNumericSortKey("  42", v)); // leading whitespace skipped
    EXPECT_DOUBLE_EQ(v, 42.0);
    EXPECT_TRUE(ParseNumericSortKey("-3.5", v));
    EXPECT_DOUBLE_EQ(v, -3.5);
}

TEST(DataTableSort, RejectsNonNumeric_NeverThrows) {
    double v = 999;
    EXPECT_NO_THROW({
        EXPECT_FALSE(ParseNumericSortKey("abc", v));
        EXPECT_FALSE(ParseNumericSortKey("", v));
        EXPECT_FALSE(ParseNumericSortKey("-", v));
        EXPECT_FALSE(ParseNumericSortKey("   ", v));
    });
}

TEST(DataTableSort, NumericComparisonBeatsLexical) {
    // Lexically "100" < "99"; numerically 100 > 99 — the numeric key must win.
    EXPECT_LT(CompareSortCells("99", "100", /*ascending=*/true), 0);
    EXPECT_GT(CompareSortCells("100", "99", /*ascending=*/true), 0);
    EXPECT_EQ(CompareSortCells("42", "42", /*ascending=*/true), 0);
}

TEST(DataTableSort, FallsBackToLexicalForText) {
    EXPECT_LT(CompareSortCells("apple", "banana", /*ascending=*/true), 0);
    EXPECT_GT(CompareSortCells("banana", "apple", /*ascending=*/true), 0);
}

TEST(DataTableSort, DescendingInverts) {
    EXPECT_GT(CompareSortCells("99", "100", /*ascending=*/false), 0);
}

// ── Accessibility: the table registers its dimensions / filter / selection ──
class DataTableA11yTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
        unigui::a11y::SetEnabled(true);
        unigui::a11y::BeginFrame();
        unigui::a11y::DrainAnnouncements();
    }
    void TearDown() override {
        unigui::a11y::SetEnabled(false);
        ImGui::Render();
        ImGui::DestroyContext();
    }

    static unigui::DataTable<int> MakeTable(const char* name, const std::vector<int>* data) {
        unigui::DataTable<int> t(name, {{"A"}, {"B"}, {"C"}});
        t.SetDataSource(data);
        t.SetCellFormatter([](int row, int col, const int& v) {
            return std::to_string(v * 10 + col) + (row == 0 ? "alpha" : "");
        });
        return t;
    }
};

TEST_F(DataTableA11yTest, Container_ReportsDimensions_AndVisibleRows) {
    const std::vector<int> data = {1, 2, 3, 4, 5};
    auto t = MakeTable("dt_a11y", &data);
    t.Render();
    bool sawTable = false;
    int rowItems = 0;
    for (const auto& n : unigui::a11y::Tree()) {
        if (n.role == unigui::a11y::Role::Table) {
            sawTable = true;
            EXPECT_EQ(n.value, "3x5 rows");
        }
        if (n.role == unigui::a11y::Role::ListItem)
            ++rowItems;
    }
    EXPECT_TRUE(sawTable);
    EXPECT_EQ(rowItems, 5); // each visible row registered
}

TEST_F(DataTableA11yTest, Filter_NarrowsShownCount) {
    const std::vector<int> data = {1, 2, 3, 4, 5};
    auto t = MakeTable("dt_a11y_f", &data);
    t.SetFilterText("alpha"); // only row 0's cells contain "alpha"
    t.Render();
    bool saw = false;
    for (const auto& n : unigui::a11y::Tree())
        if (n.role == unigui::a11y::Role::Table) {
            saw = true;
            EXPECT_EQ(n.value, "3x5 rows, 1 shown");
        }
    EXPECT_TRUE(saw);
}

TEST_F(DataTableA11yTest, Selection_AppearsInContainerValue) {
    const std::vector<int> data = {1, 2, 3};
    auto t = MakeTable("dt_a11y_s", &data);
    t.SetSelectedRow(2);
    t.Render();
    bool saw = false;
    for (const auto& n : unigui::a11y::Tree())
        if (n.role == unigui::a11y::Role::Table) {
            saw = true;
            EXPECT_EQ(n.value, "3x3 rows, row 2 selected");
        }
    EXPECT_TRUE(saw);
}
