#pragma once

#include <unigui/backend/platform_backend.h>
#include <unigui/backend/renderer_backend.h>

// Mock implementations for testing backend interfaces

namespace unigui {

class MockPlatformBackend : public PlatformBackend {
public:
    bool Init(void* native_window_handle = nullptr) override {
        init_called = true;
        handle_passed = native_window_handle;
        return init_return_value;
    }
    void Shutdown() override { shutdown_called = true; }
    void NewFrame() override { new_frame_called = true; }
    void PollEvents() override { poll_events_called = true; }
    bool ShouldClose() const override { return should_close_value; }

    bool init_called = false;
    bool shutdown_called = false;
    bool new_frame_called = false;
    bool poll_events_called = false;
    void* handle_passed = nullptr;
    bool init_return_value = true;
    bool should_close_value = false;
};

class MockRendererBackend : public RendererBackend {
public:
    bool Init(ImGuiContext* context) override {
        init_called = true;
        context_passed = context;
        return init_return_value;
    }
    void Shutdown() override { shutdown_called = true; }
    void RenderDrawData(ImDrawData* draw_data) override {
        render_called = true;
        last_draw_data = draw_data;
    }
    void SetClearColor(float r, float g, float b, float a) override {
        set_clear_color_called = true;
        last_clear_r = r;
        last_clear_g = g;
        last_clear_b = b;
        last_clear_a = a;
    }

    bool init_called = false;
    bool shutdown_called = false;
    bool render_called = false;
    bool set_clear_color_called = false;
    ImGuiContext* context_passed = nullptr;
    ImDrawData* last_draw_data = nullptr;
    bool init_return_value = true;
    float last_clear_r = 0, last_clear_g = 0, last_clear_b = 0, last_clear_a = 0;
};

} // namespace unigui
