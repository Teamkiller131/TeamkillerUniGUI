#include <unigui/unigui.h>
#include <unigui/widgets/notification.h>
#include <imgui.h>
#include <gtest/gtest.h>
class NotificationTest : public ::testing::Test {
protected:
    void SetUp() override { ImGui::CreateContext(); ImGui::GetIO().DisplaySize=ImVec2(800,600); ImGui::GetIO().Fonts->Build(); ImGui::NewFrame(); }
    void TearDown() override { ImGui::Render(); ImGui::DestroyContext(); }
};
TEST_F(NotificationTest, PendingCount_DefaultsToZero) { unigui::Notification nf("nf"); EXPECT_EQ(nf.PendingCount(),0u); }
TEST_F(NotificationTest, Show_IncreasesCount) { unigui::Notification nf("nf"); nf.Show("Title","Message"); EXPECT_EQ(nf.PendingCount(),1u); }
