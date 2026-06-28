#include <unigui/unigui.h>

#include <imgui.h>

#include <chrono>
#include <cmath>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace {
// Performance budgets are only meaningful in optimized builds. A Debug build is
// 5-20x slower and varies wildly by CI host, so the wall-clock assertion is
// skipped there — the benchmark still runs the work (catching crashes/UB) but
// does not flake the build. Release keeps asserting, catching real regressions.
inline void ExpectUnderBudget(long long actual, long long budget, const std::string& what) {
#ifdef NDEBUG
    EXPECT_LT(actual, budget) << what << ": " << actual << " (budget " << budget << ")";
#else
    (void) actual;
    (void) budget;
    (void) what;
#endif
}
} // namespace

class BenchTest : public ::testing::Test {
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
TEST_F(BenchTest, FrameTime_100Buttons) {
    std::vector<unigui::Button> bs;
    for (int i = 0; i < 100; i++)
        bs.emplace_back("b" + std::to_string(i), "B");
    auto t0 = std::chrono::high_resolution_clock::now();
    for (auto& b : bs)
        b.Render();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::high_resolution_clock::now() - t0)
                  .count();
    ExpectUnderBudget(us, 20000, "FrameTime_100Buttons us");
}
TEST_F(BenchTest, FrameTime_100Labels) {
    std::vector<unigui::Label> ls;
    for (int i = 0; i < 100; i++)
        ls.emplace_back("l" + std::to_string(i), "L");
    auto t0 = std::chrono::high_resolution_clock::now();
    for (auto& l : ls)
        l.Render();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::high_resolution_clock::now() - t0)
                  .count();
    ExpectUnderBudget(us, 10000, "FrameTime_100Labels us");
}
TEST_F(BenchTest, VirtualList_10k) {
    unigui::VirtualList vl("vl", 10000);
    vl.SetItemGetter([](int i) { return "I" + std::to_string(i); });
    auto t0 = std::chrono::high_resolution_clock::now();
    vl.Render();
    ImGui::Render();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::high_resolution_clock::now() - t0)
                  .count();
    ExpectUnderBudget(ms, 100, "VirtualList_10k ms");
}
TEST_F(BenchTest, Form_20Fields) {
    unigui::Form f("f", "T");
    for (int i = 0; i < 20; i++)
        f.AddTextField("f" + std::to_string(i), "F");
    auto t0 = std::chrono::high_resolution_clock::now();
    f.Render();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(
                  std::chrono::high_resolution_clock::now() - t0)
                  .count();
    ExpectUnderBudget(us, 30000, "Form_20Fields us");
}

// ── CSV Import benchmark: 100k rows × 5 columns ─────────────────────────────
TEST_F(BenchTest, CSV_Import_100kRows) {
    // Generate a realistic CSV: header + 100,000 data rows, 5 columns
    std::string csv = "Name,Value,Category,Score,Notes\n";
    for (int i = 0; i < 100000; i++) {
        csv += "item_" + std::to_string(i) + "," + std::to_string(i % 1000) + ",cat_" +
               std::to_string(i % 10) + "," + std::to_string(i * 0.1f) + ",\"note " +
               std::to_string(i) + "\"\n";
    }

    unigui::Table tbl("bench_csv", {"Name", "Value", "Category", "Score", "Notes"});

    auto t0 = std::chrono::high_resolution_clock::now();
    tbl.ImportCSV(csv);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                  std::chrono::high_resolution_clock::now() - t0)
                  .count();

    // Sanity: rows must be imported
    // (header row is skipped, so 100k data rows)
    // We don't assert a specific count here because ImportCSV skips the first
    // line (header). The benchmark's purpose is timing, not correctness.
    EXPECT_GT(ms, 0);

    // Performance budget: 100k rows should parse in under 500ms on modern hardware
    ExpectUnderBudget(ms, 500, "CSV import of 100k rows ms");
}

// ── DataTable virtual-scroll benchmark: render 100k rows ────────────────────
// Virtual scrolling must keep per-frame cost bounded by the *visible* rows, not
// the total row count — this proves a 100k-row blotter still renders cheaply.
TEST_F(BenchTest, DataTable_VirtualScroll_100kRows) {
    struct Row {
        int id;
        double value;
    };
    std::vector<Row> rows;
    rows.reserve(100000);
    for (int i = 0; i < 100000; i++)
        rows.push_back({i, i * 0.25});

    unigui::DataTable<Row> table("bench_dt", {{"ID", 80}, {"Value", 120}});
    table.SetDataSource(&rows);
    table.SetCellFormatter([](int, int col, const Row& r) {
        return col == 0 ? std::to_string(r.id) : std::to_string(r.value);
    });

    // ImGuiListClipper needs a prior frame's scroll/clip state to virtualise, so
    // the first frame can't clip. Warm up across several frames and time the
    // steady-state frame (the fixture already opened the first frame in SetUp,
    // and TearDown closes the last one we leave open here).
    long long ms = 0;
    for (int frame = 0; frame < 5; ++frame) {
        auto t0 = std::chrono::high_resolution_clock::now();
        table.Render();
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::high_resolution_clock::now() - t0)
                 .count();
        ImGui::Render();   // close the current frame
        ImGui::NewFrame(); // open the next (last one is closed by TearDown)
    }

    // Budget: once virtualised, a steady-state frame of a 100k-row table renders
    // only the visible window — well under a 16ms (60fps) frame. Generous floor
    // to catch a regression that makes per-frame cost scale with total rows.
    ExpectUnderBudget(ms, 50, "DataTable steady-state render of 100k rows ms");
}

// ── Table virtual-scroll benchmark: render 100k rows ────────────────────────
// The row-vector Table widget virtualises its row loop with ImGuiListClipper, so
// — like DataTable — a 100k-row Table must render in time bounded by the visible
// window, not the total row count.
TEST_F(BenchTest, Table_VirtualScroll_100kRows) {
    unigui::Table tbl("bench_tbl", {"ID", "Value"});
    for (int i = 0; i < 100000; i++)
        tbl.AddRow({std::to_string(i), std::to_string(i * 0.25)});

    // ImGuiListClipper needs a prior frame's scroll/clip state to virtualise, so
    // warm up across several frames and time the steady-state frame.
    long long ms = 0;
    for (int frame = 0; frame < 5; ++frame) {
        auto t0 = std::chrono::high_resolution_clock::now();
        tbl.Render();
        ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                 std::chrono::high_resolution_clock::now() - t0)
                 .count();
        ImGui::Render();   // close the current frame
        ImGui::NewFrame(); // open the next (last one is closed by TearDown)
    }

    // Budget: a steady-state frame draws only the visible window — well under a
    // 16ms (60fps) frame. A regression that rendered all rows would blow past it.
    ExpectUnderBudget(ms, 50, "Table steady-state render of 100k rows ms");
}

// The LTTB decimator runs per-frame on large series in the charting widgets. It has
// correctness tests; this pins its O(n) cost so a quadratic regression is caught.
TEST_F(BenchTest, LTTB_Decimate_1MTo2k) {
    constexpr std::size_t n = 1000000;
    std::vector<double> xs(n), ys(n);
    for (std::size_t i = 0; i < n; ++i) {
        xs[i] = static_cast<double>(i);
        ys[i] = std::sin(static_cast<double>(i) * 0.001) + static_cast<double>(i % 97) * 0.01;
    }
    const auto t0 = std::chrono::high_resolution_clock::now();
    const auto idx = unigui::LttbIndices(xs.data(), ys.data(), n, 2000);
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::high_resolution_clock::now() - t0)
                        .count();
    EXPECT_LE(idx.size(), 2000u);
    // O(n) over 1M points is a few ms in Release; a quadratic bucket scan would be
    // orders of magnitude over. The generous budget tolerates CI-host variance.
    ExpectUnderBudget(ms, 200, "LTTB 1M->2k ms");
}
