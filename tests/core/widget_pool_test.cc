#include <unigui/core/widget_pool.h>

#include <gtest/gtest.h>

#include <memory>

using namespace unigui;

namespace {
// A trivial stand-in for a retained widget; tracks live instances so we can
// assert eviction actually destroys them.
struct FakeWidget {
    static inline int live = 0;
    int value;
    explicit FakeWidget(int v) : value(v) { ++live; }
    ~FakeWidget() { --live; }
};
} // namespace

class WidgetPoolTest : public ::testing::Test {
protected:
    void SetUp() override { FakeWidget::live = 0; }
};

TEST_F(WidgetPoolTest, GetOrCreate_ConstructsOnce) {
    WidgetPool<FakeWidget> pool;
    int builds = 0;
    auto make = [&](int v) {
        return [&, v] {
            ++builds;
            return std::make_unique<FakeWidget>(v);
        };
    };
    pool.BeginFrame();
    FakeWidget& a = pool.GetOrCreate(7, make(100));
    FakeWidget& b = pool.GetOrCreate(7, make(999)); // same key → no rebuild
    pool.EndFrame();
    EXPECT_EQ(builds, 1);
    EXPECT_EQ(&a, &b);
    EXPECT_EQ(a.value, 100);
    EXPECT_EQ(pool.Size(), 1u);
}

TEST_F(WidgetPoolTest, StableAcrossFrames) {
    WidgetPool<FakeWidget> pool;
    FakeWidget* first = nullptr;
    for (int frame = 0; frame < 3; ++frame) {
        pool.BeginFrame();
        FakeWidget& w = pool.GetOrCreate(42, [] { return std::make_unique<FakeWidget>(1); });
        if (!first)
            first = &w;
        EXPECT_EQ(&w, first); // same instance every frame
        pool.EndFrame();
    }
    EXPECT_EQ(FakeWidget::live, 1);
}

TEST_F(WidgetPoolTest, EvictsRowsNotTouchedThisFrame) {
    WidgetPool<FakeWidget> pool;
    auto f = [](int v) { return [v] { return std::make_unique<FakeWidget>(v); }; };

    // Frame 1: three rows.
    pool.BeginFrame();
    pool.GetOrCreate(1, f(1));
    pool.GetOrCreate(2, f(2));
    pool.GetOrCreate(3, f(3));
    pool.EndFrame();
    EXPECT_EQ(pool.Size(), 3u);
    EXPECT_EQ(FakeWidget::live, 3);

    // Frame 2: row 2 removed — its widget must be destroyed, not leaked.
    pool.BeginFrame();
    pool.GetOrCreate(1, f(1));
    pool.GetOrCreate(3, f(3));
    pool.EndFrame();
    EXPECT_EQ(pool.Size(), 2u);
    EXPECT_EQ(FakeWidget::live, 2);
    EXPECT_FALSE(pool.Contains(2));
    EXPECT_TRUE(pool.Contains(1));
}

TEST_F(WidgetPoolTest, Clear_DestroysAll) {
    WidgetPool<FakeWidget> pool;
    pool.BeginFrame();
    pool.GetOrCreate(1, [] { return std::make_unique<FakeWidget>(1); });
    pool.GetOrCreate(2, [] { return std::make_unique<FakeWidget>(2); });
    pool.EndFrame();
    EXPECT_EQ(FakeWidget::live, 2);
    pool.Clear();
    EXPECT_EQ(pool.Size(), 0u);
    EXPECT_EQ(FakeWidget::live, 0);
}
