#include <unigui/widgets/connection_status.h>

#include <imgui.h>

#include <gtest/gtest.h>

using namespace unigui;

class ConnectionStatusBarTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(1000, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(ConnectionStatusBarTest, Connected_RendersWithoutCrash) {
    ConnectionStatusBar bar("conn");
    bar.WithConnected(true)
        .WithCaption("Relay 192.168.1.240")
        .WithLatencyUs(850.0, 1200.0)
        .WithFps(60.f);
    EXPECT_NO_THROW(bar.Render());
}

TEST_F(ConnectionStatusBarTest, Disconnected_WithReconnectCountdown) {
    ConnectionStatusBar bar("conn2");
    bar.WithConnected(false).WithCaption("Server").WithReconnectIn(3.0);
    EXPECT_NO_THROW(bar.Render());
}

TEST_F(ConnectionStatusBarTest, Sparkline_AcceptsSamplesAndRenders) {
    ConnectionStatusBar bar("conn3");
    bar.WithConnected(true).WithSparkline(true).WithLatencyThresholds(2000.0, 10000.0);
    for (int i = 0; i < 200; ++i)
        bar.PushLatencySample(500.0 + i * 10.0);
    bar.WithLatencyUs(2500.0);
    EXPECT_NO_THROW(bar.Render());
}

TEST_F(ConnectionStatusBarTest, HighLatency_GradesWithoutCrash) {
    ConnectionStatusBar bar("conn4");
    bar.WithConnected(true).WithLatencyThresholds(2000.0, 10000.0).WithLatencyUs(15000.0); // crit
    EXPECT_NO_THROW(bar.Render());
}
