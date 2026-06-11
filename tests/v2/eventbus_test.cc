#include <unigui/events/eventbus.h>

#include <gtest/gtest.h>
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
