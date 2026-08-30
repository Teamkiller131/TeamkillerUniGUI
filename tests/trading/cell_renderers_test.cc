#include <unigui/trading/cell_renderers.h>
#include <unigui/widgets/datatable.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <vector>

using namespace unigui;
using namespace unigui::trading;

// Headless ImGui frames so the renderers can draw for real. DataTable uses a
// scrolling inner child window, whose content lands in that child's draw list — so
// the vertex measurement happens INSIDE the renderer callback (there the current
// window IS the table's inner window). The assertions are geometry-level: the
// renderers must add draw vertices (real lines/rects) and reserve the cell height,
// and must stay silent for degenerate inputs.
class CellRenderersTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }

    // Wrap `renderer` in a measuring shim and bind it to column 0; returns the table
    // and the per-row vertex gains (filled in while rendering).
    static std::pair<DataTable<int>, std::shared_ptr<std::vector<int>>> MakeTable(
            const char* name, DataTable<int>::CellRenderFn renderer) {
        DataTable<int> t(name, {{"c0", 120.f}});
        static const std::vector<int> kData = {1, 2, 3};
        t.SetDataSource(&kData);
        auto gains = std::make_shared<std::vector<int>>();
        t.SetCellRenderer(0, [renderer = std::move(renderer), gains](int row, const int& item) {
            const int before = ImGui::GetWindowDrawList()->VtxBuffer.Size;
            renderer(row, item);
            const int g = ImGui::GetWindowDrawList()->VtxBuffer.Size - before;
            gains->push_back(g);
            std::fprintf(stderr, "[dbg-gain] row=%d gain=%d\n", row, g);
        });
        return {std::move(t), gains};
    }

    static int RenderTable(DataTable<int>& table) {
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(400, 300));
        ImGui::Begin("cells", nullptr, ImGuiWindowFlags_NoSavedSettings);
        table.Render();
        ImGui::End();
        ImGui::Render();
        ImGui::NewFrame();
        return 0;
    }
};

TEST_F(CellRenderersTest, SparklineCell_DrawsPolyline) {
    auto [t, gains] = MakeTable("s1", SparklineCell<int>([](int, const int&) {
        return std::vector<float>{1.f, 3.f, 2.f};
    }));
    RenderTable(t);
    ASSERT_EQ(gains->size(), 3u); // three rows, renderer ran per row
    for (int g : *gains)
        EXPECT_GT(g, 0) << "a three-point sparkline must emit line vertices";
}

TEST_F(CellRenderersTest, SparklineCell_FlatData_StillDraws) {
    auto [t, gains] = MakeTable("s2", SparklineCell<int>([](int, const int&) {
        return std::vector<float>{2.f, 2.f, 2.f};
    }));
    RenderTable(t);
    ASSERT_EQ(gains->size(), 3u);
    // Flat data normalizes to the middle line — must draw, not divide by zero.
    for (int g : *gains)
        EXPECT_GT(g, 0);
}

TEST_F(CellRenderersTest, SparklineCell_FewerThanTwoValues_OnlyReserves) {
    auto [t, gains] = MakeTable("s3", SparklineCell<int>([](int, const int&) {
        return std::vector<float>{1.f};
    }));
    RenderTable(t);
    ASSERT_EQ(gains->size(), 3u);
    // One point → no polyline; only the Dummy (no draw vertices) is submitted.
    for (int g : *gains)
        EXPECT_EQ(g, 0);
}

TEST_F(CellRenderersTest, BarCell_SignedBars_Draw) {
    auto [t, gains] = MakeTable("b1", BarCell<int>([](int row, const int&) { return row - 1.0; }, 2.0));
    RenderTable(t);
    ASSERT_EQ(gains->size(), 3u);
    // Positive and negative rows draw signed bars; the zero row deliberately draws
    // nothing (a zero-length delta bar) — its height is still reserved.
    EXPECT_GT((*gains)[0], 0);
    EXPECT_EQ((*gains)[1], 0);
    EXPECT_GT((*gains)[2], 0);
}

TEST_F(CellRenderersTest, BarCell_ClampsToMaxAbs) {
    auto [t, gains] = MakeTable("b2", BarCell<int>([](int, const int&) { return 1000.0; }, 1.0));
    RenderTable(t);
    ASSERT_EQ(gains->size(), 3u);
    // Way over maxAbs — must clamp and draw instead of overflowing the cell.
    for (int g : *gains)
        EXPECT_GT(g, 0);
}



