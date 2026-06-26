// Application layer (unigui::dsl): Store (shared state), Component::Watch /
// OnCleanup (the shared-state→render bridge and effect-cleanup lifecycle), and
// Navigator (a screen stack). The Component/Navigator pieces run in a headless
// ImGui frame; the pure Store tests do not need one.

#include <unigui/dsl/app.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

using unigui::Computed;
using unigui::dsl::Component;
using unigui::dsl::Navigator;
using unigui::dsl::NodePtr;
using unigui::dsl::Store;
using unigui::dsl::Text;

// ── Store (pure) ─────────────────────────────────────────────────────────────
TEST(StoreTest, GetSetUpdate) {
    Store<int> s{1};
    EXPECT_EQ(s.Get(), 1);
    s.Set(5);
    EXPECT_EQ(s(), 5);
    s.Update([](int& v) { v += 10; });
    EXPECT_EQ(s(), 15);
}

TEST(StoreTest, FeedsComputed) {
    Store<int> s{3};
    Computed<int> doubled{[](int n) { return n * 2; }, s.AsObservable()};
    EXPECT_EQ(doubled.Get(), 6);
    s.Set(10);
    EXPECT_EQ(doubled.Get(), 20);
}

// ── Component Watch / OnCleanup + Navigator (headless frame) ─────────────────
namespace {
class Screen : public Component {
public:
    int builds = 0;
    NodePtr Build() override {
        ++builds;
        return Text("screen");
    }
};

class WatchComp : public Component {
public:
    Store<int>* store;
    int builds = 0;
    explicit WatchComp(Store<int>* s)
            : store(s) {}
    void OnMount() override { Watch(*store); }
    NodePtr Build() override {
        ++builds;
        return Text("v=" + std::to_string((*store)()));
    }
};

class CleanupComp : public Component {
public:
    std::vector<int>* log;
    explicit CleanupComp(std::vector<int>* l)
            : log(l) {}
    void OnMount() override {
        OnCleanup([this] { log->push_back(1); });
        OnCleanup([this] { log->push_back(2); });
    }
    NodePtr Build() override { return Text("x"); }
};
} // namespace

class AppTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
        ImGui::Begin("t");
    }
    void TearDown() override {
        ImGui::End();
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(AppTest, ComponentWatchesStoreAndRerenders) {
    Store<int> store{0};
    WatchComp c{&store};
    c.Render(); // mount → Watch(store), first build
    EXPECT_EQ(c.builds, 1);
    store.Set(5); // shared-state change marks the watcher dirty
    EXPECT_TRUE(c.IsDirty());
    c.Render(); // rebuild reflects the new store value
    EXPECT_EQ(c.builds, 2);
}

TEST_F(AppTest, StoreSameValueDoesNotRerender) {
    Store<int> store{4};
    WatchComp c{&store};
    c.Render();
    store.Set(4); // unchanged → no notification → not dirty
    EXPECT_FALSE(c.IsDirty());
    c.Render();
    EXPECT_EQ(c.builds, 1);
}

TEST_F(AppTest, CleanupsRunInReverseOnUnmount) {
    std::vector<int> log;
    CleanupComp c{&log};
    c.Render();  // mount → register two cleanups
    c.Unmount(); // run them in reverse registration order
    ASSERT_EQ(log.size(), 2u);
    EXPECT_EQ(log[0], 2);
    EXPECT_EQ(log[1], 1);
}

TEST_F(AppTest, NavigatorPushPopRendersAndUnmountsTop) {
    auto a = std::make_shared<Screen>();
    auto b = std::make_shared<Screen>();
    Navigator nav;
    EXPECT_TRUE(nav.Empty());
    nav.Push(a);
    nav.Push(b);
    EXPECT_EQ(nav.Depth(), 2);
    EXPECT_EQ(nav.Top(), b.get());

    nav.Render(); // renders + mounts the top screen (b)
    EXPECT_TRUE(b->IsMounted());
    EXPECT_EQ(b->builds, 1);

    nav.Pop(); // unmounts b, reveals a
    EXPECT_FALSE(b->IsMounted());
    EXPECT_EQ(nav.Depth(), 1);
    EXPECT_EQ(nav.Top(), a.get());
}

TEST_F(AppTest, NavigatorReplaceUnmountsOutgoing) {
    auto a = std::make_shared<Screen>();
    auto b = std::make_shared<Screen>();
    Navigator nav;
    nav.Push(a);
    nav.Render();
    EXPECT_TRUE(a->IsMounted());
    nav.Replace(b);
    EXPECT_FALSE(a->IsMounted()); // outgoing screen unmounted
    EXPECT_EQ(nav.Depth(), 1);
    EXPECT_EQ(nav.Top(), b.get());
}
