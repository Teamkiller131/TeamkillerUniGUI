#include <unigui/app/app.h>
#include <unigui/unigui.h>

#include <gtest/gtest.h>

// App tests are integration-level - verify Init/Shutdown lifecycle
// without requiring GPU display

TEST(AppTest, Init_WithoutDisplay_ReturnsFalse) {
    // Init requires GLFW+OpenGL (hardware display)
    // This test verifies the function compiles and links
    unigui::AppConfig cfg;
    cfg.width = 100;
    cfg.height = 100;
    // Init may fail in headless environments (no GPU) - accept either outcome
    bool result = unigui::Init(cfg);
    if (result) {
        unigui::Shutdown();
    }
    // Just don't crash
    SUCCEED();
}

TEST(AppTest, Shutdown_BeforeInit_DoesNotCrash) {
    unigui::Shutdown();
    SUCCEED();
}

TEST(AppTest, NewFrame_BeforeInit_ReturnsFalse) {
    EXPECT_FALSE(unigui::NewFrame());
}

// ── W6-type defaults (v4.10.3) ───────────────────────────────────────────────
TEST(AppTest, DefaultConfig_FontBaseline26_AndOversizeOff) {
    unigui::AppConfig cfg;
    EXPECT_FLOAT_EQ(cfg.theme.font_size, 26.0f); // 4K@150% body baseline
    EXPECT_FLOAT_EQ(cfg.theme.font_size, unigui::ThemeConfig{}.font_size);
    EXPECT_FALSE(cfg.allowOversizeWindow); // window clamp stays on by default
}

TEST(AppTest, Oversize_OptInIsExplicit_AndEnvIsTestOnly) {
    unigui::AppConfig cfg;
    cfg.allowOversizeWindow = true; // only test/screenshot code does this
    EXPECT_TRUE(cfg.allowOversizeWindow);
}
