// Interaction tests — the application framework (dsl::Component/State/Store/Navigator).
// The audit flagged this heavily-documented subsystem as having ~3 trivial tests; these
// drive the reactive loop end to end through real input: click -> State write -> dirty
// mark -> rebuild, Store dispatch -> subscriber + watching component, Navigator
// push/pop with mount/unmount lifecycle, and Host() per-child dirty isolation.
// Compiled only when UNIGUI_TEST_ENGINE=ON.
#include <unigui/dsl/app.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "interaction_harness.h"

class FrameworkDslInteractionTest : public itest::InteractionFixture {};

// Framework spine: a Component with local State<int> whose Build() view has a
// DSL "+1" button. A driven click writes the State, which change-detects, marks
// the component dirty, and the next frame re-Build()s the view with the new
// count — the reactive loop end to end, through real input.
TEST_F(FrameworkDslInteractionTest, ComponentStateClickIncrements) {
    class CounterScreen : public unigui::dsl::Component {
    public:
        unigui::dsl::State<int> count{this, 0};
        std::string lastBuiltText; // what the most recent Build() put on screen
        unigui::dsl::NodePtr Build() override {
            lastBuiltText = "Count: " + std::to_string(count());
            return unigui::dsl::VBox({
                unigui::dsl::Text(lastBuiltText),
                unigui::dsl::Button("+1", [this] { count = count() + 1; }),
            });
        }
    };

    CounterScreen screen;
    const auto st = Run(
        "dsl_component_state_click", [&] { screen.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/+1");
            ctx->ItemClick("**/+1");
            ctx->Yield(2); // give the dirty component a frame to rebuild
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(screen.count(), 2);                // both clicks landed on the State
    EXPECT_GE(screen.BuildCount(), 3);           // initial build + one rebuild per click
    EXPECT_EQ(screen.lastBuiltText, "Count: 2"); // rebuilt view reflects the new state
}

// Shared app-state round trip: a driven click dispatches into a Store, the
// external subscriber sees the notification, and a component Watch()ing the
// Store is marked dirty and re-Build()s with the new value.
TEST_F(FrameworkDslInteractionTest, StoreDispatchSubscribeRoundTrip) {
    unigui::dsl::Store<int> store{0};
    std::vector<int> received; // values delivered to the external subscriber
    const unigui::Subscription sub =
        store.AsObservable().Subscribe([&](const int& v) { received.push_back(v); });

    class StoreScreen : public unigui::dsl::Component {
    public:
        unigui::dsl::Store<int>* store = nullptr;
        void OnMount() override { Watch(*store); } // app-wide state -> this view
        unigui::dsl::NodePtr Build() override {
            return unigui::dsl::VBox({
                unigui::dsl::Text("value = " + std::to_string(store->Get())),
                unigui::dsl::Button("Dispatch", [this] { store->Set(store->Get() + 1); }),
            });
        }
    };

    StoreScreen screen;
    screen.store = &store;
    const auto st = Run(
        "dsl_store_dispatch_click", [&] { screen.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Dispatch");
            ctx->Yield(2); // let the watching component rebuild
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(store.Get(), 1);      // dispatch reached the Store
    ASSERT_EQ(received.size(), 1u); // subscriber notified exactly once
    EXPECT_EQ(received[0], 1);
    EXPECT_GE(screen.BuildCount(), 2); // Watch() marked it dirty -> rebuilt
}

// Routing: two screens on a Navigator stack. A driven click on the home screen's
// button pushes the settings route (current route changes), and a click on the
// settings screen's Back button pops it — running the unmount lifecycle — so the
// current route returns to home.
TEST_F(FrameworkDslInteractionTest, NavigatorPushPopViaClicks) {
    class NavScreen : public unigui::dsl::Component {
    public:
        std::string title;
        std::string buttonLabel;
        std::function<void()> onNav;
        int unmounts = 0;
        unigui::dsl::NodePtr Build() override {
            return unigui::dsl::VBox({
                unigui::dsl::Text(title),
                unigui::dsl::Button(buttonLabel, [this] { onNav(); }),
            });
        }
        void OnUnmount() override { ++unmounts; }
    };

    unigui::dsl::Navigator nav;
    auto settings = std::make_shared<NavScreen>();
    settings->title = "Settings screen";
    settings->buttonLabel = "Back";
    settings->onNav = [&] { nav.Pop(); };
    auto home = std::make_shared<NavScreen>();
    home->title = "Home screen";
    home->buttonLabel = "Go to Settings";
    home->onNav = [&] { nav.Push(settings); };
    nav.Push(home);

    int depthAfterPush = 0;
    unigui::dsl::Component* topAfterPush = nullptr;
    const auto st = Run(
        "dsl_navigator_push_pop", [&] { nav.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Go to Settings"); // home -> settings
            ctx->Yield();                        // settings mounts + renders
            depthAfterPush = nav.Depth();
            topAfterPush = nav.Top();
            ctx->ItemClick("**/Back"); // settings -> home
            ctx->Yield(2);
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(depthAfterPush, 2); // push made settings the current route
    EXPECT_EQ(topAfterPush, settings.get());
    EXPECT_GE(settings->BuildCount(), 1); // it really rendered while current
    EXPECT_EQ(nav.Depth(), 1);            // Back popped it again
    EXPECT_EQ(nav.Top(), home.get());
    EXPECT_EQ(settings->unmounts, 1); // Pop ran the unmount lifecycle
    EXPECT_FALSE(settings->IsMounted());
}

// Composition: Host(child) inside a parent's Build() tree keeps per-child dirty
// tracking. A driven click on the hosted child's button rebuilds ONLY the child;
// the parent's cached tree is untouched (its build count stays at 1).
TEST_F(FrameworkDslInteractionTest, HostedChildRebuildsWithoutParent) {
    class ChildComp : public unigui::dsl::Component {
    public:
        unigui::dsl::State<int> n{this, 0};
        unigui::dsl::NodePtr Build() override {
            return unigui::dsl::VBox({
                unigui::dsl::Text("child n = " + std::to_string(n())),
                unigui::dsl::Button("Child +1", [this] { n = n() + 1; }),
            });
        }
    };
    class ParentComp : public unigui::dsl::Component {
    public:
        ChildComp child; // parent owns the child (must outlive the rendered tree)
        unigui::dsl::NodePtr Build() override {
            return unigui::dsl::VBox({
                unigui::dsl::Text("parent shell"),
                unigui::dsl::Host(child),
            });
        }
    };

    ParentComp parent;
    const auto st = Run(
        "dsl_host_child_isolation", [&] { parent.Render(); },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("**/Child +1");
            ctx->Yield(2); // let the dirty child rebuild
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(parent.child.n(), 1);          // click reached the child's State
    EXPECT_GE(parent.child.BuildCount(), 2); // child rebuilt after the click
    EXPECT_EQ(parent.BuildCount(), 1);       // parent untouched: dirt is per-component
}