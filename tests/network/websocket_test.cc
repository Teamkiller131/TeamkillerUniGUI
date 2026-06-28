#include <unigui/network/network.h>

#include <atomic>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace unigui::network;

// The IXWebSocket message handler runs on a background thread and reads the three
// callback std::functions; the OnMessage/OnOpen/OnClose setters may rewrite them from
// another thread. The setters are now mutex-guarded and the handler snapshots under
// the lock before invoking. These tests exercise the setter path under contention —
// run under ThreadSanitizer to actually catch a missing lock; without TSan they at
// least assert no deadlock/crash. No network connection is established.

TEST(WebSocketClientTest, ConstructDestruct_NoCrash) {
    EXPECT_NO_THROW({ WebSocketClient ws; });
}

TEST(WebSocketClientTest, ConcurrentSetters_NoCrashOrDeadlock) {
    WebSocketClient ws;
    std::atomic<bool> stop{false};
    std::atomic<int> spins{0};

    std::vector<std::thread> threads;
    for (int t = 0; t < 4; t++) {
        threads.emplace_back([&, t] {
            while (!stop.load(std::memory_order_relaxed)) {
                if (t % 3 == 0)
                    ws.OnMessage([](const std::string&) {});
                else if (t % 3 == 1)
                    ws.OnOpen([] {});
                else
                    ws.OnClose([] {});
                spins.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }

    // Let the setters race for a bounded number of iterations, then stop.
    while (spins.load(std::memory_order_relaxed) < 20000)
        std::this_thread::yield();
    stop.store(true, std::memory_order_relaxed);
    for (auto& th : threads)
        th.join();

    SUCCEED(); // reaching here without a crash/deadlock is the assertion
}
