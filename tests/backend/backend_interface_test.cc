#include <unigui/backend/platform_backend.h>
#include <unigui/backend/renderer_backend.h>
#include <unigui/unigui.h>

#include <gtest/gtest.h>
#include <type_traits>

#include "mock_backends.h"

TEST(PlatformBackend, IsAbstract) {
    EXPECT_TRUE(std::is_abstract_v<unigui::PlatformBackend>);
}

TEST(RendererBackend, IsAbstract) {
    EXPECT_TRUE(std::is_abstract_v<unigui::RendererBackend>);
}

TEST(MockPlatform, InitReturnsTrue) {
    unigui::MockPlatformBackend backend;
    EXPECT_TRUE(backend.Init());
    EXPECT_TRUE(backend.init_called);
}

TEST(MockPlatform, InitPassesHandle) {
    unigui::MockPlatformBackend backend;
    int dummy = 42;
    backend.Init(&dummy);
    EXPECT_EQ(backend.handle_passed, &dummy);
}

TEST(MockPlatform, ShutdownCalled) {
    unigui::MockPlatformBackend backend;
    backend.Init();
    backend.Shutdown();
    EXPECT_TRUE(backend.shutdown_called);
}

TEST(MockPlatform, NewFrameCalled) {
    unigui::MockPlatformBackend backend;
    backend.NewFrame();
    EXPECT_TRUE(backend.new_frame_called);
}

TEST(MockRenderer, InitWithValidContextSucceeds) {
    unigui::MockRendererBackend backend;
    // Pass nullptr as context (mock doesn't use it)
    EXPECT_TRUE(backend.Init(nullptr));
    EXPECT_TRUE(backend.init_called);
}

TEST(MockRenderer, RenderDrawDataNullDoesNotCrash) {
    unigui::MockRendererBackend backend;
    backend.Init(nullptr);
    backend.RenderDrawData(nullptr);
    EXPECT_TRUE(backend.render_called);
    EXPECT_EQ(backend.last_draw_data, nullptr);
}

TEST(MockRenderer, SetClearColorStoresValues) {
    unigui::MockRendererBackend backend;
    backend.SetClearColor(0.1f, 0.2f, 0.3f, 1.0f);
    EXPECT_TRUE(backend.set_clear_color_called);
    EXPECT_FLOAT_EQ(backend.last_clear_r, 0.1f);
    EXPECT_FLOAT_EQ(backend.last_clear_g, 0.2f);
    EXPECT_FLOAT_EQ(backend.last_clear_b, 0.3f);
    EXPECT_FLOAT_EQ(backend.last_clear_a, 1.0f);
}
