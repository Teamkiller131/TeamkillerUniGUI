#include <unigui/theme/style_scope.h>
#include <unigui/unigui.h>

#include <imgui.h>

#include <gtest/gtest.h>

class StyleScopeTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        // Set a known baseline color
        ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(StyleScopeTest, PushColor_ModifiesColor) {
    ImVec4 before = ImGui::GetStyle().Colors[ImGuiCol_Button];
    {
        unigui::StyleScope scope;
        scope.PushColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        ImVec4 inside = ImGui::GetStyle().Colors[ImGuiCol_Button];
        EXPECT_FLOAT_EQ(inside.x, 1.0f);
    }
    ImVec4 after = ImGui::GetStyle().Colors[ImGuiCol_Button];
    EXPECT_FLOAT_EQ(after.x, 0.5f);
}

TEST_F(StyleScopeTest, PushVar_ModifiesStyleVar) {
    float before = ImGui::GetStyle().FrameRounding;
    {
        unigui::StyleScope scope;
        scope.PushVar(ImGuiStyleVar_FrameRounding, 20.0f);
        EXPECT_FLOAT_EQ(ImGui::GetStyle().FrameRounding, 20.0f);
    }
    EXPECT_FLOAT_EQ(ImGui::GetStyle().FrameRounding, before);
}

TEST_F(StyleScopeTest, MoveSemantics_TransfersOwnership) {
    ImVec4 baseline = ImGui::GetStyle().Colors[ImGuiCol_Button];
    {
        unigui::StyleScope a;
        a.PushColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        EXPECT_FLOAT_EQ(ImGui::GetStyle().Colors[ImGuiCol_Button].x, 0.0f);

        unigui::StyleScope b(std::move(a));
        EXPECT_FALSE(a.active());
        EXPECT_TRUE(b.active());
        EXPECT_FLOAT_EQ(ImGui::GetStyle().Colors[ImGuiCol_Button].x, 0.0f);
    }
    EXPECT_FLOAT_EQ(ImGui::GetStyle().Colors[ImGuiCol_Button].x, 0.5f);
}

TEST_F(StyleScopeTest, MultiplePushes_AllPopped) {
    ImVec4 baseline = ImGui::GetStyle().Colors[ImGuiCol_Button];
    {
        unigui::StyleScope scope;
        scope.PushColor(ImGuiCol_Button, ImVec4(1.0f, 0.0f, 0.0f, 1.0f));
        scope.PushColor(ImGuiCol_Button, ImVec4(0.0f, 1.0f, 0.0f, 1.0f));
        EXPECT_FLOAT_EQ(ImGui::GetStyle().Colors[ImGuiCol_Button].y, 1.0f);
    }
    EXPECT_FLOAT_EQ(ImGui::GetStyle().Colors[ImGuiCol_Button].x, 0.5f);
}
