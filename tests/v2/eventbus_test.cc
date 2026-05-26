#include <unigui/v2/eventbus.h>
#include <gtest/gtest.h>
#include <string>
using namespace unigui::v2;

TEST(EventBusTest, Subscribe_Publish_Delivers) {
    std::string received;
    auto id = EventBus::Instance().Subscribe("test.topic", [&](auto& e){ received = std::any_cast<std::string>(e); });
    EventBus::Instance().Publish("test.topic", std::string("hello"));
    EXPECT_EQ(received, "hello");
    EventBus::Instance().Unsubscribe(id);
}

TEST(EventBusTest, Wildcard_Matches) {
    int count = 0;
    auto id = EventBus::Instance().Subscribe("window.*", [&](auto&){ count++; });
    EventBus::Instance().Publish("window.close", int{1});
    EventBus::Instance().Publish("window.resize", int{2});
    EXPECT_EQ(count, 2);
    EventBus::Instance().Unsubscribe(id);
}

TEST(EventBusTest, Unsubscribe_StopsDelivery) {
    std::string received;
    auto id = EventBus::Instance().Subscribe("x.y", [&](auto& e){ received=std::any_cast<std::string>(e); });
    EventBus::Instance().Unsubscribe(id);
    EventBus::Instance().Publish("x.y", std::string("nope"));
    EXPECT_EQ(received, "");
}

TEST(EventBusTest, SubscribeAll_Wildcard) {
    int count = 0;
    auto id = EventBus::Instance().SubscribeAll([&](auto&){ count++; });
    EventBus::Instance().Publish("a.b", int{1});
    EventBus::Instance().Publish("c.d", int{2});
    EXPECT_EQ(count, 2);
    EventBus::Instance().Unsubscribe(id);
}
