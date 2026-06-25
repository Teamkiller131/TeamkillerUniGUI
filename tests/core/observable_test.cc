#include <unigui/core/observable.h>

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using unigui::Bind;
using unigui::Computed;
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

// ── Computed<T> ──────────────────────────────────────────────────────────────

TEST(ComputedTest, DerivesInitialValue) {
    Observable<int> a{2}, b{3};
    Computed<int> sum{[](int x, int y) { return x + y; }, a, b};
    EXPECT_EQ(sum.Get(), 5);
    EXPECT_EQ(static_cast<int>(sum), 5); // implicit conversion
}

TEST(ComputedTest, RecomputesWhenAnySourceChanges) {
    Observable<int> a{2}, b{3};
    Computed<int> sum{[](int x, int y) { return x + y; }, a, b};
    a.Set(10);
    EXPECT_EQ(sum.Get(), 13);
    b.Set(7);
    EXPECT_EQ(sum.Get(), 17);
}

TEST(ComputedTest, NotifiesSubscribersOnRecompute) {
    Observable<int> a{1};
    Computed<int> doubled{[](int n) { return n * 2; }, a};
    int seen = -1, calls = 0;
    auto sub = doubled.Subscribe([&](const int& v) {
        seen = v;
        ++calls;
    });
    a.Set(5);
    EXPECT_EQ(seen, 10);
    EXPECT_EQ(calls, 1);
}

TEST(ComputedTest, OnlyNotifiesWhenDerivedValueActuallyChanges) {
    Observable<int> a{2};
    Computed<int> parity{[](int n) { return n % 2; }, a}; // 0 for even
    int calls = 0;
    auto sub = parity.Subscribe([&](const int&) { ++calls; });
    a.Set(4); // still even → derived value unchanged (0) → no notification
    EXPECT_EQ(calls, 0);
    a.Set(5); // now odd → derived flips to 1 → notifies
    EXPECT_EQ(parity.Get(), 1);
    EXPECT_EQ(calls, 1);
}

TEST(ComputedTest, ChainsThroughAnotherComputed) {
    Observable<int> a{1};
    Computed<int> doubled{[](int n) { return n * 2; }, a};
    Computed<int> plusOne{[](int n) { return n + 1; }, doubled};
    EXPECT_EQ(plusOne.Get(), 3); // (1*2)+1
    a.Set(5);
    EXPECT_EQ(doubled.Get(), 10);
    EXPECT_EQ(plusOne.Get(), 11); // recompute propagates through the chain
}

TEST(ComputedTest, BindFiresImmediatelyAndOnRecompute) {
    Observable<int> a{1}, b{1};
    Computed<int> product{[](int x, int y) { return x * y; }, a, b};
    int mirror = -1;
    auto sub = Bind(product, [&](const int& v) { mirror = v; });
    EXPECT_EQ(mirror, 1); // immediate
    a.Set(6);
    b.Set(7);
    EXPECT_EQ(mirror, 42);
}

TEST(ComputedTest, SafeWhenDestroyedBeforeSource) {
    Observable<int> src{1};
    Subscription downstream;
    int seen = -1;
    {
        Computed<int> c{[](int n) { return n + 1; }, src};
        downstream = c.Subscribe([&](const int& v) { seen = v; });
        src.Set(2);
        EXPECT_EQ(seen, 3);
    } // c destroyed: its subscription to src is removed (RAII)
    EXPECT_NO_THROW({ src.Set(10); }); // must not call into the destroyed Computed
    EXPECT_EQ(seen, 3);                // downstream observer no longer fires
    EXPECT_NO_THROW({ downstream.Reset(); });
}

TEST(ComputedTest, SafeWhenASourceIsDestroyedBeforeTheComputed) {
    // The Computed caches source values, so a source dying while the Computed
    // lives on must not dangle: its slot keeps the last value and a surviving
    // sibling source can still drive recomputes.
    Observable<int> b{3};
    auto a = std::make_unique<Observable<int>>(2);
    Computed<int> sum{[](int x, int y) { return x + y; }, *a, b};
    EXPECT_EQ(sum.Get(), 5);
    a->Set(10);
    EXPECT_EQ(sum.Get(), 13);
    a.reset();                      // destroy one source; the Computed lives on
    EXPECT_NO_THROW({ b.Set(7); }); // recompute reads the cache, not the dead source
    EXPECT_EQ(sum.Get(), 17);       // 10 (last cached a) + 7
}

TEST(ComputedTest, ZeroSourcesIsAConstant) {
    Computed<int> c{[] { return 42; }};
    EXPECT_EQ(c.Get(), 42);
    EXPECT_EQ(c.ObserverCount(), 0u);
}

TEST(ComputedTest, HeterogeneousSourceTypes) {
    Observable<int> n{3};
    Observable<std::string> s{"x"};
    Computed<std::string> label{
        [](int i, const std::string& str) { return str + std::to_string(i); }, n, s};
    EXPECT_EQ(label.Get(), "x3");
    n.Set(9);
    EXPECT_EQ(label.Get(), "x9");
    s.Set("y");
    EXPECT_EQ(label.Get(), "y9");
}

TEST(ComputedTest, DiamondSettlesToCorrectValue) {
    // Diamond: a → b=2a, and c = a + b (= 3a). Propagation is eventually
    // consistent (a transient intermediate may be observed), but c must settle on
    // the correct value and the last notification delivered must be that value.
    Observable<int> a{1};
    Computed<int> b{[](int n) { return n * 2; }, a};
    Computed<int> c{[](int x, int y) { return x + y; }, a, b};
    std::vector<int> delivered;
    auto sub = c.Subscribe([&](const int& v) { delivered.push_back(v); });
    a.Set(5);
    EXPECT_EQ(c.Get(), 15); // settled: 5 + 10
    ASSERT_FALSE(delivered.empty());
    EXPECT_EQ(delivered.back(), 15); // last value delivered is the settled one
}
