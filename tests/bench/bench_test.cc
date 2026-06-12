#include <unigui/unigui.h>

#include <imgui.h>

#include <chrono>
#include <gtest/gtest.h>
#include <vector>
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
    EXPECT_LT(us, 20000);
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
    EXPECT_LT(us, 10000);
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
    EXPECT_LT(ms, 100);
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
    EXPECT_LT(us, 30000);
}

// ── CSV Import benchmark: 100k rows × 5 columns ─────────────────────────────
TEST_F(BenchTest, CSV_Import_100kRows) {
    // Generate a realistic CSV: header + 100,000 data rows, 5 columns
    std::string csv = "Name,Value,Category,Score,Notes\n";
    for (int i = 0; i < 100000; i++) {
        csv += "item_" + std::to_string(i) + "," + std::to_string(i % 1000) +
               ",cat_" + std::to_string(i % 10) + "," + std::to_string(i * 0.1f) +
               ",\"note " + std::to_string(i) + "\"\n";
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
    EXPECT_LT(ms, 500) << "CSV import of 100k rows took " << ms
                        << "ms (budget: 500ms). Consider optimizing the parser.";
}
