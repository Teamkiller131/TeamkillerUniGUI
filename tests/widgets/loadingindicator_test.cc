#include <unigui/unigui.h>
#include <unigui/widgets/loadingindicator.h>
#include <imgui.h>
#include <gtest/gtest.h>

class LoadingIndicatorTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

// ---- basic rendering ----

TEST_F(LoadingIndicatorTest, Render_DoesNotCrash) {
    unigui::LoadingIndicator li("li");
    li.Render();
}

TEST_F(LoadingIndicatorTest, Render_DefaultRadius) {
    unigui::LoadingIndicator li("li");
    EXPECT_TRUE(li.IsActive());
    li.Render();
}

TEST_F(LoadingIndicatorTest, Render_CustomRadius) {
    unigui::LoadingIndicator li("li", 32.0f);
    EXPECT_TRUE(li.IsActive());
    li.Render();
}

TEST_F(LoadingIndicatorTest, Render_SmallRadius) {
    unigui::LoadingIndicator li("li", 4.0f);
    li.Render();
}

TEST_F(LoadingIndicatorTest, Render_LargeRadius) {
    unigui::LoadingIndicator li("li", 128.0f);
    li.Render();
}

// ---- active/inactive ----

TEST_F(LoadingIndicatorTest, IsActive_DefaultsToTrue) {
    unigui::LoadingIndicator li("li");
    EXPECT_TRUE(li.IsActive());
}

TEST_F(LoadingIndicatorTest, SetActive_True) {
    unigui::LoadingIndicator li("li");
    li.SetActive(true);
    EXPECT_TRUE(li.IsActive());
    li.Render();
}

TEST_F(LoadingIndicatorTest, SetActive_False) {
    unigui::LoadingIndicator li("li");
    li.SetActive(false);
    EXPECT_FALSE(li.IsActive());
}

TEST_F(LoadingIndicatorTest, Render_WhenInactive) {
    unigui::LoadingIndicator li("li");
    li.SetActive(false);
    li.Render(); // Should be a no-op render
    EXPECT_FALSE(li.IsActive());
}

TEST_F(LoadingIndicatorTest, ToggleActive_Repeatedly) {
    unigui::LoadingIndicator li("li");
    li.SetActive(false);
    li.Render();
    EXPECT_FALSE(li.IsActive());

    li.SetActive(true);
    li.Render();
    EXPECT_TRUE(li.IsActive());

    li.SetActive(false);
    li.Render();
    EXPECT_FALSE(li.IsActive());

    li.SetActive(true);
    li.Render();
    EXPECT_TRUE(li.IsActive());
}

// ---- multiple renders (animation test) ----

TEST_F(LoadingIndicatorTest, MultipleRenders_DoesNotCrash) {
    unigui::LoadingIndicator li("li");
    for (int i = 0; i < 10; ++i) {
        li.Render();
    }
}

TEST_F(LoadingIndicatorTest, MultipleRenders_Inactive) {
    unigui::LoadingIndicator li("li");
    li.SetActive(false);
    for (int i = 0; i < 10; ++i) {
        li.Render();
    }
}

// ---- visibility ----

TEST_F(LoadingIndicatorTest, Hidden_DoesNotRender) {
    unigui::LoadingIndicator li("li");
    li.Hide();
    li.Render();
    EXPECT_FALSE(li.IsVisible());
}

TEST_F(LoadingIndicatorTest, Show_AfterHide) {
    unigui::LoadingIndicator li("li");
    li.Hide();
    EXPECT_FALSE(li.IsVisible());
    li.Show();
    EXPECT_TRUE(li.IsVisible());
    li.Render();
}

TEST_F(LoadingIndicatorTest, HiddenAndInactive) {
    unigui::LoadingIndicator li("li");
    li.Hide();
    li.SetActive(false);
    li.Render();
    EXPECT_FALSE(li.IsVisible());
    EXPECT_FALSE(li.IsActive());
}

// ---- base Widget features ----

TEST_F(LoadingIndicatorTest, GetName_ReturnsName) {
    unigui::LoadingIndicator li("loading_id");
    EXPECT_EQ(li.GetName(), "loading_id");
}

TEST_F(LoadingIndicatorTest, Tooltip_DoesNotCrash) {
    unigui::LoadingIndicator li("li");
    li.SetTooltip("Loading...");
    li.Render();
}
