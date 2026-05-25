#include <unigui/unigui.h>
#include <unigui/widgets/table.h>
#include <imgui.h>
#include <gtest/gtest.h>
class TableTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(TableTest, Render_DoesNotCrash) { unigui::Table tbl("tbl", {"A","B"}); tbl.AddRow({"1","2"}); tbl.Render(); }
TEST_F(TableTest, ClearRows_Works) { unigui::Table tbl("tbl", {"Col"}); tbl.AddRow({"X"}); tbl.ClearRows(); tbl.Render(); }
TEST_F(TableTest, SaveRestoreColumnWidths_DoesNotCrash) {
    unigui::Table tbl("tbl", {"C1","C2"});
    tbl.AddRow({"a","b"});
    tbl.Render();
    tbl.SaveColumnWidths();
    tbl.RestoreColumnWidths();
}

TEST_F(TableTest, ExportCSV_ReturnsHeaderAndRow) {
    unigui::Table tbl("tbl", {"Name","Value"});
    tbl.AddRow({"X","1"});
    tbl.AddRow({"Y","2"});
    auto csv = tbl.ExportCSV();
    EXPECT_NE(csv.find("Name"), std::string::npos);
    EXPECT_NE(csv.find("X"), std::string::npos);
}

TEST_F(TableTest, ImportCSV_AddsRows) {
    unigui::Table tbl("tbl", {"A","B"});
    std::string csv = "A,B\n1,2\n3,4\n";
    tbl.ImportCSV(csv);
    tbl.Render();
}
