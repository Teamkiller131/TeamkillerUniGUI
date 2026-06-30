#include <unigui/core/accessibility.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace unigui;

class A11yTest : public ::testing::Test {
protected:
    void SetUp() override {
        a11y::SetOnFocusChanged(nullptr);
        a11y::SetOnAnnounce(nullptr);
        a11y::SetEnabled(true);
        a11y::ClearFocus();
        a11y::BeginFrame();         // reset the per-frame tree
        a11y::DrainAnnouncements(); // reset the announcement queue
    }
    void TearDown() override {
        a11y::SetOnFocusChanged(nullptr);
        a11y::SetOnAnnounce(nullptr);
        a11y::ClearFocus();
        a11y::SetEnabled(false);
    }
};

TEST_F(A11yTest, RoleName) {
    EXPECT_STREQ(a11y::RoleName(a11y::Role::Button), "button");
    EXPECT_STREQ(a11y::RoleName(a11y::Role::CheckBox), "checkbox");
    EXPECT_STREQ(a11y::RoleName(a11y::Role::Unknown), "unknown");
}

TEST_F(A11yTest, SetFocused_TracksAndFires) {
    int fires = 0;
    a11y::Node last;
    a11y::SetOnFocusChanged([&](const a11y::Node& n) {
        ++fires;
        last = n;
    });
    a11y::SetFocused({"Save", "Ctrl+S", "", a11y::Role::Button});
    EXPECT_TRUE(a11y::HasFocus());
    EXPECT_EQ(fires, 1);
    EXPECT_EQ(last.name, "Save");
    EXPECT_EQ(last.role, a11y::Role::Button);
}

TEST_F(A11yTest, SetFocused_NoRefireOnSameNode) {
    int fires = 0;
    a11y::SetOnFocusChanged([&](const a11y::Node&) { ++fires; });
    const a11y::Node n{"Qty", "", "5", a11y::Role::Input};
    a11y::SetFocused(n);
    a11y::SetFocused(n); // identical → must NOT re-announce
    a11y::SetFocused(n);
    EXPECT_EQ(fires, 1);
    a11y::SetFocused({"Qty", "", "6", a11y::Role::Input}); // value changed → fires
    EXPECT_EQ(fires, 2);
}

TEST_F(A11yTest, ClearFocus_Fires) {
    int fires = 0;
    a11y::SetOnFocusChanged([&](const a11y::Node&) { ++fires; });
    a11y::SetFocused({"X", "", "", a11y::Role::Text});
    EXPECT_EQ(fires, 1);
    a11y::ClearFocus();
    EXPECT_FALSE(a11y::HasFocus());
    EXPECT_EQ(fires, 2);
}

TEST_F(A11yTest, Disabled_IsNoOp) {
    a11y::SetEnabled(false);
    int fires = 0;
    a11y::SetOnFocusChanged([&](const a11y::Node&) { ++fires; });
    a11y::SetFocused({"Y", "", "", a11y::Role::Button});
    EXPECT_FALSE(a11y::HasFocus());
    EXPECT_EQ(fires, 0);
}

TEST_F(A11yTest, RoleName_NewRoles) {
    EXPECT_STREQ(a11y::RoleName(a11y::Role::Toggle), "switch");
    EXPECT_STREQ(a11y::RoleName(a11y::Role::Progress), "progressbar");
    EXPECT_STREQ(a11y::RoleName(a11y::Role::Status), "status");
    EXPECT_STREQ(a11y::RoleName(a11y::Role::Window), "window");
}

TEST_F(A11yTest, Tree_CollectsNodesPerFrameAndResets) {
    a11y::BeginFrame();
    a11y::AddNode({"Save", "", "", a11y::Role::Button});
    a11y::AddNode({"Qty", "", "5", a11y::Role::Input});
    ASSERT_EQ(a11y::Tree().size(), 2u);
    EXPECT_EQ(a11y::Tree()[0].name, "Save");
    EXPECT_EQ(a11y::Tree()[1].value, "5");
    a11y::BeginFrame(); // next frame resets the tree
    EXPECT_TRUE(a11y::Tree().empty());
}

TEST_F(A11yTest, AddNode_FocusedDrivesFocusCallback) {
    int fires = 0;
    a11y::Node last;
    a11y::SetOnFocusChanged([&](const a11y::Node& n) {
        ++fires;
        last = n;
    });
    a11y::BeginFrame();
    a11y::AddNode({"A", "", "", a11y::Role::Button, /*focused=*/false});
    EXPECT_EQ(fires, 0); // a non-focused node registers in the tree but doesn't announce
    a11y::AddNode({"B", "", "", a11y::Role::Button, /*focused=*/true});
    EXPECT_EQ(fires, 1);
    EXPECT_EQ(last.name, "B");
    EXPECT_TRUE(a11y::HasFocus());
}

TEST_F(A11yTest, Tree_NoOpWhenDisabled) {
    a11y::SetEnabled(false);
    a11y::BeginFrame();
    a11y::AddNode({"X", "", "", a11y::Role::Button});
    EXPECT_TRUE(a11y::Tree().empty());
}

TEST_F(A11yTest, Announce_QueuesFiresAndDrains) {
    std::vector<a11y::Announcement> got;
    a11y::SetOnAnnounce([&](const a11y::Announcement& a) { got.push_back(a); });
    a11y::Announce("Saved");
    a11y::Announce("Error!", a11y::Live::Assertive);
    ASSERT_EQ(got.size(), 2u);
    EXPECT_EQ(got[0].message, "Saved");
    EXPECT_EQ(got[0].politeness, a11y::Live::Polite);
    EXPECT_EQ(got[1].politeness, a11y::Live::Assertive);
    auto drained = a11y::DrainAnnouncements();
    EXPECT_EQ(drained.size(), 2u);
    EXPECT_TRUE(a11y::DrainAnnouncements().empty()); // queue cleared on drain
}

TEST_F(A11yTest, Announce_NoOpWhenDisabledOrEmpty) {
    a11y::Announce(""); // empty message ignored
    EXPECT_TRUE(a11y::DrainAnnouncements().empty());
    a11y::SetEnabled(false);
    a11y::Announce("ignored while disabled");
    EXPECT_TRUE(a11y::DrainAnnouncements().empty());
}

TEST_F(A11yTest, BeginFrame_AutoClearsFocusAfterNoneReported) {
    int fires = 0;
    a11y::SetOnFocusChanged([&](const a11y::Node&) { ++fires; });
    a11y::AddNode({"A", "", "", a11y::Role::Button, /*focused=*/true}); // report focus this frame
    EXPECT_TRUE(a11y::HasFocus());
    EXPECT_EQ(fires, 1);
    a11y::BeginFrame(); // the frame that just ended had focus → keep it
    EXPECT_TRUE(a11y::HasFocus());
    a11y::BeginFrame(); // the frame that just ended reported no focus → auto-clear
    EXPECT_FALSE(a11y::HasFocus());
    EXPECT_EQ(fires, 2);
}

TEST_F(A11yTest, Announce_QueueIsBounded) {
    // Subscribe-only (never drains): the queue must not grow without limit.
    for (int i = 0; i < 1000; ++i)
        a11y::Announce("msg");
    EXPECT_LE(a11y::DrainAnnouncements().size(), 256u);
}

TEST_F(A11yTest, InstallSystemBridge_NullHandleEnablesAndFallsBack) {
    // A null window handle forces the logging fallback on every platform; it must still
    // enable a11y. (This also links the platform bridge TU — the Windows build pulls in
    // UI Automation here.)
    a11y::SetEnabled(false);
    a11y::InstallSystemBridge(nullptr);
    EXPECT_TRUE(a11y::IsEnabled());
}
