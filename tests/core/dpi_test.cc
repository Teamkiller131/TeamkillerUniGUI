#include <unigui/core/dpi.h>

#include <cmath>
#include <gtest/gtest.h>
#include <limits>

using namespace unigui::dpi;

TEST(Dpi, DpiToScaleBaseline) {
    EXPECT_FLOAT_EQ(DpiToScale(96.0f), 1.0f);
    EXPECT_FLOAT_EQ(DpiToScale(120.0f), 1.25f);
    EXPECT_FLOAT_EQ(DpiToScale(144.0f), 1.5f);
    EXPECT_FLOAT_EQ(DpiToScale(192.0f), 2.0f);
}

TEST(Dpi, DpiToScaleCustomBase) {
    EXPECT_FLOAT_EQ(DpiToScale(160.0f, 80.0f), 2.0f);
}

TEST(Dpi, DpiToScaleNonPositiveFallsBackToOne) {
    EXPECT_FLOAT_EQ(DpiToScale(0.0f), 1.0f);
    EXPECT_FLOAT_EQ(DpiToScale(-100.0f), 1.0f);
    EXPECT_FLOAT_EQ(DpiToScale(144.0f, 0.0f), 1.0f);
}

TEST(Dpi, NormalizeSnapsFractionalToQuarterStep) {
    EXPECT_FLOAT_EQ(NormalizeContentScale(1.4583f), 1.5f); // GLFW "150%"
    EXPECT_FLOAT_EQ(NormalizeContentScale(1.2f), 1.25f);
    EXPECT_FLOAT_EQ(NormalizeContentScale(1.1f), 1.0f); // nearest 0.25 is 1.0
    EXPECT_FLOAT_EQ(NormalizeContentScale(1.0f), 1.0f);
    EXPECT_FLOAT_EQ(NormalizeContentScale(2.0f), 2.0f);
}

TEST(Dpi, NormalizeClampsToBounds) {
    EXPECT_FLOAT_EQ(NormalizeContentScale(0.5f), 1.0f);  // below default min
    EXPECT_FLOAT_EQ(NormalizeContentScale(10.0f), 4.0f); // above default max
    EXPECT_FLOAT_EQ(NormalizeContentScale(0.5f, 0.5f, 4.0f), 0.5f);
}

TEST(Dpi, NormalizeNoSnapWhenStepZero) {
    // step <= 0 → clamp only, fractional value preserved.
    EXPECT_FLOAT_EQ(NormalizeContentScale(1.4583f, 1.0f, 4.0f, 0.0f), 1.4583f);
}

TEST(Dpi, NormalizeHandlesNanAndNonPositive) {
    EXPECT_FLOAT_EQ(NormalizeContentScale(std::numeric_limits<float>::quiet_NaN()), 1.0f);
    EXPECT_FLOAT_EQ(NormalizeContentScale(0.0f), 1.0f);
    EXPECT_FLOAT_EQ(NormalizeContentScale(-2.0f), 1.0f);
}

TEST(Dpi, NormalizeSwapsReversedBounds) {
    // min/max passed in the wrong order must not break clamping.
    EXPECT_FLOAT_EQ(NormalizeContentScale(2.0f, 4.0f, 1.0f), 2.0f);
}

TEST(Dpi, NormalizeStaysWithinBoundsAfterSnap) {
    // A min that is not a multiple of step must still be respected.
    const float v = NormalizeContentScale(1.05f, 1.1f, 4.0f, 0.25f);
    EXPECT_GE(v, 1.1f);
    EXPECT_LE(v, 4.0f);
}
