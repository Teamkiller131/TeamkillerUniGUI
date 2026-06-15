#include <unigui/widgets/editabledatagrid.h>

#include <imgui.h>

#include <gtest/gtest.h>

#include <string>
#include <vector>

using namespace unigui;

namespace {
struct PodRow {
    std::string name;
    int mode = 0;     // combo index
    int lots = 1;     // int editor
    float price = 0.f; // float editor
    bool running = false;
};
} // namespace

class EditableDataGridTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1000, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(EditableDataGridTest, RendersAllEditorKindsWithoutCrash) {
    std::vector<PodRow> rows = {{"A", 0, 1, 10.5f, false}, {"B", 1, 3, 9.2f, true}};
    EditableDataGrid<PodRow> grid("pods",
                                  {{"Name", 80}, {"Mode", 90}, {"Lots", 70}, {"Px", 80}, {"Act", 70}});
    grid.SetDataSource(&rows);
    grid.SetCellFormatter([](int, int c, const PodRow& r) {
        return c == 0 ? r.name : std::string();
    });
    grid.SetComboColumn(
        1, [](int, const PodRow&) { return std::vector<std::string>{"Open", "Close"}; },
        [](int, const PodRow& r) { return r.mode; }, [&](int row, int v) { rows[row].mode = v; });
    grid.SetIntColumn(
        2, [](int, const PodRow& r) { return r.lots; }, [&](int row, int v) { rows[row].lots = v; });
    grid.SetFloatColumn(
        3, [](int, const PodRow& r) { return r.price; },
        [&](int row, float v) { rows[row].price = v; }, "%.2f");
    grid.SetButtonColumn(
        4, [](int, const PodRow& r) { return r.running ? "Stop" : "Start"; },
        [&](int row) { rows[row].running = !rows[row].running; });
    // Frozen-when-running: row 1 is running → its editors render as static text.
    grid.SetRowReadOnly([](int, const PodRow& r) { return r.running; });

    EXPECT_NO_THROW(grid.Render());
}

TEST_F(EditableDataGridTest, EmptyAndReadOnlyAllRows) {
    std::vector<PodRow> rows = {{"X", 0, 2, 1.f, true}};
    EditableDataGrid<PodRow> grid("g2", {{"Name", 80}, {"Lots", 70}});
    grid.SetDataSource(&rows);
    grid.SetCellFormatter([](int, int, const PodRow& r) { return r.name; });
    grid.SetIntColumn(
        1, [](int, const PodRow& r) { return r.lots; }, [&](int row, int v) { rows[row].lots = v; });
    grid.SetRowReadOnly([](int, const PodRow&) { return true; }); // all frozen
    EXPECT_NO_THROW(grid.Render());

    std::vector<PodRow> empty;
    EditableDataGrid<PodRow> g3("g3", {{"Name", 80}});
    g3.SetDataSource(&empty);
    g3.SetEmptyText("no pods");
    EXPECT_NO_THROW(g3.Render());
}
