/// Fuzz test: random widget creation and rendering to detect crashes.
#include <unigui/unigui.h>

#include <imgui.h>

#include <functional>
#include <gtest/gtest.h>
#include <random>
#include <vector>

class FuzzTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1280, 800);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(FuzzTest, RandomWidgetSequence_100Iterations) {
    std::mt19937 rng(42);
    std::uniform_int_distribution<int> widgetSelect(0, 10);

    for (int iter = 0; iter < 100; iter++) {
        ImGui::NewFrame();
        int w = widgetSelect(rng);
        switch (w) {
        case 0: {
            unigui::Button("bt", "B");
            unigui::Button("bt", "B").Render();
            break;
        }
        case 1: {
            unigui::CheckBox("cb", "C");
            unigui::CheckBox("cb", "C").Render();
            break;
        }
        case 2: {
            unigui::LineEdit("le", "L");
            unigui::LineEdit("le", "L").Render();
            break;
        }
        case 3: {
            unigui::ProgressBar("pb", 0.5f);
            unigui::ProgressBar("pb", 0.5f).Render();
            break;
        }
        case 4: {
            unigui::Slider<float> sl("sl", "S", 0, 100, 50);
            sl.Render();
            break;
        }
        case 5: {
            unigui::ComboBox("cmb", "C", {"A", "B", "C"});
            unigui::ComboBox("cmb", "C", {"A", "B", "C"}).Render();
            break;
        }
        case 6: {
            unigui::Label("lbl", "Hello");
            unigui::Label("lbl", "Hello").Render();
            break;
        }
        case 7: {
            unigui::GroupBox("gb", "G");
            break;
        }
        case 8: {
            unigui::MultiLine("ml", "text");
            unigui::MultiLine("ml", "text").Render();
            break;
        }
        case 9: {
            unigui::Tag("tg", "tag");
            unigui::Tag("tg", "tag").Render();
            break;
        }
        case 10: {
            unigui::Separator("sep");
            unigui::Separator("sep").Render();
            break;
        }
        }
        ImGui::Render();
        SUCCEED(); // No crash = pass
    }
}

TEST_F(FuzzTest, AllNewWidgets_RenderNoCrash) {
    ImGui::NewFrame();
    // v1.2 widgets
    unigui::VirtualList("vl", 10).Render();
    unigui::MultiCombo("mc", "M", {"A", "B"}).Render();
    unigui::PropertyGrid("pg").Render();
    unigui::SearchBox("sb", "Search").Render();
    unigui::Toast::Info("test");
    unigui::Toast::Instance().Render();
    unigui::PasswordInput("pi", "Pwd").Render();
    unigui::Wizard("wz").Render();
    ImGui::Render();
    SUCCEED();
}
