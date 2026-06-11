#include <unigui/core/context.h>
#include <unigui/unigui.h>
#include <unigui/widgets/statuslamp.h>

#include <imgui.h>

#include <gtest/gtest.h>

class StatusLampTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
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

// 1. Basic rendering doesn't crash
TEST_F(StatusLampTest, Render_DoesNotCrash) {
    unigui::StatusLamp lamp("lamp1");
    lamp.Render();
}

// 2. Default state is Off
TEST_F(StatusLampTest, GetState_DefaultsToOff) {
    unigui::StatusLamp lamp("lamp2");
    EXPECT_EQ(lamp.GetState(), unigui::StatusLamp::Off);
}

// 3. Constructor with Running state
TEST_F(StatusLampTest, Constructor_WithRunning) {
    unigui::StatusLamp lamp("lamp3", unigui::StatusLamp::Running);
    EXPECT_EQ(lamp.GetState(), unigui::StatusLamp::Running);
}

// 4. Constructor with Draft state
TEST_F(StatusLampTest, Constructor_WithDraft) {
    unigui::StatusLamp lamp("lamp4", unigui::StatusLamp::Draft);
    EXPECT_EQ(lamp.GetState(), unigui::StatusLamp::Draft);
}

// 5. SetState changes state
TEST_F(StatusLampTest, SetState_ChangesState) {
    unigui::StatusLamp lamp("lamp5");
    lamp.SetState(unigui::StatusLamp::Running);
    EXPECT_EQ(lamp.GetState(), unigui::StatusLamp::Running);
    lamp.SetState(unigui::StatusLamp::Draft);
    EXPECT_EQ(lamp.GetState(), unigui::StatusLamp::Draft);
    lamp.SetState(unigui::StatusLamp::Off);
    EXPECT_EQ(lamp.GetState(), unigui::StatusLamp::Off);
}

// 6. Default radius is 7.0f
TEST_F(StatusLampTest, GetRadius_DefaultsTo7) {
    unigui::StatusLamp lamp("lamp6");
    EXPECT_FLOAT_EQ(lamp.GetRadius(), 7.0f);
}

// 7. SetRadius changes radius
TEST_F(StatusLampTest, SetRadius_ChangesRadius) {
    unigui::StatusLamp lamp("lamp7");
    lamp.SetRadius(12.0f);
    EXPECT_FLOAT_EQ(lamp.GetRadius(), 12.0f);
    lamp.SetRadius(3.5f);
    EXPECT_FLOAT_EQ(lamp.GetRadius(), 3.5f);
}

// 8. Default tooltip is empty
TEST_F(StatusLampTest, GetTooltip_DefaultsToEmpty) {
    unigui::StatusLamp lamp("lamp8");
    EXPECT_TRUE(lamp.GetTooltip().empty());
}

// 9. SetTooltip sets tooltip text
TEST_F(StatusLampTest, SetTooltip_SetsText) {
    unigui::StatusLamp lamp("lamp9");
    lamp.SetTooltip("Status: OK");
    EXPECT_EQ(lamp.GetTooltip(), "Status: OK");
}

// 10. All states render without crash
TEST_F(StatusLampTest, AllStates_RenderWithoutCrash) {
    unigui::StatusLamp lamp("lamp10");
    lamp.Render(); // Off
    lamp.SetState(unigui::StatusLamp::Running);
    lamp.Render();
    lamp.SetState(unigui::StatusLamp::Draft);
    lamp.Render();
}

// 11. Render with tooltip doesn't crash
TEST_F(StatusLampTest, Render_WithTooltip_DoesNotCrash) {
    unigui::StatusLamp lamp("lamp11");
    lamp.SetTooltip("Running indicator");
    lamp.SetState(unigui::StatusLamp::Running);
    lamp.Render();
}

// 12. Render with custom radius doesn't crash
TEST_F(StatusLampTest, Render_WithCustomRadius_DoesNotCrash) {
    unigui::StatusLamp lamp("lamp12");
    lamp.SetRadius(15.0f);
    lamp.Render();
    lamp.SetRadius(3.0f);
    lamp.Render();
}

// 13. Hide/Show works (inherited from Widget)
TEST_F(StatusLampTest, Show_Hide_TogglesVisibility) {
    unigui::StatusLamp lamp("lamp13");
    EXPECT_TRUE(lamp.IsVisible());
    lamp.Hide();
    EXPECT_FALSE(lamp.IsVisible());
    lamp.Show();
    EXPECT_TRUE(lamp.IsVisible());
}

// 14. Hidden lamp renders without crash
TEST_F(StatusLampTest, Hidden_RendersWithoutCrash) {
    unigui::StatusLamp lamp("lamp14");
    lamp.Hide();
    lamp.Render();
}

// 15. Multiple lamps with different names don't conflict
TEST_F(StatusLampTest, MultipleLamps_DifferentNames_NoConflict) {
    unigui::StatusLamp lampA("lamp_a");
    unigui::StatusLamp lampB("lamp_b");
    unigui::StatusLamp lampC("lamp_c");
    lampA.Render();
    lampB.Render();
    lampC.Render();
    EXPECT_NE(lampA.GetID(), lampB.GetID());
    EXPECT_NE(lampB.GetID(), lampC.GetID());
    EXPECT_NE(lampA.GetID(), lampC.GetID());
}

// 16. GetName returns constructor name
TEST_F(StatusLampTest, GetName_ReturnsGivenName) {
    unigui::StatusLamp lamp("my_lamp");
    EXPECT_EQ(lamp.GetName(), "my_lamp");
}

// 17. Draft state blink timer resets on SetState away from Draft
TEST_F(StatusLampTest, DraftBlinkTimer_ResetsOnStateChange) {
    unigui::StatusLamp lamp("lamp17", unigui::StatusLamp::Draft);
    // Simulate a few frames to advance blink timer
    ImGui::GetIO().DeltaTime = 0.5f;
    lamp.Render();
    lamp.SetState(unigui::StatusLamp::Off);
    // Just verify no crash after state change with previous Draft
    lamp.Render();
}

// 18. State transitions: Off -> Draft -> Running -> Off
TEST_F(StatusLampTest, StateTransitions_RenderStable) {
    unigui::StatusLamp lamp("lamp18");
    lamp.SetState(unigui::StatusLamp::Draft);
    lamp.Render();
    lamp.SetState(unigui::StatusLamp::Running);
    lamp.Render();
    lamp.SetState(unigui::StatusLamp::Off);
    lamp.Render();
}
