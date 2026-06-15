#pragma once
#include <cstddef>
#include <functional>
#include <memory>

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

/// Opaque lifetime token. Hold one as a member of an object that posts UI work
/// from a worker thread; pass it (or a weak handle) to WeakInvokeOnMainThread so
/// a queued task is silently dropped once the owner is destroyed — replacing the
/// hand-rolled `shared_ptr<atomic<bool>> alive_` guard pattern.
using LifetimeToken = std::shared_ptr<void>;

/// Create a fresh lifetime token.
inline LifetimeToken MakeLifetimeToken() {
    return std::make_shared<int>(0);
}

/// Like InvokeOnMainThread, but the task runs at drain time only if `alive` has
/// not expired. Capture a weak handle (`std::weak_ptr<void>(token)`) from the
/// worker thread; destroying the token (e.g. in the owner's destructor) cancels
/// any still-queued task.
void WeakInvokeOnMainThread(std::weak_ptr<void> alive, std::function<void()> fn);

} // namespace unigui
