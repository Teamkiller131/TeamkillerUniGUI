#pragma once
#include <cstddef>
#include <functional>

namespace unigui {

/// Cross-thread dispatcher.  Call from any thread to schedule UI work on main thread.
/// Must call ProcessMainThreadTasks() each frame in the render loop.
///
/// Usage (from network thread):
///   unigui::InvokeOnMainThread([&]() { positions.push_back(data); });
///
/// Usage (in main loop):
///   unigui::ProcessMainThreadTasks();  // drain pending callbacks

void InvokeOnMainThread(std::function<void()> fn);
void ProcessMainThreadTasks();
size_t PendingMainThreadTasks();

} // namespace unigui
