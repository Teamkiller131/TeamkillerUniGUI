#include <unigui/widgets/datatable.h>
#include <unigui/widgets/table.h>

#include <imgui.h>

#include <gtest/gtest.h>
class TableTest : public ::testing::Test {
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
TEST_F(TableTest, Render_DoesNotCrash) {
    unigui::Table tbl("tbl", {"A", "B"});
    tbl.AddRow({"1", "2"});
    tbl.Render();
}
TEST_F(TableTest, ClearRows_Works) {
    unigui::Table tbl("tbl", {"Col"});
    tbl.AddRow({"X"});
    tbl.ClearRows();
    tbl.Render();
}
TEST_F(TableTest, SaveRestoreColumnWidths_DoesNotCrash) {
    unigui::Table tbl("tbl", {"C1", "C2"});
    tbl.AddRow({"a", "b"});
    tbl.Render();
    tbl.SaveColumnWidths();
    tbl.RestoreColumnWidths();
}

TEST_F(TableTest, ExportCSV_ReturnsHeaderAndRow) {
    unigui::Table tbl("tbl", {"Name", "Value"});
    tbl.AddRow({"X", "1"});
    tbl.AddRow({"Y", "2"});
    auto csv = tbl.ExportCSV();
    EXPECT_NE(csv.find("Name"), std::string::npos);
    EXPECT_NE(csv.find("X"), std::string::npos);
}

TEST_F(TableTest, ImportCSV_AddsRows) {
    unigui::Table tbl("tbl", {"A", "B"});
    std::string csv = "A,B\n1,2\n3,4\n";
    tbl.ImportCSV(csv);
    tbl.Render();
}

// ── CSV escaping round-trip: quoted commas and embedded quotes survive. ──
TEST_F(TableTest, CSV_RoundTrip_QuotedCommaAndQuotes) {
    unigui::Table tbl("tbl", {"A", "B"});
    tbl.AddRow({"hello, world", "say \"hi\""});
    auto csv = tbl.ExportCSV();

    unigui::Table tbl2("tbl2", {"A", "B"});
    tbl2.ImportCSV(csv);
    ASSERT_EQ(tbl2.RowCount(), 1);
    EXPECT_EQ(tbl2.CellText(0, 0), "hello, world");
    EXPECT_EQ(tbl2.CellText(0, 1), "say \"hi\"");
}

TEST_F(TableTest, ImportCSV_MalformedShortAndLongRows_DoNotCrash) {
    unigui::Table tbl("tbl", {"A", "B", "C"});
    EXPECT_NO_THROW(tbl.ImportCSV("A,B,C\n1\n2,3\n4,5,6,7\n\n"));
    tbl.Render();
}

TEST_F(TableTest, ImportCSV_Empty_ClearsRows) {
    unigui::Table tbl("tbl", {"A"});
    tbl.AddRow({"x"});
    tbl.ImportCSV("");
    EXPECT_EQ(tbl.RowCount(), 0);
}

TEST_F(TableTest, DataTableVirtualScroll_DoesNotCrash) {
    struct Row {
        std::string name;
        int value;
    };
    std::vector<Row> rows;
    for (int i = 0; i < 100; ++i)
        rows.push_back({"R" + std::to_string(i), i});

    unigui::DataTable<Row> table("dt", {{"Name", 120}, {"Value", 80}});
    table.SetDataSource(&rows);
    table.SetVirtualScroll(true);
    table.SetCellFormatter([](int, int col, const Row& row) {
        return col == 0 ? row.name : std::to_string(row.value);
    });
    table.Render();
}

TEST_F(TableTest, ColumnAlignmentAndUnit_RenderDoesNotCrash) {
    unigui::Table tbl("tbl", {"Name", "Volume", "Price"});
    tbl.SetColumnAlignment(1, unigui::Table::Alignment::Center);
    tbl.SetColumnAlignment(2, unigui::Table::Alignment::Right);
    tbl.SetColumnUnit(1, "手");
    tbl.AddRow({"IF2506", "702", "12.5"});
    tbl.AddRow({"IH2506", "8", "9.2"});
    tbl.Render();
    EXPECT_EQ(tbl.GetColumnAlignment(1), unigui::Table::Alignment::Center);
    EXPECT_EQ(tbl.GetColumnAlignment(2), unigui::Table::Alignment::Right);
    EXPECT_EQ(tbl.GetColumnUnit(1), "手");
}

TEST_F(TableTest, ApplySort_ParsesUnitSuffixedNumbers) {
    unigui::Table tbl("tbl", {"Volume"});
    tbl.AddRow({"8手"});
    tbl.AddRow({"702手"});
    tbl.SortByColumn(0, true);
    EXPECT_EQ(tbl.CellText(0, 0), "8手");
    EXPECT_EQ(tbl.CellText(1, 0), "702手");

    tbl.SortByColumn(0, false);
    EXPECT_EQ(tbl.CellText(0, 0), "702手");
    EXPECT_EQ(tbl.CellText(1, 0), "8手");
}

TEST_F(TableTest, ApplySort_UsesConfiguredUnitForRawNumbers) {
    unigui::Table tbl("tbl", {"Volume"});
    tbl.SetColumnUnit(0, "手");
    tbl.AddRow({"8"});
    tbl.AddRow({"702"});
    tbl.SortByColumn(0, false);
    EXPECT_EQ(tbl.CellText(0, 0), "702");
    EXPECT_EQ(tbl.CellText(1, 0), "8");
}

// Signed values must order numerically (from_chars accepts '-' and a leading
// '+'), not lexically.
TEST_F(TableTest, ApplySort_OrdersSignedNumbers) {
    unigui::Table tbl("tbl", {"PnL"});
    tbl.AddRow({"+3.5"});
    tbl.AddRow({"-10"});
    tbl.AddRow({"2"});
    tbl.SortByColumn(0, true); // ascending: -10, 2, +3.5
    EXPECT_EQ(tbl.CellText(0, 0), "-10");
    EXPECT_EQ(tbl.CellText(1, 0), "2");
    EXPECT_EQ(tbl.CellText(2, 0), "+3.5");
}

// Non-numeric cells mixed with numbers must not crash the sort (the parser is
// non-throwing) and must still sort cleanly.
TEST_F(TableTest, ApplySort_MixedNumericAndTextDoesNotCrash) {
    unigui::Table tbl("tbl", {"Col"});
    tbl.AddRow({"banana"});
    tbl.AddRow({"42"});
    tbl.AddRow({"apple"});
    tbl.AddRow({"7"});
    EXPECT_NO_THROW(tbl.SortByColumn(0, true));
    EXPECT_NO_THROW(tbl.SortByColumn(0, false));
}

// ── DataTable enhancements (jzdz-fit) ────────────────────────────────────────

namespace {
struct DtRow {
    std::string name;
    double pnl = 0.0;
    unsigned char enabled = 0; // flag stored as uint8_t (the UB-prone case)
};
} // namespace

TEST_F(TableTest, DataTable_EmptyText_RendersWithoutCrash) {
    std::vector<DtRow> rows; // empty
    unigui::DataTable<DtRow> dt("dt", {{"Name", 100}, {"PnL", 80}});
    dt.SetDataSource(&rows);
    dt.SetEmptyText("no data");
    dt.SetCellFormatter([](int, int c, const DtRow& r) {
        return c == 0 ? r.name : std::to_string(r.pnl);
    });
    EXPECT_NO_THROW(dt.Render());
}

TEST_F(TableTest, DataTable_CheckboxValue_GetSetNoUB) {
    std::vector<DtRow> rows = {{"a", 1.0, 0}, {"b", -2.0, 1}};
    unigui::DataTable<DtRow> dt("dt2", {{"On", 40}, {"Name", 100}});
    dt.SetDataSource(&rows);
    // Read via getter, write via setter — no reinterpret_cast<bool*> over uint8_t.
    dt.SetCellCheckboxValue(
        0, [&](int row, const DtRow& r) { return r.enabled != 0; },
        [&](int row, bool v) { rows[row].enabled = v ? 1 : 0; });
    dt.SetCellFormatter([](int, int, const DtRow& r) { return r.name; });
    EXPECT_NO_THROW(dt.Render());
    // The getter/setter wiring compiles and runs against uint8_t-backed storage.
    EXPECT_EQ(rows[1].enabled, 1);
}

// ── Row-accessor data source (no per-frame vector copy) ──────────────────────
TEST_F(TableTest, DataTable_RowAccessorSource_Renders) {
    // A model exposing Count()/GetAt(i) instead of a contiguous vector.
    std::vector<DtRow> backing = {{"x", 1.0, 0}, {"y", 2.0, 0}, {"z", 3.0, 0}};
    unigui::DataTable<DtRow> dt("dt_acc", {{"Name", 100}, {"PnL", 80}});
    dt.SetDataSource(backing.size(), [&](std::size_t i) -> const DtRow& { return backing[i]; });
    int formatted = 0;
    dt.SetCellFormatter([&](int, int c, const DtRow& r) {
        ++formatted;
        return c == 0 ? r.name : std::to_string(r.pnl);
    });
    EXPECT_NO_THROW(dt.Render());
    EXPECT_GT(formatted, 0); // the accessor source was iterated
}

TEST_F(TableTest, DataTable_SignColorColumn_Renders) {
    std::vector<DtRow> rows = {{"up", 5.0, 0}, {"flat", 0.0, 0}, {"down", -3.0, 0}};
    unigui::DataTable<DtRow> dt("dt_sign", {{"Name", 100}, {"PnL", 80}});
    dt.SetDataSource(&rows);
    dt.SetCellFormatter([](int, int c, const DtRow& r) {
        return c == 0 ? r.name : std::to_string(r.pnl);
    });
    // Column 1 coloured by the sign of pnl via theme Up/Down tokens.
    dt.SetCellSignColor(1, [](int, const DtRow& r) { return r.pnl; });
    EXPECT_NO_THROW(dt.Render());
}
