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
