#include <unigui/core/accessibility.h>

#include <gtest/gtest.h>
#include <string>
#include <vector>

using namespace unigui;

class A11yTest : public ::testing::Test {
protected:
    void SetUp() override {
        a11y::SetOnFocusChanged(nullptr);
        a11y::ClearFocus();
        a11y::SetEnabled(true);
    }
    void TearDown() override {
        a11y::SetOnFocusChanged(nullptr);
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
