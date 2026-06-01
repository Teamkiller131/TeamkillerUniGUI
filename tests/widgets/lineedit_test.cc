#include <unigui/unigui.h>
#include <unigui/widgets/lineedit.h>
#include <imgui.h>
#include <gtest/gtest.h>
#include <cstring>

class LineEditTest : public ::testing::Test {
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

// 1. Defaults to empty
TEST_F(LineEditTest, DefaultsToEmpty) {
    unigui::LineEdit le("le", "Name");
    EXPECT_EQ(le.GetValue(), "");
}

// 2. SetValue works
TEST_F(LineEditTest, SetValue_Works) {
    unigui::LineEdit le("le", "Name");
    le.SetValue("Alice");
    EXPECT_EQ(le.GetValue(), "Alice");
}

// 3. HasError defaults to false
TEST_F(LineEditTest, HasError_DefaultsToFalse) {
    unigui::LineEdit le("le", "Email");
    EXPECT_FALSE(le.HasError());
}

// 4. Render doesn't crash
TEST_F(LineEditTest, Render_DoesNotCrash) {
    unigui::LineEdit le("le", "Field");
    le.SetPlaceholder("Enter text...");
    le.Render();
}

// 5. Undo/Redo works
TEST_F(LineEditTest, Undo_Redo_Works) {
    unigui::LineEdit le("le", "Field");
    le.SetValue("A");
    le.SetValue("B");
    EXPECT_EQ(le.GetValue(), "B");
    EXPECT_TRUE(le.CanUndo());
    le.Undo();
    EXPECT_EQ(le.GetValue(), "A");
    EXPECT_TRUE(le.CanRedo());
    le.Redo();
    EXPECT_EQ(le.GetValue(), "B");
}

// 6. CanUndo defaults to false
TEST_F(LineEditTest, CanUndo_DefaultsToFalse) {
    unigui::LineEdit le("le", "Field");
    EXPECT_FALSE(le.CanUndo());
    le.SetValue("new");
    EXPECT_TRUE(le.CanUndo());
}

// 7. Empty string set and get
TEST_F(LineEditTest, EmptyString_Roundtrip) {
    unigui::LineEdit le("le", "Field");
    le.SetValue("");
    EXPECT_EQ(le.GetValue(), "");
    le.SetValue("data");
    le.SetValue("");
    EXPECT_EQ(le.GetValue(), "");
}

// 8. Multiline mode renders without crash
TEST_F(LineEditTest, Multiline_Mode_DoesNotCrash) {
    unigui::LineEdit le("le", "Desc");
    le.SetMultiline(true);
    le.SetValue("Line 1\nLine 2\nLine 3");
    le.Render();
}

// 9. Password mode renders without crash
TEST_F(LineEditTest, Password_Mode_DoesNotCrash) {
    unigui::LineEdit le("le", "Password");
    le.SetPasswordMode(true);
    le.SetValue("secret123");
    le.Render();
}

// 10. ReadOnly mode renders without crash
TEST_F(LineEditTest, ReadOnly_Mode_DoesNotCrash) {
    unigui::LineEdit le("le", "ReadOnly");
    le.SetReadOnly(true);
    le.SetValue("cannot edit");
    le.Render();
}

// 11. MaxLength setter
TEST_F(LineEditTest, SetMaxLength_DoesNotCrash) {
    unigui::LineEdit le("le", "Limited");
    le.SetMaxLength(10);
    le.SetValue("short");
    le.Render();
}

// 12. Undo stack depth tracking
TEST_F(LineEditTest, UndoStackDepth_TracksChanges) {
    unigui::LineEdit le("le", "Field");
    le.SetValue("v1");
    le.SetValue("v2");
    le.SetValue("v3");
    // Can undo multiple times
    EXPECT_TRUE(le.CanUndo());
    le.Undo();
    EXPECT_EQ(le.GetValue(), "v2");
    EXPECT_TRUE(le.CanUndo());
    le.Undo();
    EXPECT_EQ(le.GetValue(), "v1");
    EXPECT_TRUE(le.CanUndo());
    le.Undo();
    EXPECT_EQ(le.GetValue(), "");
    EXPECT_FALSE(le.CanUndo());
}

// 13. CanRedo tracks redo depth
TEST_F(LineEditTest, CanRedo_TracksRedoDepth) {
    unigui::LineEdit le("le", "Field");
    le.SetValue("A");
    le.SetValue("B");
    le.SetValue("C");
    le.Undo(); // B
    le.Undo(); // A
    EXPECT_TRUE(le.CanRedo());
    le.Redo(); // B
    EXPECT_EQ(le.GetValue(), "B");
    EXPECT_TRUE(le.CanRedo());
    le.Redo(); // C
    EXPECT_EQ(le.GetValue(), "C");
    EXPECT_FALSE(le.CanRedo());
}

// 14. Validator with valid input
TEST_F(LineEditTest, Validator_AcceptsValid) {
    unigui::LineEdit le("le", "Email");
    le.SetValidator([](const std::string& s) { return !s.empty(); });
    le.SetValue("user@example.com");
    le.Render();
    // Validator doesn't reject valid input — HasError should be false
    EXPECT_FALSE(le.HasError());
}

// 15. Placeholder roundtrip
TEST_F(LineEditTest, Placeholder_SetAndRender) {
    unigui::LineEdit le("le", "Field");
    le.SetPlaceholder("Enter name...");
    le.Render();
    // Placeholder persists across renders
    le.Render();
}
