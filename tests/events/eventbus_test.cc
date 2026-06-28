#include <unigui/events/eventbus.h>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <gtest/gtest.h>
#include <mutex>
#include <stdexcept>
#include <string>
using namespace unigui::events;

TEST(BusTest, Subscribe_Publish_Delivers) {
    std::string received;
    auto id = Bus::Instance().Subscribe("test.topic",
                                        [&](auto& e) { received = std::any_cast<std::string>(e); });
    Bus::Instance().Publish("test.topic", std::string("hello"));
    EXPECT_EQ(received, "hello");
    Bus::Instance().Unsubscribe(id);
}

TEST(BusTest, Wildcard_Matches) {
    int count = 0;
    auto id = Bus::Instance().Subscribe("window.*", [&](auto&) { count++; });
    Bus::Instance().Publish("window.close", int{1});
    Bus::Instance().Publish("window.resize", int{2});
    EXPECT_EQ(count, 2);
    Bus::Instance().Unsubscribe(id);
}

TEST(BusTest, Unsubscribe_StopsDelivery) {
    std::string received;
    auto id = Bus::Instance().Subscribe("x.y",
                                        [&](auto& e) { received = std::any_cast<std::string>(e); });
    Bus::Instance().Unsubscribe(id);
    Bus::Instance().Publish("x.y", std::string("nope"));
    EXPECT_EQ(received, "");
}

TEST(BusTest, SubscribeAll_Wildcard) {
    int count = 0;
    auto id = Bus::Instance().SubscribeAll([&](auto&) { count++; });
    Bus::Instance().Publish("a.b", int{1});
    Bus::Instance().Publish("c.d", int{2});
    EXPECT_EQ(count, 2);
    Bus::Instance().Unsubscribe(id);
}

// ── Negative / edge cases for the wildcard matcher ────────────────────────────

TEST(BusTest, Wildcard_DifferentPrefix_NoMatch) {
    int count = 0;
    auto id = Bus::Instance().Subscribe("window.*", [&](auto&) { count++; });
    Bus::Instance().Publish("dialog.close", int{1});
    EXPECT_EQ(count, 0);
    Bus::Instance().Unsubscribe(id);
}

TEST(BusTest, Wildcard_StarAbsorbsDots_MatchesNested) {
    int count = 0;
    auto id = Bus::Instance().Subscribe("window.*", [&](auto&) { count++; });
    Bus::Instance().Publish("window.tab.close", int{1}); // '*' spans the extra dot
    EXPECT_EQ(count, 1);
    Bus::Instance().Unsubscribe(id);
}

TEST(BusTest, Wildcard_MiddleStar) {
    int hits = 0;
    auto id = Bus::Instance().Subscribe("a.*.c", [&](auto&) { hits++; });
    Bus::Instance().Publish("a.b.c", int{1});   // match
    Bus::Instance().Publish("a.xyz.c", int{1}); // match
    Bus::Instance().Publish("a.c", int{1});     // no middle segment → no match
    Bus::Instance().Publish("a.b.d", int{1});   // wrong suffix → no match
    EXPECT_EQ(hits, 2);
    Bus::Instance().Unsubscribe(id);
}

TEST(BusTest, ExactTopic_NoSubstringMatch) {
    int count = 0;
    auto id = Bus::Instance().Subscribe("win", [&](auto&) { count++; });
    Bus::Instance().Publish("window.close", int{1});
    EXPECT_EQ(count, 0);
    Bus::Instance().Unsubscribe(id);
}

TEST(BusTest, HandlerException_IsContained_OthersStillRun) {
    int after = 0;
    auto id1 = Bus::Instance().Subscribe("boom",
                                         [](auto&) { throw std::runtime_error("handler failed"); });
    auto id2 = Bus::Instance().Subscribe("boom", [&](auto&) { after++; });
    EXPECT_NO_THROW(Bus::Instance().Publish("boom", int{1}));
    EXPECT_EQ(after, 1); // exception in first handler must not stop the second
    Bus::Instance().Unsubscribe(id1);
    Bus::Instance().Unsubscribe(id2);
}

// ── RAII Subscription tests ─────────────────────────────────────────────────

TEST(BusTest, Scoped_DestructorUnsubscribes) {
    int count = 0;
    {
        auto sub = Bus::Instance().SubscribeScoped("scoped.a", [&](auto&) { count++; });
        Bus::Instance().Publish("scoped.a", int{1});
        EXPECT_EQ(count, 1);
    }
    // sub destroyed — handler should no longer fire
    Bus::Instance().Publish("scoped.a", int{2});
    EXPECT_EQ(count, 1);
}

TEST(BusTest, Scoped_MoveTransfersOwnership) {
    int count = 0;
    Subscription sub;
    {
        Subscription tmp;
        tmp = Bus::Instance().SubscribeScoped("scoped.b", [&](auto&) { count++; });
        sub = std::move(tmp);
        EXPECT_TRUE(sub.Valid());
        EXPECT_FALSE(tmp.Valid());
    }
    Bus::Instance().Publish("scoped.b", int{1});
    EXPECT_EQ(count, 1);
    sub.Unsubscribe();
}

TEST(BusTest, Scoped_UnsubscribeManual) {
    int count = 0;
    auto sub = Bus::Instance().SubscribeScoped("scoped.c", [&](auto&) { count++; });
    sub.Unsubscribe();
    EXPECT_FALSE(sub.Valid());
    Bus::Instance().Publish("scoped.c", int{1});
    EXPECT_EQ(count, 0);
}

TEST(BusTest, Scoped_DoubleUnsubscribe_NoCrash) {
    auto sub = Bus::Instance().SubscribeScoped("scoped.d", [](auto&) {});
    sub.Unsubscribe();
    EXPECT_NO_THROW(sub.Unsubscribe()); // no-op
}

TEST(BusTest, Scoped_HandlerUnsubscribesItself_NoDeadlock) {
    Subscription selfRef;
    int count = 0;
    // Handler captures selfRef by reference and unsubscribes itself
    selfRef = Bus::Instance().SubscribeScoped("scoped.e", [&](auto&) {
        count++;
        selfRef.Unsubscribe(); // unsubscribe from within handler
    });
    Bus::Instance().Publish("scoped.e", int{1});
    EXPECT_EQ(count, 1);
    // Second publish should not trigger the handler
    Bus::Instance().Publish("scoped.e", int{2});
    EXPECT_EQ(count, 1);
}

// ── Async worker thread (PublishAsync + Shutdown drain) ──────────────────────

TEST(BusAsyncTest, PublishAsync_EventuallyDelivers) {
    std::atomic<int> got{-1};
    std::mutex m;
    std::condition_variable cv;
    auto sub = Bus::Instance().SubscribeScoped("async.deliver", [&](const std::any& e) {
        {
            std::lock_guard<std::mutex> lk(m);
            got.store(std::any_cast<int>(e));
        }
        cv.notify_one();
    });
    Bus::Instance().PublishAsync("async.deliver", int{42});
    std::unique_lock<std::mutex> lk(m);
    EXPECT_TRUE(cv.wait_for(lk, std::chrono::seconds(2), [&] { return got.load() == 42; }));
    EXPECT_EQ(got.load(), 42);
}

TEST(BusAsyncTest, Shutdown_DrainsPendingEvents) {
    // A standalone bus so Shutdown() is deterministic and the shared singleton is
    // untouched. Every event published before Shutdown() must still be delivered.
    auto bus = Bus::CreateForTesting();
    std::atomic<int> delivered{0};
    bus->Subscribe("drain.test", [&](const std::any&) { delivered.fetch_add(1); });
    constexpr int kN = 50;
    for (int i = 0; i < kN; i++)
        bus->PublishAsync("drain.test", i);
    bus->Shutdown(); // drains the queue, then joins the worker
    EXPECT_EQ(delivered.load(), kN);
}

TEST(BusAsyncTest, Shutdown_IsIdempotent) {
    auto bus = Bus::CreateForTesting();
    bus->Shutdown();
    EXPECT_NO_THROW(bus->Shutdown()); // second call is a no-op (worker not joinable)
}
