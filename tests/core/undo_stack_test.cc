#include <unigui/unigui.h>
#include <unigui/core/undo_stack.h>
#include <gtest/gtest.h>
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
    stack.Execute([](){});
    stack.Clear();
    EXPECT_FALSE(stack.CanUndo());
}
