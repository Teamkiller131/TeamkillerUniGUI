/// Integration test: verifies DX11/OpenGL render pipeline produces no GL errors.
/// Requires a visible window + real GPU. Skipped in headless CI.
#include <unigui/unigui.h>
#include <unigui/backend/backend_factory.h>
#ifdef UNIGUI_HAS_DX11
#include <unigui/backend/dx11_renderer.h>
#include <imgui_impl_dx11.h>
#endif
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <gtest/gtest.h>

#ifdef _WIN32
class RenderTest : public ::testing::Test {
protected:
    void SetUp() override {
        if (!glfwInit()) { skip_ = true; return; }
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        window_ = glfwCreateWindow(800, 600, "RenderTest", nullptr, nullptr);
        if (!window_) { skip_ = true; return; }
        glfwMakeContextCurrent(window_);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) { skip_ = true; return; }
        IMGUI_CHECKVERSION(); ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
    }
    void TearDown() override {
        if (!skip_) {
            ImGui::DestroyContext();
            glfwDestroyWindow(window_);
            glfwTerminate();
        }
    }
    GLFWwindow* window_ = nullptr;
    bool skip_ = false;
};

TEST_F(RenderTest, OpenGL3_RenderFrame_NoErrors) {
    if (skip_) GTEST_SKIP();
    auto renderer = unigui::CreateOpenGL3Renderer();
    ASSERT_TRUE(renderer->Init(nullptr));

    for (int f = 0; f < 3; f++) {
        ImGui::NewFrame();
        ImGui::Begin("Test"); ImGui::Text("Frame %d", f); ImGui::End();
        ImGui::Render();
        while (glGetError() != GL_NO_ERROR) {}
        ImDrawData* dd = ImGui::GetDrawData();
        ASSERT_NE(dd, nullptr);
        renderer->RenderDrawData(dd);
        GLenum err = glGetError();
        EXPECT_EQ(err, (GLenum)GL_NO_ERROR) << "GL error on frame " << f << ": 0x" << std::hex << err;
        while (glGetError() != GL_NO_ERROR) {}
    }
    renderer->Shutdown();
}

TEST_F(RenderTest, MultiWidget_RenderFrame_NoErrors) {
    if (skip_) GTEST_SKIP();
    auto renderer = unigui::CreateOpenGL3Renderer();
    ASSERT_TRUE(renderer->Init(nullptr));

    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();
    while (glGetError() != GL_NO_ERROR) {}
    ImDrawData* dd = ImGui::GetDrawData();
    renderer->RenderDrawData(dd);
    GLenum err = glGetError();
    EXPECT_EQ(err, (GLenum)GL_NO_ERROR) << "GL error after ShowDemoWindow: 0x" << std::hex << err;
    renderer->Shutdown();
}
#endif
