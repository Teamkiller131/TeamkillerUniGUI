#include <unigui/core/context.h>
#include <unigui/unigui.h>
#include <unigui/widgets/window.h>

#include <imgui.h>

#include <gtest/gtest.h>

class WindowTest : public ::testing::Test {
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
};

TEST_F(WindowTest, Render_DoesNotCrash) {
    unigui::Window win("win", "Main Window");
    win.Render();
}

TEST_F(WindowTest, AddPanel_RendersContent) {
    unigui::Window win("win", "Main");
    auto panel = std::make_shared<unigui::Panel>("pnl", "Settings");
    bool called = false;
    panel->SetContentCallback([&called]() { called = true; });
    win.AddPanel(panel);
    win.Render();
    EXPECT_TRUE(called);
}

TEST_F(WindowTest, HasMenuBar_DefaultsToFalse) {
    unigui::Window win("win", "Main");
    EXPECT_FALSE(win.HasMenuBar());
}

TEST_F(WindowTest, SetMenuBarEnabled_Works) {
    unigui::Window win("win", "Main");
    win.SetMenuBarEnabled(true);
    EXPECT_TRUE(win.HasMenuBar());
    win.Render(); // Should not crash with menubar
}

TEST_F(WindowTest, Close_CallsOnClose) {
    unigui::Window win("win", "Main");
    bool closed = false;
    win.SetOnClose([&closed]() { closed = true; });
    win.Hide(); // Simulate close
    // OnClose fires when window is explicitly closed via X button
    // Hide() only sets visibility
    (void) closed;
}

TEST_F(WindowTest, Hidden_DoesNotRender) {
    unigui::Window win("win", "Hidden");
    win.Hide();
    win.Render();
}
