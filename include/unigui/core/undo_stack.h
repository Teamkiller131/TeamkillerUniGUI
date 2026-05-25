#pragma once
#include <vector>
#include <functional>
#include <stdexcept>

namespace unigui {

/// Command-pattern undo/redo stack.
/// Actions are user-defined lambdas: ()=>void for do/undo.
template<typename Action>
class UndoStack {
public:
    /// Execute an action and push it onto the undo stack.
    void Execute(Action&& action) {
        // Truncate redo tail
        if (index_ + 1 < (int)stack_.size()) stack_.resize(index_ + 1);
        stack_.push_back(std::move(action));
        index_ = (int)stack_.size() - 1;
        if ((int)stack_.size() > maxDepth_) { stack_.erase(stack_.begin()); index_--; }
        // Execute the action
        stack_[index_]();
    }

    /// Undo the last action. Returns true if an action was undone.
    bool Undo() {
        if (index_ < 0) return false;
        stack_[index_]();  // Re-execute (caller provides undo logic in action)
        index_--;
        return true;
    }

    /// Redo the last undone action. Returns true if an action was redone.
    bool Redo() {
        if (index_ + 1 >= (int)stack_.size()) return false;
        index_++;
        stack_[index_]();
        return true;
    }

    bool CanUndo() const { return index_ >= 0; }
    bool CanRedo() const { return index_ + 1 < (int)stack_.size(); }
    int Depth() const { return index_ + 1; }
    int RedoDepth() const { return (int)stack_.size() - index_ - 1; }
    void Clear() { stack_.clear(); index_ = -1; }
    void SetMaxDepth(int depth) { maxDepth_ = depth; }

private:
    std::vector<Action> stack_;
    int index_ = -1;
    int maxDepth_ = 100;
};

} // namespace unigui
