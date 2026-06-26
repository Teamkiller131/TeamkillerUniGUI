// Reactive Component + State (unigui::dsl). Runs in a headless ImGui frame: a
// Component mounts once, rebuilds its DSL view only when its State changes, and
// composes via Host(child) with independent per-child dirty tracking.

#include <unigui/dsl/component.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>

using unigui::Computed;
using unigui::dsl::Component;
using unigui::dsl::Host;
using unigui::dsl::NodePtr;
using unigui::dsl::State;
using unigui::dsl::Text;
using unigui::dsl::VBox;

namespace {
// Views are Text-only (no widget IDs) so repeated Render() calls within one test
// frame can't trip ImGui ID conflicts; we assert on build/lifecycle counters.
class CounterComp : public Component {
public:
    State<int> count{this, 0};
    int builds = 0;
    int mounts = 0;
    int unmounts = 0;
    NodePtr Build() override {
        ++builds;
        return Text("Count: " + std::to_string(count()));
    }
    void OnMount() override { ++mounts; }
    void OnUnmount() override { ++unmounts; }
};
} // namespace

class ComponentTest : public ::testing::Test {
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

TEST_F(ComponentTest, FirstRenderMountsAndBuildsOnce) {
    CounterComp c;
    EXPECT_FALSE(c.IsMounted());
    c.Render();
    EXPECT_TRUE(c.IsMounted());
    EXPECT_EQ(c.mounts, 1);
    EXPECT_EQ(c.builds, 1);
}

TEST_F(ComponentTest, NoStateChangeDoesNotRebuild) {
    CounterComp c;
    c.Render();
    c.Render();
    c.Render();
    EXPECT_EQ(c.builds, 1); // cached; not rebuilt without a state change
    EXPECT_EQ(c.mounts, 1);
}

TEST_F(ComponentTest, StateChangeTriggersRebuild) {
    CounterComp c;
    c.Render();
    EXPECT_EQ(c.builds, 1);
    c.count = 5;
    EXPECT_TRUE(c.IsDirty());
    c.Render();
    EXPECT_EQ(c.builds, 2);
    EXPECT_EQ(c.count(), 5);
}

TEST_F(ComponentTest, SameValueWriteDoesNotRebuild) {
    CounterComp c;
    c.Render();
    c.count = 0; // unchanged
    EXPECT_FALSE(c.IsDirty());
    c.Render();
    EXPECT_EQ(c.builds, 1);
}

TEST_F(ComponentTest, MutateTriggersRebuild) {
    CounterComp c;
    c.Render();
    c.count.Mutate([](int& v) { v += 3; });
    c.Render();
    EXPECT_EQ(c.builds, 2);
    EXPECT_EQ(c.count(), 3);
}

TEST_F(ComponentTest, StateFeedsComputed) {
    CounterComp c;
    Computed<int> doubled{[](int n) { return n * 2; }, c.count.AsObservable()};
    EXPECT_EQ(doubled.Get(), 0);
    c.count = 7;
    EXPECT_EQ(doubled.Get(), 14);
}

TEST_F(ComponentTest, UnmountFiresOnUnmount) {
    CounterComp c;
    c.Render();
    c.Unmount();
    EXPECT_EQ(c.unmounts, 1);
    EXPECT_FALSE(c.IsMounted());
}

namespace {
// Composition: a parent hosting a child; the child keeps independent state + dirty.
class ParentComp : public Component {
public:
    CounterComp child;
    int builds = 0;
    NodePtr Build() override {
        ++builds;
        return VBox({Text("parent"), Host(child)});
    }
};
} // namespace

TEST_F(ComponentTest, HostRendersChildIndependently) {
    ParentComp p;
    p.Render();
    EXPECT_EQ(p.builds, 1);
    EXPECT_EQ(p.child.builds, 1); // child built via Host(child)
    EXPECT_TRUE(p.child.IsMounted());

    // A child state change rebuilds only the CHILD, not the cached parent tree.
    p.child.count = 9;
    p.Render();
    EXPECT_EQ(p.builds, 1);       // parent tree still cached
    EXPECT_EQ(p.child.builds, 2); // child rebuilt
    EXPECT_EQ(p.child.count(), 9);
}
