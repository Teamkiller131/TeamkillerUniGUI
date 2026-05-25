#include <unigui/unigui.h>
#include <unigui/widgets/button.h>
#include <imgui.h>
#include <gtest/gtest.h>
#include <chrono>

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
    std::vector<unigui::Button> buttons;
    for (int i = 0; i < 100; i++) buttons.emplace_back("btn" + std::to_string(i), "B" + std::to_string(i));
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& b : buttons) b.Render();
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    EXPECT_LT(us, 5000); // < 5ms for 100 buttons
}

TEST_F(BenchTest, FrameTime_100Labels) {
    std::vector<unigui::Label> labels;
    for (int i = 0; i < 100; i++) labels.emplace_back("lbl" + std::to_string(i), "Label " + std::to_string(i));
    auto start = std::chrono::high_resolution_clock::now();
    for (auto& l : labels) l.Render();
    auto end = std::chrono::high_resolution_clock::now();
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    EXPECT_LT(us, 3000); // < 3ms for 100 labels
}
