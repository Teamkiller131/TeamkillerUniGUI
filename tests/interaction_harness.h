// Shared harness for interaction-driven tests (Dear ImGui test engine + GoogleTest).
//
// The engine clicks/types/navigates through ImGui's input queue against real UniGUI
// widgets — behavior tests, not render smoke. Only compiled when UNIGUI_TEST_ENGINE=1
// (the `testengine` vcpkg manifest feature; see the windows-msvc-debug-testengine
// preset and the linux-testengine CI lane).
//
// Usage:
//   class MyTest : public itest::InteractionFixture {};
//   TEST_F(MyTest, Clicks) {
//       unigui::Button b("n", "Save");
//       auto st = Run("case_name",
//                     [&] { b.Render(); },                       // GUI, drawn in window "TW"
//                     [&](ImGuiTestContext* ctx) {               // driver (may capture!)
//                         ctx->SetRef("TW");
//                         ctx->ItemClick("**/Save");
//                     });
//       EXPECT_EQ(st, ImGuiTestStatus_Success);
//   }
#pragma once

#ifdef UNIGUI_TEST_ENGINE

#include <imgui.h>
#include <imgui_te_context.h>
#include <imgui_te_engine.h>

#include <functional>
#include <gtest/gtest.h>

namespace itest {

// The engine's GuiFunc/TestFunc are plain function pointers (no captures), so the
// current test's GUI body and driver are bridged through file-static std::functions.
inline std::function<void()> g_gui;
inline std::function<void(ImGuiTestContext*)> g_drive;

inline void GuiThunk(ImGuiTestContext*) {
    ImGui::SetNextWindowPos(ImVec2(50, 50), ImGuiCond_Appearing);
    ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_Appearing);
    ImGui::Begin("TW", nullptr, ImGuiWindowFlags_NoSavedSettings);
    if (g_gui)
        g_gui();
    ImGui::End();
}

inline void DriveThunk(ImGuiTestContext* ctx) {
    if (g_drive)
        g_drive(ctx);
}

class InteractionFixture : public ::testing::Test {
protected:
    ImGuiTestEngine* engine_ = nullptr;

    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(1280, 720);
        io.DeltaTime = 1.0f / 60.0f;
        // Production parity: the app loop always enables keyboard nav (app.cc), and the
        // keyboard-navigation interaction tests depend on it.
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.Fonts->Build();

        engine_ = ImGuiTestEngine_CreateContext();
        ImGuiTestEngineIO& eio = ImGuiTestEngine_GetIO(engine_);
        eio.ConfigRunSpeed = ImGuiTestRunSpeed_Fast; // no real-time waits, headless
        eio.ConfigVerboseLevel = ImGuiTestVerboseLevel_Error;
        eio.ConfigVerboseLevelOnError = ImGuiTestVerboseLevel_Debug;
        ImGuiTestEngine_Start(engine_, ImGui::GetCurrentContext());
    }

    void TearDown() override {
        ImGuiTestEngine_Stop(engine_);
        // Engine must be destroyed AFTER the ImGui context it hooked (upstream app
        // examples use exactly this order).
        ImGui::DestroyContext();
        ImGuiTestEngine_DestroyContext(engine_);
        g_gui = nullptr;
        g_drive = nullptr;
    }

    // Register a test whose GUI is `gui` (drawn inside window "TW") and whose driver is
    // `drive`, run it to completion, and return its status. The engine steps its
    // coroutine one frame at a time from the NewFrame/PostSwap hooks.
    ImGuiTestStatus Run(const char* name, std::function<void()> gui,
                        std::function<void(ImGuiTestContext*)> drive, int maxFrames = 2000) {
        g_gui = std::move(gui);
        g_drive = std::move(drive);
        ImGuiTest* t = IM_REGISTER_TEST(engine_, "unigui", name);
        t->GuiFunc = GuiThunk;
        t->TestFunc = DriveThunk;
        ImGuiTestEngine_QueueTest(engine_, t, ImGuiTestRunFlags_None);
        while (!ImGuiTestEngine_IsTestQueueEmpty(engine_) && maxFrames-- > 0) {
            ImGui::NewFrame();
            ImGui::Render();
            ImGuiTestEngine_PostSwap(engine_);
        }
        EXPECT_GT(maxFrames, 0) << "engine did not finish within the frame budget";
        if (t->Output.Status != ImGuiTestStatus_Success) {
            // Surface the engine's own log (e.g. "Unable to locate item: ...") so a
            // failing ref path is diagnosable straight from the gtest output.
            ADD_FAILURE() << "test engine log for '" << name << "':\n"
                          << t->Output.Log.Buffer.c_str();
        }
        return t->Output.Status;
    }
};

} // namespace itest

#endif // UNIGUI_TEST_ENGINE
