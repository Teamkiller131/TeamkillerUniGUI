// Data-binding wiring tests: ValueWidget<T>::BindValue (two-way) and
// Label::BindText (one-way). These exercise the binding glue without an ImGui
// frame by driving value changes through a tiny test subclass (which exposes the
// protected NotifyChange hook a real widget calls from Render on a user edit) and
// through the widgets' public setters.

#include <unigui/core/observable.h>
#include <unigui/widgets/checkbox.h>
#include <unigui/widgets/label.h>
#include <unigui/widgets/lineedit.h>
#include <unigui/widgets/passwordinput.h>
#include <unigui/widgets/value_widget.h>

#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>

using unigui::CheckBox;
using unigui::Label;
using unigui::LineEdit;
using unigui::Observable;
using unigui::PasswordInput;
using unigui::ValueWidget;

namespace {
// Exposes a user-edit path so we can simulate the user changing the value in the
// UI (what a widget's Render() does on interaction) without an ImGui frame.
class TestValueWidget : public ValueWidget<int> {
public:
    explicit TestValueWidget(std::string name)
            : ValueWidget<int>(std::move(name), 0) {}
    void Render() override {}
    void UserEdit(int v) {
        int old = value_;
        value_ = v;
        NotifyChange(old);
    }
};
} // namespace

TEST(DataBindingTest, ModelToViewUpdatesWidget) {
    Observable<int> model{3};
    TestValueWidget w{"w"};
    w.BindValue(model);
    EXPECT_EQ(w.GetValue(), 3); // adopts the source value on bind
    model.Set(42);
    EXPECT_EQ(w.GetValue(), 42); // observable change flows into the widget
}

TEST(DataBindingTest, ViewToModelPushesBackTwoWay) {
    Observable<int> model{0};
    TestValueWidget w{"w"};
    w.BindValue(model); // two-way by default
    w.UserEdit(7);
    EXPECT_EQ(model.Get(), 7); // user edit writes back to the observable
}

TEST(DataBindingTest, OneWayDoesNotPushBack) {
    Observable<int> model{0};
    TestValueWidget w{"w"};
    w.BindValue(model, /*twoWay=*/false);
    w.UserEdit(7);
    EXPECT_EQ(w.GetValue(), 7); // local change applied
    EXPECT_EQ(model.Get(), 0);  // but not propagated to the observable
}

TEST(DataBindingTest, NoFeedbackLoop) {
    Observable<int> model{0};
    TestValueWidget w{"w"};
    int modelNotifications = 0;
    auto sub = model.Subscribe([&](const int&) { ++modelNotifications; });
    w.BindValue(model);
    w.UserEdit(5);
    // Exactly one observable notification (the push-back). The binding's own
    // observer re-applying that value must not trigger a second push.
    EXPECT_EQ(modelNotifications, 1);
    EXPECT_EQ(model.Get(), 5);
    EXPECT_EQ(w.GetValue(), 5);
}

TEST(DataBindingTest, UnbindStopsUpdates) {
    Observable<int> model{1};
    TestValueWidget w{"w"};
    w.BindValue(model);
    w.Unbind();
    model.Set(99);
    EXPECT_EQ(w.GetValue(), 1); // no longer tracking the model
    w.UserEdit(2);
    EXPECT_EQ(model.Get(), 99); // no longer pushing back
}

TEST(DataBindingTest, CheckBoxBindsBool) {
    Observable<bool> enabled{false};
    CheckBox cb{"cb", "Enable", false};
    cb.BindValue(enabled);
    enabled.Set(true);
    EXPECT_TRUE(cb.GetValue());
}

TEST(DataBindingTest, LabelBindTextOneWay) {
    Observable<std::string> status{"idle"};
    Label lbl{"lbl"};
    lbl.BindText(status);
    EXPECT_EQ(lbl.GetText(), "idle"); // adopts on bind
    status.Set("running");
    EXPECT_EQ(lbl.GetText(), "running");
}

TEST(DataBindingTest, BindingAutoDetachesOnWidgetDestruction) {
    Observable<int> model{0};
    {
        TestValueWidget w{"w"};
        w.BindValue(model);
        EXPECT_EQ(model.ObserverCount(), 1u);
    } // widget destroyed → its binding subscription is dropped
    EXPECT_EQ(model.ObserverCount(), 0u);
    EXPECT_NO_THROW({ model.Set(5); }); // no dangling callback into the dead widget
}

// LineEdit/PasswordInput override ApplyBoundValue to route a model push through
// their buffer-syncing SetValue; verify the model→view path keeps GetValue right.
TEST(DataBindingTest, LineEditBindRoutesThroughSetValue) {
    Observable<std::string> model{"alpha"};
    LineEdit le{"le", "Name"};
    le.BindValue(model);
    EXPECT_EQ(le.GetValue(), "alpha"); // adopts on bind via SetValue (buffer synced)
    model.Set("bravo");
    EXPECT_EQ(le.GetValue(), "bravo"); // model push routed through SetValue
}

TEST(DataBindingTest, PasswordInputBindRoutesThroughSetValue) {
    Observable<std::string> model{"pw1"};
    PasswordInput pw{"pw", "Password"};
    pw.BindValue(model);
    EXPECT_EQ(pw.GetValue(), "pw1");
    model.Set("pw2");
    EXPECT_EQ(pw.GetValue(), "pw2");
}

TEST(DataBindingTest, RebindSwitchesSource) {
    Observable<int> m1{1}, m2{2};
    TestValueWidget w{"w"};
    w.BindValue(m1);
    EXPECT_EQ(w.GetValue(), 1);
    w.BindValue(m2);                   // rebind to a different source
    EXPECT_EQ(w.GetValue(), 2);        // adopts the new source's value
    EXPECT_EQ(m1.ObserverCount(), 0u); // old subscription dropped
    EXPECT_EQ(m2.ObserverCount(), 1u);
    m1.Set(99);
    EXPECT_EQ(w.GetValue(), 2); // no longer tracking m1
    m2.Set(42);
    EXPECT_EQ(w.GetValue(), 42); // tracks m2
    w.UserEdit(7);
    EXPECT_EQ(m2.Get(), 7);  // two-way push goes to the new source
    EXPECT_EQ(m1.Get(), 99); // not the old one
}

TEST(DataBindingTest, TwoWaySafeWhenSourceDestroyedFirst) {
    // A user edit after the bound source is gone must not dereference a stale
    // pointer; the lifetime guard skips the push-back.
    TestValueWidget w{"w"};
    auto model = std::make_unique<Observable<int>>(0);
    w.BindValue(*model);
    model.reset(); // source destroyed before the widget
    EXPECT_NO_THROW({ w.UserEdit(5); });
    EXPECT_EQ(w.GetValue(), 5); // local edit still applies
}

// The move-only Subscription member makes bound widgets non-copyable; movability
// is intentionally retained (so unbound widgets can live in containers), with the
// "do not relocate after binding" contract documented in the header.
static_assert(!std::is_copy_constructible_v<TestValueWidget>);
static_assert(!std::is_copy_constructible_v<CheckBox>);
static_assert(!std::is_copy_constructible_v<Label>);
