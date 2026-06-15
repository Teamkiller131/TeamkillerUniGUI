#include <unigui/core/session_axis.h>

#include <gtest/gtest.h>

using namespace unigui;

TEST(SessionAxisTest, TotalCollapsesGaps) {
    auto ax = SessionAxis::AShareFutures(); // 09:30–11:30 + 13:00–15:00 = 4h
    EXPECT_EQ(ax.TotalSeconds(), 4 * 3600);
}

TEST(SessionAxisTest, MapsSessionTimes) {
    auto ax = SessionAxis::AShareFutures();
    EXPECT_NEAR(ax.ToAxis(9 * 3600 + 30 * 60), 0.0, 1e-6);      // open
    EXPECT_NEAR(ax.ToAxis(11 * 3600), 5400.0, 1e-6);            // 11:00 → 1.5h in
    EXPECT_NEAR(ax.ToAxis(13 * 3600), 7200.0, 1e-6);            // afternoon open → 2h
    EXPECT_NEAR(ax.ToAxis(14 * 3600), 10800.0, 1e-6);           // 14:00 → 3h
    EXPECT_NEAR(ax.ToAxis(15 * 3600), 14400.0, 1e-6);           // close → 4h
}

TEST(SessionAxisTest, CollapsesLunchGap) {
    auto ax = SessionAxis::AShareFutures();
    // 11:30 (morning end), 12:00 (lunch), 13:00 (afternoon open) all map to 7200.
    EXPECT_NEAR(ax.ToAxis(11 * 3600 + 30 * 60), 7200.0, 1e-6);
    EXPECT_NEAR(ax.ToAxis(12 * 3600), 7200.0, 1e-6);
    EXPECT_NEAR(ax.ToAxis(13 * 3600), 7200.0, 1e-6);
}

TEST(SessionAxisTest, ClampsOutsideHours) {
    auto ax = SessionAxis::AShareFutures();
    EXPECT_NEAR(ax.ToAxis(8 * 3600), 0.0, 1e-6);       // pre-open
    EXPECT_NEAR(ax.ToAxis(16 * 3600), 14400.0, 1e-6);  // post-close
}

TEST(SessionAxisTest, RoundTripsWithinSessions) {
    auto ax = SessionAxis::AShareFutures();
    for (int t : {9 * 3600 + 30 * 60, 10 * 3600, 11 * 3600, 13 * 3600 + 30 * 60,
                  14 * 3600 + 59 * 60}) {
        const double a = ax.ToAxis(t);
        EXPECT_EQ(ax.FromAxis(a), t) << "t=" << t;
    }
}

TEST(SessionAxisTest, FormatsHHMM) {
    auto ax = SessionAxis::AShareFutures();
    EXPECT_EQ(ax.FormatAxis(0.0), "09:30");
    EXPECT_EQ(ax.FormatAxis(7200.0), "13:00");
    EXPECT_EQ(ax.FormatAxis(10800.0), "14:00");
    EXPECT_EQ(ax.FormatAxis(14400.0), "15:00");
}

TEST(SessionAxisTest, CustomSpans) {
    SessionAxis ax({{0, 100}, {200, 250}}); // total 150
    EXPECT_EQ(ax.TotalSeconds(), 150);
    EXPECT_NEAR(ax.ToAxis(50), 50.0, 1e-6);
    EXPECT_NEAR(ax.ToAxis(150), 100.0, 1e-6); // gap → end of first span
    EXPECT_NEAR(ax.ToAxis(220), 120.0, 1e-6); // 100 + 20
}
