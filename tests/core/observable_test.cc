#include <unigui/core/observable.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

using unigui::Bind;
using unigui::Observable;
using unigui::Subscription;

TEST(ObservableTest, GetReturnsInitialValue) {
    Observable<int> o{42};
    EXPECT_EQ(o.Get(), 42);
    EXPECT_EQ(static_cast<int>(o), 42); // implicit conversion
}

TEST(ObservableTest, SetNotifiesOnChange) {
    Observable<int> o{0};
    int seen = -1;
    int calls = 0;
    auto sub = o.Subscribe([&](const int& v) {
        seen = v;
        ++calls;
    });
    EXPECT_TRUE(o.Set(5));
    EXPECT_EQ(seen, 5);
    EXPECT_EQ(calls, 1);
}

TEST(ObservableTest, SetSameValueDoesNotNotify) {
    Observable<int> o{7};
    int calls = 0;
    auto sub = o.Subscribe([&](const int&) { ++calls; });
    EXPECT_FALSE(o.Set(7)); // unchanged
    EXPECT_EQ(calls, 0);
}

TEST(ObservableTest, AssignmentOperatorSets) {
    Observable<std::string> o{"a"};
    std::string seen;
    auto sub = o.Subscribe([&](const std::string& v) { seen = v; });
    o = std::string("b");
    EXPECT_EQ(o.Get(), "b");
    EXPECT_EQ(seen, "b");
}

TEST(ObservableTest, ForceSetNotifiesEvenWhenEqual) {
    Observable<int> o{3};
    int calls = 0;
    auto sub = o.Subscribe([&](const int&) { ++calls; });
    o.ForceSet(3);
    EXPECT_EQ(calls, 1);
}

TEST(ObservableTest, MutateInPlaceNotifies) {
    Observable<std::vector<int>> o{std::vector<int>{1, 2}};
    int calls = 0;
    auto sub = o.Subscribe([&](const std::vector<int>&) { ++calls; });
    o.Mutate([](std::vector<int>& v) { v.push_back(3); });
    EXPECT_EQ(o.Get().size(), 3u);
    EXPECT_EQ(calls, 1);
}

TEST(ObservableTest, MultipleObserversAllFire) {
    Observable<int> o{0};
    int a = 0, b = 0;
    auto s1 = o.Subscribe([&](const int& v) { a = v; });
    auto s2 = o.Subscribe([&](const int& v) { b = v; });
    EXPECT_EQ(o.ObserverCount(), 2u);
    o.Set(9);
    EXPECT_EQ(a, 9);
    EXPECT_EQ(b, 9);
}

TEST(ObservableTest, SubscriptionResetUnsubscribes) {
    Observable<int> o{0};
    int calls = 0;
    auto sub = o.Subscribe([&](const int&) { ++calls; });
    o.Set(1);
    EXPECT_EQ(calls, 1);
    sub.Reset();
    EXPECT_FALSE(sub.Active());
    EXPECT_EQ(o.ObserverCount(), 0u);
    o.Set(2);
    EXPECT_EQ(calls, 1); // no further notifications
}

TEST(ObservableTest, SubscriptionDestructorUnsubscribes) {
    Observable<int> o{0};
    int calls = 0;
    {
        auto sub = o.Subscribe([&](const int&) { ++calls; });
        o.Set(1);
    } // sub destroyed here
    EXPECT_EQ(o.ObserverCount(), 0u);
    o.Set(2);
    EXPECT_EQ(calls, 1);
}

TEST(ObservableTest, SubscriptionSafeAfterObservableDestroyed) {
    Subscription sub;
    {
        Observable<int> o{0};
        sub = o.Subscribe([](const int&) {});
        EXPECT_TRUE(sub.Active());
    } // Observable (and its registry) destroyed first
    // Destroying/resetting the subscription now must not dangle.
    EXPECT_NO_THROW({ sub.Reset(); });
}

TEST(ObservableTest, SubscribeAndFireFiresImmediately) {
    Observable<int> o{11};
    int seen = -1;
    auto sub = o.SubscribeAndFire([&](const int& v) { seen = v; });
    EXPECT_EQ(seen, 11); // fired with current value on subscribe
}

TEST(ObservableTest, BindUpdatesSinkOnChange) {
    Observable<int> source{1};
    int mirror = 0;
    auto sub = Bind(source, [&](const int& v) { mirror = v; });
    EXPECT_EQ(mirror, 1); // immediate
    source.Set(99);
    EXPECT_EQ(mirror, 99);
}

TEST(ObservableTest, MovePreservesExistingSubscriptions) {
    Observable<int> a{0};
    int calls = 0;
    auto sub = a.Subscribe([&](const int&) { ++calls; });
    Observable<int> b = std::move(a);
    b.Set(5); // observer registered before the move still fires
    EXPECT_EQ(calls, 1);
}
