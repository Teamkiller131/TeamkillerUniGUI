#pragma once

// ─────────────────────────────────────────────────────────────────────────────
// UniGUI Widget Factory Helpers  (namespace unigui)
//
// Reduce the boilerplate of constructing retained-mode widgets:
//
//     auto btn = unigui::Make<unigui::Button>("save", "Save");      // explicit name
//     auto lbl = unigui::Make<unigui::Label>("Read-only");          // auto name
//
// `Make<T>` forwards its arguments to T's constructor and returns a
// std::shared_ptr<T>. The `MakeNamed<T>` overload prepends an auto-generated,
// process-unique name so callers no longer have to invent unique id strings for
// every widget instance (UniGUI widgets still scope their ImGui IDs by name).
// ─────────────────────────────────────────────────────────────────────────────

#include <atomic>
#include <memory>
#include <string>
#include <utility>

namespace unigui {

/// Monotonic, thread-safe counter backing the auto-generated widget names.
inline std::string NextAutoName(const char* prefix = "w") {
    static std::atomic<unsigned long long> counter{0};
    return std::string(prefix) + "##auto" +
           std::to_string(counter.fetch_add(1, std::memory_order_relaxed));
}

/// Construct a widget of type T with an explicit name (forwarded as the first
/// constructor argument, matching every UniGUI widget's signature).
template <class T, class... Args>
std::shared_ptr<T> Make(std::string name, Args&&... args) {
    return std::make_shared<T>(std::move(name), std::forward<Args>(args)...);
}

/// Construct a widget of type T with an auto-generated unique name. Use when the
/// instance does not need a stable, human-chosen id.
template <class T, class... Args>
std::shared_ptr<T> MakeNamed(Args&&... args) {
    return std::make_shared<T>(NextAutoName(), std::forward<Args>(args)...);
}

}  // namespace unigui
