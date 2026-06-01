#include <unigui/unigui.h>
#include <unigui/widgets/dialog.h>
#include <imgui.h>
#include <gtest/gtest.h>

class DialogTest : public ::testing::Test {
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

// ── Core open/close ──────────────────────────────────────
TEST_F(DialogTest, IsOpen_DefaultsToFalse) {
    unigui::Dialog dlg("dlg", "Title", "Msg");
    EXPECT_FALSE(dlg.IsOpen());
}

TEST_F(DialogTest, Open_SetsOpen) {
    unigui::Dialog dlg("dlg", "Title", "Msg");
    dlg.Open();
    EXPECT_TRUE(dlg.IsOpen());
}

TEST_F(DialogTest, Close_SetsClosed) {
    unigui::Dialog dlg("dlg", "Title", "Msg");
    dlg.Open();
    dlg.Close();
    EXPECT_FALSE(dlg.IsOpen());
}

// ── Render does not crash ────────────────────────────────
TEST_F(DialogTest, Render_DoesNotCrash) {
    unigui::Dialog dlg("dlg2", "Greeting", "Hello World");
    dlg.Open();
    dlg.Render();
}

TEST_F(DialogTest, Render_WhenClosed_DoesNotCrash) {
    unigui::Dialog dlg("dlg3", "Hidden", "You should not see this");
    // Dialog is closed by default — render should still be safe
    dlg.Render();
}

// ── Re-open after close ─────────────────────────────────
TEST_F(DialogTest, Reopen_AfterClose) {
    unigui::Dialog dlg("dlg4", "Reopen", "Can reopen?");

    dlg.Open();
    EXPECT_TRUE(dlg.IsOpen());

    dlg.Close();
    EXPECT_FALSE(dlg.IsOpen());

    dlg.Open();
    EXPECT_TRUE(dlg.IsOpen());

    dlg.Render(); // Should not crash
}

// ── OK / Cancel callbacks ───────────────────────────────
TEST_F(DialogTest, OkCallback_IsSetAndRenderDoesNotCrash) {
    unigui::Dialog dlg("dlg5", "Confirm", "Are you sure?");
    bool ok_fired = false;
    dlg.SetOnOk([&]() { ok_fired = true; });
    dlg.Open();
    dlg.Render();
    // OK callback fires when the OK button is actually pressed in the UI;
    // just rendering does not auto-trigger it.
    EXPECT_FALSE(ok_fired);
}

TEST_F(DialogTest, CancelCallback_IsSetAndRenderDoesNotCrash) {
    unigui::Dialog dlg("dlg6", "Cancel Test", "Press cancel");
    bool cancel_fired = false;
    dlg.SetOnCancel([&]() { cancel_fired = true; });
    dlg.Open();
    dlg.Render();
    EXPECT_FALSE(cancel_fired);
}

TEST_F(DialogTest, CustomButtons_RenderDoesNotCrash) {
    unigui::Dialog dlg("dlg7", "Custom", "Custom buttons");
    dlg.SetButtons("Yes", "No");
    dlg.Open();
    dlg.Render();
}

// ── Modal vs non-modal (conceptual) ─────────────────────
// In unigui, Dialogs are implemented as ImGui popups/modals.
// The API treats every Dialog the same way — Open() shows it,
// Close() hides it. Modal behavior is controlled at the ImGui
// level via ImGuiWindowFlags_Modal. The unigui wrapper exposes
// a consistent interface.

TEST_F(DialogTest, MultipleDialogs_CanBeOpenedTogether) {
    unigui::Dialog dlgA("dlgA", "First", "Message A");
    unigui::Dialog dlgB("dlgB", "Second", "Message B");

    dlgA.Open();
    dlgB.Open();

    EXPECT_TRUE(dlgA.IsOpen());
    EXPECT_TRUE(dlgB.IsOpen());

    // Render both — no crash
    dlgA.Render();
    dlgB.Render();
}

TEST_F(DialogTest, OpenCloseToggle_Stress) {
    unigui::Dialog dlg("dlg8", "Toggle", "Stress test");

    for (int i = 0; i < 10; ++i) {
        dlg.Open();
        EXPECT_TRUE(dlg.IsOpen());
        dlg.Render();
        dlg.Close();
        EXPECT_FALSE(dlg.IsOpen());
    }
}
