#include <unigui/core/scope.h>

#include <imgui.h>
#include <gtest/gtest.h>

#include <type_traits>

// RAII scope guards. Verify they pair Begin*/End* correctly without leaving the
// ImGui stack unbalanced (an unbalanced stack would assert on ImGui::Render()).
class ScopeTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();  // asserts internally if any stack is unbalanced
        ImGui::DestroyContext();
    }
};

TEST_F(ScopeTest, WindowScope_BalancesBeginEnd) {
    {
        unigui::WindowScope w{"Settings"};
        if (w) ImGui::TextUnformatted("content");
    }
    SUCCEED();
}

TEST_F(ScopeTest, ChildScope_BalancesBeginEnd) {
    unigui::WindowScope w{"Host"};
    {
        unigui::ChildScope c{"child", ImVec2(100, 100)};
        if (c) ImGui::TextUnformatted("inside");
    }
    SUCCEED();
}

TEST_F(ScopeTest, IDScope_BalancesPushPop) {
    {
        unigui::IDScope id{"row"};
        ImGui::Button("same-label");
    }
    {
        unigui::IDScope id{42};
        ImGui::Button("same-label");
    }
    SUCCEED();
}

TEST_F(ScopeTest, DisabledScope_BalancesBeginEnd) {
    {
        unigui::DisabledScope d{true};
        ImGui::Button("disabled");
    }
    {
        unigui::DisabledScope d{false};
        ImGui::Button("enabled");
    }
    SUCCEED();
}

TEST_F(ScopeTest, GroupScope_BalancesBeginEnd) {
    {
        unigui::GroupScope g;
        ImGui::TextUnformatted("grouped");
    }
    SUCCEED();
}

TEST_F(ScopeTest, TabBarScope_BalancesBeginEnd) {
    unigui::WindowScope w{"Tabs"};
    {
        unigui::TabBarScope bar{"bar"};
        if (bar) {
            unigui::TabItemScope item{"One"};
            if (item) ImGui::TextUnformatted("tab one");
        }
    }
    SUCCEED();
}

TEST_F(ScopeTest, WindowScope_MoveLeavesStackBalanced) {
    {
        unigui::WindowScope a{"Movable"};
        unigui::WindowScope b{std::move(a)};  // only b should call End()
        if (b) ImGui::TextUnformatted("moved");
    }
    SUCCEED();
}

TEST_F(ScopeTest, Scopes_AreMoveOnly) {
    static_assert(!std::is_copy_constructible_v<unigui::WindowScope>);
    static_assert(std::is_move_constructible_v<unigui::WindowScope>);
    static_assert(!std::is_copy_constructible_v<unigui::IDScope>);
    static_assert(!std::is_copy_constructible_v<unigui::DisabledScope>);
    SUCCEED();
}
