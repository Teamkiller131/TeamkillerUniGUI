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
    dt.SetCellFormatter(
        [](int, int c, const DtRow& r) { return c == 0 ? r.name : std::to_string(r.pnl); });
    EXPECT_NO_THROW(dt.Render());
}

TEST_F(TableTest, DataTable_CheckboxValue_GetSetNoUB) {
    std::vector<DtRow> rows = {{"a", 1.0, 0}, {"b", -2.0, 1}};
    unigui::DataTable<DtRow> dt("dt2", {{"On", 40}, {"Name", 100}});
    dt.SetDataSource(&rows);
    // Read via getter, write via setter — no reinterpret_cast<bool*> over uint8_t.
    dt.SetCellCheckboxValue(
        0, [&](int /*row*/, const DtRow& r) { return r.enabled != 0; },
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
    dt.SetCellFormatter(
        [](int, int c, const DtRow& r) { return c == 0 ? r.name : std::to_string(r.pnl); });
    // Column 1 coloured by the sign of pnl via theme Up/Down tokens.
    dt.SetCellSignColor(1, [](int, const DtRow& r) { return r.pnl; });
    EXPECT_NO_THROW(dt.Render());
}

// --- POD 分组改造配套：可变行高 / 行底色 / 行矩形命中（2026-10-27） ---------
// 这几个能力是给「POD 表里插分组标题行、折叠、拖拽排序」用的：Table 原来假设
// 所有行等高（clipper 虚拟化），分组标题行要 34px、POD 行 26px，所以新增了
// row_height_provider_，并且要把每行矩形记下来给拖拽做命中。

namespace {
void RenderInHostWindow(unigui::Table& tbl) {
    ImGui::SetNextWindowSize(ImVec2(640.0f, 480.0f));
    ImGui::Begin("tbl_host", nullptr, ImGuiWindowFlags_NoSavedSettings);
    tbl.Render();
    ImGui::End();
}
} // namespace

TEST_F(TableTest, RowHeightProvider_LaysOutVariableRowHeights) {
    unigui::Table tbl("tbl_rh", {"A", "B"});
    for (int i = 0; i < 3; ++i)
        tbl.AddRow({std::to_string(i), "x"});
    // 第 0 行当「分组标题行」34px，其余 POD 行 26px。
    tbl.SetRowHeightProvider([](int row) { return row == 0 ? 34.0f : 26.0f; });
    RenderInHostWindow(tbl);
    ImVec2 mn0, mx0, mn1, mx1;
    ASSERT_TRUE(tbl.GetRowRect(0, mn0, mx0));
    ASSERT_TRUE(tbl.GetRowRect(1, mn1, mx1));
    EXPECT_GE(mx0.y - mn0.y, 33.5f);
    EXPECT_LE(mx0.y - mn0.y, 40.0f);
    EXPECT_GE(mx1.y - mn1.y, 25.5f);
    EXPECT_LE(mx1.y - mn1.y, 30.0f);
    EXPECT_LT(mx1.y - mn1.y, mx0.y - mn0.y);
}

TEST_F(TableTest, RowHeightProvider_MakesRowsContiguous) {
    unigui::Table tbl("tbl_rh2", {"A"});
    for (int i = 0; i < 3; ++i)
        tbl.AddRow({std::to_string(i)});
    tbl.SetRowHeightProvider([](int) { return 26.0f; });
    RenderInHostWindow(tbl);
    ImVec2 mn, mx;
    for (int r = 1; r < 3; ++r) {
        ImVec2 pmn, pmx;
        ASSERT_TRUE(tbl.GetRowRect(r - 1, pmn, pmx));
        ASSERT_TRUE(tbl.GetRowRect(r, mn, mx));
        EXPECT_NEAR(mn.y, pmx.y, 0.5f);  // 行与行之间不能留缝，否则拖拽落点会错
    }
}

TEST_F(TableTest, RowBgProvider_SeesEveryVisibleRow) {
    unigui::Table tbl("tbl_bg", {"A"});
    for (int i = 0; i < 3; ++i)
        tbl.AddRow({std::to_string(i)});
    std::vector<int> seen;
    const ImVec4 band(0.20f, 0.30f, 0.45f, 0.08f);
    tbl.SetRowBgProvider([&](int row, ImVec4& out) {
        seen.push_back(row);
        out = band;
        return row != 0;  // 第 0 行不染色（分组标题行自己画）
    });
    RenderInHostWindow(tbl);
    ASSERT_EQ(seen.size(), 3u);
    EXPECT_EQ(seen[0], 0);
    EXPECT_EQ(seen[1], 1);
    EXPECT_EQ(seen[2], 2);
}

TEST_F(TableTest, GetRowRect_ReportsNoGeometryBeforeRender) {
    unigui::Table tbl("tbl_norect", {"A"});
    tbl.AddRow({"1"});
    ImVec2 mn, mx;
    EXPECT_FALSE(tbl.GetRowRect(0, mn, mx));
    EXPECT_EQ(tbl.GetRowAtScreenY(100.0f), -1);
}

TEST_F(TableTest, GetRowAtScreenY_HitsTheRowThatWasLaidOut) {
    unigui::Table tbl("tbl_hit", {"A"});
    tbl.AddRow({"1"});
    tbl.AddRow({"2"});
    RenderInHostWindow(tbl);
    ImVec2 mn, mx;
    ASSERT_TRUE(tbl.GetRowRect(1, mn, mx));
    EXPECT_EQ(tbl.GetRowAtScreenY(mn.y + (mx.y - mn.y) * 0.5f), 1);
    EXPECT_EQ(tbl.GetRowAtScreenY(mn.y - 100000.0f), -1);
}

TEST_F(TableTest, ProviderSetters_ReportState) {
    unigui::Table tbl("tbl_flags", {"A"});
    EXPECT_FALSE(tbl.HasRowHeightProvider());
    tbl.SetRowHeightProvider([](int) { return 0.0f; });
    EXPECT_TRUE(tbl.HasRowHeightProvider());
}
