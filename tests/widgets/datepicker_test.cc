#include <unigui/unigui.h>
#include <unigui/widgets/datepicker.h>

#include <imgui.h>

#include <gtest/gtest.h>
class DatePickerTest : public ::testing::Test {
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
TEST_F(DatePickerTest, GetDate_Works) {
    unigui::DatePicker dp("dp", "Date");
    auto d = dp.GetDate();
    EXPECT_EQ(d[0], 2026);
}
TEST_F(DatePickerTest, Render_DoesNotCrash) {
    unigui::DatePicker dp("dp", "Date");
    dp.Render();
}
