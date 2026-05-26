#include <unigui/events/eventbus.h>
#include <gtest/gtest.h>
#include <string>
using namespace unigui::events;

TEST(BusTest, Subscribe_Publish_Delivers) {
    std::string received;
    auto id = Bus::Instance().Subscribe("test.topic", [&](auto& e){ received = std::any_cast<std::string>(e); });
    Bus::Instance().Publish("test.topic", std::string("hello"));
    EXPECT_EQ(received, "hello");
    Bus::Instance().Unsubscribe(id);
}

TEST(BusTest, Wildcard_Matches) {
    int count = 0;
    auto id = Bus::Instance().Subscribe("window.*", [&](auto&){ count++; });
    Bus::Instance().Publish("window.close", int{1});
    Bus::Instance().Publish("window.resize", int{2});
    EXPECT_EQ(count, 2);
    Bus::Instance().Unsubscribe(id);
}

TEST(BusTest, Unsubscribe_StopsDelivery) {
    std::string received;
    auto id = Bus::Instance().Subscribe("x.y", [&](auto& e){ received=std::any_cast<std::string>(e); });
    Bus::Instance().Unsubscribe(id);
    Bus::Instance().Publish("x.y", std::string("nope"));
    EXPECT_EQ(received, "");
}

TEST(BusTest, SubscribeAll_Wildcard) {
    int count = 0;
    auto id = Bus::Instance().SubscribeAll([&](auto&){ count++; });
    Bus::Instance().Publish("a.b", int{1});
    Bus::Instance().Publish("c.d", int{2});
    EXPECT_EQ(count, 2);
    Bus::Instance().Unsubscribe(id);
}
