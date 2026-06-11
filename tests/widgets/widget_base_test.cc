#include <unigui/unigui.h>
#include <unigui/widgets/widget_base.h>

#include <gtest/gtest.h>

TEST(Widget, GetID_ReturnsConsistentValue) {
    unigui::Widget* w1 = nullptr; // Widget is abstract, test via subclass
    // Use a concrete test widget
    class TestWidget : public unigui::Widget {
    public:
        explicit TestWidget(std::string name)
                : Widget(std::move(name)) {}
        void Render() override {}
    };

    TestWidget a("panel_a");
    TestWidget b("panel_b");
    TestWidget a2("panel_a");

    EXPECT_EQ(a.GetID(), a2.GetID());
    EXPECT_NE(a.GetID(), b.GetID());
}

TEST(Widget, IsVisible_DefaultsToTrue) {
    class TestWidget : public unigui::Widget {
    public:
        explicit TestWidget(std::string name)
                : Widget(std::move(name)) {}
        void Render() override {}
    };

    TestWidget w("test");
    EXPECT_TRUE(w.IsVisible());
}

TEST(Widget, Show_Hide_TogglesVisibility) {
    class TestWidget : public unigui::Widget {
    public:
        explicit TestWidget(std::string name)
                : Widget(std::move(name)) {}
        void Render() override {}
    };

    TestWidget w("test");
    w.Hide();
    EXPECT_FALSE(w.IsVisible());
    w.Show();
    EXPECT_TRUE(w.IsVisible());
}

TEST(Widget, GetName_ReturnsGivenName) {
    class TestWidget : public unigui::Widget {
    public:
        explicit TestWidget(std::string name)
                : Widget(std::move(name)) {}
        void Render() override {}
    };

    TestWidget w("MyPanel");
    EXPECT_EQ(w.GetName(), "MyPanel");
}

TEST(Widget, SetMinSize_StoresValues) {
    class TestWidget : public unigui::Widget {
    public:
        explicit TestWidget(std::string name)
                : Widget(std::move(name)) {}
        void Render() override {}
    };
    TestWidget w("test");
    w.SetMinSize(100, 200);
    EXPECT_FLOAT_EQ(w.GetMinSize().x, 100);
    EXPECT_FLOAT_EQ(w.GetMinSize().y, 200);
}

// ── Step 4: elevation wires into a widget's ShadowConfig ─────────────────────

namespace {
class ElevWidget : public unigui::Widget {
public:
    explicit ElevWidget(std::string name)
            : Widget(std::move(name)) {}
    void Render() override {}
};
} // namespace

TEST(Widget, SetElevation_EnablesAndFillsShadow) {
    ElevWidget w("panel");
    w.SetElevation(unigui::fx::Elevation::Medium);
    const auto& sc = w.GetShadowConfig();
    EXPECT_TRUE(sc.enabled);
    EXPECT_GT(sc.radius, 0.f);
}

TEST(Widget, SetElevation_NoneDisablesShadow) {
    ElevWidget w("panel");
    w.SetElevation(unigui::fx::Elevation::High);
    EXPECT_TRUE(w.GetShadowConfig().enabled);
    w.SetElevation(unigui::fx::Elevation::None);
    EXPECT_FALSE(w.GetShadowConfig().enabled);
}

TEST(Widget, WithElevation_HigherMeansLargerRadius) {
    ElevWidget low("a");
    low.WithElevation(unigui::fx::Elevation::Low);
    float lowR = low.GetShadowConfig().radius;
    ElevWidget high("b");
    high.WithElevation(unigui::fx::Elevation::High);
    EXPECT_GT(high.GetShadowConfig().radius, lowR);
}
