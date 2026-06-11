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
