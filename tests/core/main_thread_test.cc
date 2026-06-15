#include <unigui/core/main_thread.h>

#include <gtest/gtest.h>

#include <memory>

using namespace unigui;

// Drain any tasks left by other tests so counts are deterministic.
class MainThreadTest : public ::testing::Test {
protected:
    void SetUp() override { ProcessMainThreadTasks(); }
    void TearDown() override { ProcessMainThreadTasks(); }
};

TEST_F(MainThreadTest, Invoke_QueuesAndDrains) {
    int n = 0;
    InvokeOnMainThread([&] { ++n; });
    EXPECT_EQ(PendingMainThreadTasks(), 1u);
    EXPECT_EQ(n, 0); // not run until drained
    ProcessMainThreadTasks();
    EXPECT_EQ(n, 1);
    EXPECT_EQ(PendingMainThreadTasks(), 0u);
}

TEST_F(MainThreadTest, WeakInvoke_RunsWhenTokenAlive) {
    auto token = MakeLifetimeToken();
    int n = 0;
    WeakInvokeOnMainThread(std::weak_ptr<void>(token), [&] { ++n; });
    ProcessMainThreadTasks();
    EXPECT_EQ(n, 1);
}

TEST_F(MainThreadTest, WeakInvoke_DroppedWhenTokenExpired) {
    auto token = MakeLifetimeToken();
    std::weak_ptr<void> weak = token;
    int n = 0;
    WeakInvokeOnMainThread(weak, [&] { ++n; });
    token.reset(); // owner destroyed before the task drains
    ProcessMainThreadTasks();
    EXPECT_EQ(n, 0); // task silently cancelled
}

TEST_F(MainThreadTest, WeakInvoke_OneOfTwoCancelled) {
    auto a = MakeLifetimeToken();
    auto b = MakeLifetimeToken();
    int na = 0, nb = 0;
    WeakInvokeOnMainThread(std::weak_ptr<void>(a), [&] { ++na; });
    WeakInvokeOnMainThread(std::weak_ptr<void>(b), [&] { ++nb; });
    a.reset();
    ProcessMainThreadTasks();
    EXPECT_EQ(na, 0);
    EXPECT_EQ(nb, 1);
}
