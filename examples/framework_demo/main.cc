// ─────────────────────────────────────────────────────────────────────────────
// framework_demo — a multi-screen app built entirely in the UniGUI *framework*
// idiom: dsl::Component + dsl::State + dsl::Store + dsl::Navigator.
//
// The framework owns the loop and re-renders a screen only when its state changes.
// There is no hand-written per-frame immediate-mode UI here — each screen declares
// its view in Build() from reactive state; navigation is a screen stack; shared
// state is a Store that screens Watch(); effects clean up on unmount. A live
// Component inspector overlay shows what is mounted / dirty / rebuilding.
//
//   ./framework_demo --frames 10     # headless smoke (CI)
// ─────────────────────────────────────────────────────────────────────────────

#include <unigui/unigui.h>

#include <cstdio>
#include <cstdlib>
#include <memory>
#include <string>

// Only the framework (DSL) names are brought in flat — `Button`/`VBox`/etc. here
// are the *declarative* builders. The handful of app-loop names from `unigui`
// (Init/Run/AppConfig/WindowScope/im) are qualified to avoid colliding with the
// retained-widget `unigui::Button` / `unigui::Layout::VBox`.
using namespace unigui::dsl;

// ── Shared, app-wide state (a Store lives OUTSIDE the component tree) ─────────
namespace {
Store<int> g_count{0};
Store<std::string> g_lastAction{"(none)"};
} // namespace

// ── Detail screen: local State + watches shared state + an effect cleanup ────
class DetailScreen : public Component {
public:
    explicit DetailScreen(Navigator* nav)
            : nav_(nav) {}
    const char* InspectorName() const override { return "DetailScreen"; }

    void OnMount() override {
        Watch(g_count); // re-render when the shared count changes
        g_lastAction.Set("opened detail");
        OnCleanup([] { g_lastAction.Set("left detail"); }); // effect teardown on Back
    }

    NodePtr Build() override {
        return VBox({
            Text("Detail screen"),
            Separator(),
            Text("Shared count (live): " + std::to_string(g_count())),
            Text("Local ticks: " + std::to_string(ticks_())),
            Flex({Button("Tick (local state)", [this] { ticks_ = ticks_() + 1; }),
                  Button("Bump shared +10", [] { g_count.Update([](int& v) { v += 10; }); })},
                 8.0f),
            Button("< Back", [this] { nav_->Pop(); }),
        });
    }

private:
    Navigator* nav_;
    State<int> ticks_{this, 0};
};

// ── Home screen: watches the shared count, mutates it, navigates ─────────────
class HomeScreen : public Component {
public:
    explicit HomeScreen(Navigator* nav)
            : nav_(nav) {}
    const char* InspectorName() const override { return "HomeScreen"; }

    void OnMount() override { Watch(g_count); }

    NodePtr Build() override {
        return VBox({
            Text("UniGUI Framework Demo - Home"),
            Separator(),
            Text("Shared count: " + std::to_string(g_count())),
            Text("Last action: " + g_lastAction()),
            Flex({Button("Increment",
                         [] {
                             g_count.Update([](int& v) { ++v; });
                             g_lastAction.Set("increment");
                         }),
                  Button("Reset",
                         [] {
                             g_count.Set(0);
                             g_lastAction.Set("reset");
                         })},
                 {2.0f, 1.0f}, 8.0f),
            Button("Open detail >", [this] { nav_->Push(std::make_shared<DetailScreen>(nav_)); }),
        });
    }

private:
    Navigator* nav_;
};

int main(int argc, char** argv) {
    int maxFrames = 0;
    for (int i = 1; i < argc; ++i)
        if (std::string(argv[i]) == "--frames" && i + 1 < argc)
            maxFrames = std::atoi(argv[++i]);

    unigui::AppConfig cfg;
    cfg.title = "UniGUI Framework Demo";
    cfg.width = 900;
    cfg.height = 640;
    if (!unigui::Init(cfg)) {
        std::fprintf(stderr, "Init failed\n");
        return 1;
    }

    static Navigator nav;
    static const bool started = [] {
        nav.Push(std::make_shared<HomeScreen>(&nav));
        return true;
    }();
    (void) started;

    unigui::Run(
        [&] {
            unigui::im::SetNextWindowSize(ImVec2(560, 420), ImGuiCond_FirstUseEver);
            {
                unigui::WindowScope w{"App"};
                if (w)
                    nav.Render(); // the framework renders the current screen
            }
            unigui::im::SetNextWindowSize(ImVec2(340, 320), ImGuiCond_FirstUseEver);
            {
                unigui::WindowScope insp{"Component Inspector"};
                if (insp)
                    DrawInspector();
            }
        },
        maxFrames);
    return 0;
}
