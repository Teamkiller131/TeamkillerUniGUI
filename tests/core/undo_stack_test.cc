#include <unigui/core/undo_stack.h>
#include <unigui/unigui.h>

#include <functional>
#include <gtest/gtest.h>
#include <vector>
TEST(UndoStackTest, Execute_Undo_Redo) {
    int value = 0;
    unigui::UndoStack<std::function<void()>> stack;
    stack.Execute([&]() { value = 1; });
    EXPECT_EQ(value, 1);
    EXPECT_TRUE(stack.CanUndo());
    stack.Undo();
    EXPECT_EQ(value, 1);
    EXPECT_FALSE(stack.CanUndo());
    EXPECT_TRUE(stack.CanRedo());
    stack.Redo();
    EXPECT_EQ(value, 1);
}
TEST(UndoStackTest, Clear) {
    unigui::UndoStack<std::function<void()>> stack;
    stack.Execute([]() {});
    stack.Clear();
    EXPECT_FALSE(stack.CanUndo());
}

// ── Max-depth eviction: the oldest entries are dropped, indices stay valid ────
TEST(UndoStackTest, MaxDepth_EvictsOldest) {
    std::vector<int> ran;
    unigui::UndoStack<std::function<void()>> stack;
    stack.SetMaxDepth(2);
    for (int i = 0; i < 4; i++)
        stack.Execute([&ran, i] { ran.push_back(i); }); // ids 0..3 run on Execute
    // Only the two most recent (2, 3) are retained.
    EXPECT_EQ(stack.Depth(), 2);
    ran.clear();
    EXPECT_TRUE(stack.Undo());  // re-runs the top (id 3)
    EXPECT_TRUE(stack.Undo());  // then id 2
    EXPECT_FALSE(stack.Undo()); // nothing older survived eviction
    ASSERT_EQ(ran.size(), 2u);
    EXPECT_EQ(ran[0], 3); // most-recent undone first
    EXPECT_EQ(ran[1], 2);
}

// ── Redo tail is truncated when a new action is executed after an undo ────────
TEST(UndoStackTest, RedoTail_TruncatedOnNewExecute) {
    unigui::UndoStack<std::function<void()>> stack;
    stack.Execute([] {}); // A
    stack.Execute([] {}); // B
    EXPECT_TRUE(stack.Undo());
    EXPECT_EQ(stack.RedoDepth(), 1); // B is redoable
    EXPECT_TRUE(stack.CanRedo());
    stack.Execute([] {}); // C — must discard the redoable B
    EXPECT_FALSE(stack.CanRedo());
    EXPECT_FALSE(stack.Redo());
    EXPECT_EQ(stack.RedoDepth(), 0);
}

// ── RedoDepth tracks the undone tail length ──────────────────────────────────
TEST(UndoStackTest, RedoDepth_Tracks) {
    unigui::UndoStack<std::function<void()>> stack;
    stack.Execute([] {});
    stack.Execute([] {});
    stack.Execute([] {});
    EXPECT_EQ(stack.RedoDepth(), 0);
    stack.Undo();
    stack.Undo();
    EXPECT_EQ(stack.RedoDepth(), 2);
    stack.Redo();
    EXPECT_EQ(stack.RedoDepth(), 1);
}
