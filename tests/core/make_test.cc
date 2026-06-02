#include <unigui/core/make.h>
#include <unigui/widgets/button.h>
#include <unigui/widgets/label.h>

#include <gtest/gtest.h>

#include <memory>
#include <set>
#include <string>

// unigui::Make / MakeNamed factory helpers.

TEST(MakeTest, Make_ReturnsSharedPtrWithGivenName) {
    auto btn = unigui::Make<unigui::Button>("save", "Save");
    ASSERT_NE(btn, nullptr);
    EXPECT_EQ(btn->GetName(), "save");
    EXPECT_EQ(btn->GetLabel(), "Save");
}

TEST(MakeTest, MakeNamed_GeneratesUniqueNames) {
    std::set<std::string> names;
    for (int i = 0; i < 100; ++i) {
        auto b = unigui::MakeNamed<unigui::Button>("Btn");
        names.insert(b->GetName());
    }
    EXPECT_EQ(names.size(), 100u);  // all distinct
}

TEST(MakeTest, NextAutoName_IsMonotonicAndPrefixed) {
    const std::string a = unigui::NextAutoName("widget");
    const std::string b = unigui::NextAutoName("widget");
    EXPECT_NE(a, b);
    EXPECT_EQ(a.rfind("widget##auto", 0), 0u);
}

TEST(MakeTest, Make_WorksForDifferentWidgetTypes) {
    auto lbl = unigui::Make<unigui::Label>("hint", "Read-only");
    ASSERT_NE(lbl, nullptr);
    EXPECT_EQ(lbl->GetName(), "hint");
}
