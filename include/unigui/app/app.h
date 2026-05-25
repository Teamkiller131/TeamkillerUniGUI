#pragma once

#include <unigui/theme/theme.h>
#include <unigui/backend/backend_types.h>
#include <functional>

namespace unigui {

struct AppConfig {
    int width = 1280;
    int height = 720;
    const char* title = "UniGUI Application";
    ThemeConfig theme = { ThemePreset::Dark, 1.0f, 16.0f };
    BackendType backend = BackendType::GLFW_GL3;
};

bool Init(const AppConfig& config);
void Shutdown();
bool NewFrame();
void Render();
bool ShouldClose();
void Run(const std::function<void()>& callback);

} // namespace unigui
