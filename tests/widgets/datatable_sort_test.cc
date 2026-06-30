#include <unigui/widgets/datatable.h>

#include <gtest/gtest.h>
#include <string>

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
