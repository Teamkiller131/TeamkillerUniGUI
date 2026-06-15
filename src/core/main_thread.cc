#include <unigui/core/log.h>
#include <unigui/core/main_thread.h>

#include <mutex>
#include <vector>

namespace unigui {

static std::vector<std::function<void()>> g_pendingTasks;
static std::mutex g_taskMutex;

void InvokeOnMainThread(std::function<void()> fn) {
    std::lock_guard lock(g_taskMutex);
    g_pendingTasks.push_back(std::move(fn));
}

void ProcessMainThreadTasks() {
    std::vector<std::function<void()>> tasks;
    {
        std::lock_guard lock(g_taskMutex);
        tasks.swap(g_pendingTasks);
    }
    for (auto& fn : tasks) {
        try {
            fn();
        } catch (...) { UNIGUI_LOG_WARN("MainThread task threw exception"); }
    }
}

size_t PendingMainThreadTasks() {
    std::lock_guard lock(g_taskMutex);
    return g_pendingTasks.size();
}

void WeakInvokeOnMainThread(std::weak_ptr<void> alive, std::function<void()> fn) {
    InvokeOnMainThread([alive = std::move(alive), fn = std::move(fn)]() {
        if (!alive.expired())
            fn();
    });
}

} // namespace unigui
