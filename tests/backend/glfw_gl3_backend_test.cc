// clang-format off
#include <glad/glad.h> // glad must precede GLFW and unigui.h (which pulls in GLFW->GL on Windows)
#include <GLFW/glfw3.h>
// clang-format on

#include <unigui/backend/backend_factory.h>
#include <unigui/core/context.h>
#include <unigui/unigui.h>

#include <imgui.h>

#include <cstdio>
#include <gtest/gtest.h>
#include <memory>

class BackendTest : public ::testing::Test {
protected:
    void SetUp() override { glfw_init_ok_ = glfwInit(); }
    void TearDown() override {
        if (glfw_init_ok_)
            glfwTerminate();
    }

    GLFWwindow* CreateHiddenWindow() {
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        return glfwCreateWindow(640, 480, "Test", nullptr, nullptr);
    }
    bool glfw_init_ok_ = false;
};

TEST_F(BackendTest, GLFWPlatform_Init_Default_Succeeds) {
    ASSERT_TRUE(glfw_init_ok_);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto platform = unigui::CreateGLFWPlatform();
    EXPECT_TRUE(platform->Init(nullptr));
    EXPECT_FALSE(platform->ShouldClose());
    platform->Shutdown();
    ImGui::DestroyContext();
}
TEST_F(BackendTest, GLFWPlatform_PollEvents_DoesNotCrash) {
    ASSERT_TRUE(glfw_init_ok_);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto platform = unigui::CreateGLFWPlatform();
    ASSERT_TRUE(platform->Init(nullptr));
    platform->PollEvents();
    platform->Shutdown();
    ImGui::DestroyContext();
}
TEST_F(BackendTest, GLFWPlatform_Shutdown_AfterInit_CleansUp) {
    ASSERT_TRUE(glfw_init_ok_);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto platform = unigui::CreateGLFWPlatform();
    ASSERT_TRUE(platform->Init(nullptr));
    platform->Shutdown();
    platform->Shutdown();
    ImGui::DestroyContext();
}
TEST_F(BackendTest, OpenGL3Renderer_Init_WithoutContext_Succeeds) {
    ASSERT_TRUE(glfw_init_ok_);
    auto window = CreateHiddenWindow();
    ASSERT_NE(window, nullptr);
    glfwMakeContextCurrent(window);
    ASSERT_TRUE(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));
    auto renderer = unigui::CreateOpenGL3Renderer();
    EXPECT_TRUE(renderer->Init(nullptr));
    renderer->Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
}
TEST_F(BackendTest, OpenGL3Renderer_Init_WithValidContext_Succeeds) {
    ASSERT_TRUE(glfw_init_ok_);
    auto window = CreateHiddenWindow();
    ASSERT_NE(window, nullptr);
    glfwMakeContextCurrent(window);
    ASSERT_TRUE(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));
    auto renderer = unigui::CreateOpenGL3Renderer();
    EXPECT_TRUE(renderer->Init(nullptr));
    renderer->Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
}
TEST_F(BackendTest, OpenGL3Renderer_RenderDrawData_Null_DoesNotCrash) {
    ASSERT_TRUE(glfw_init_ok_);
    auto window = CreateHiddenWindow();
    ASSERT_NE(window, nullptr);
    glfwMakeContextCurrent(window);
    ASSERT_TRUE(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));
    auto renderer = unigui::CreateOpenGL3Renderer();
    ASSERT_TRUE(renderer->Init(nullptr));
    renderer->RenderDrawData(nullptr);
    renderer->Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
}
TEST_F(BackendTest, OpenGL3Renderer_SetClearColor_Works) {
    ASSERT_TRUE(glfw_init_ok_);
    auto window = CreateHiddenWindow();
    ASSERT_NE(window, nullptr);
    glfwMakeContextCurrent(window);
    ASSERT_TRUE(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress));
    auto renderer = unigui::CreateOpenGL3Renderer();
    ASSERT_TRUE(renderer->Init(nullptr));
    renderer->SetClearColor(0.2f, 0.3f, 0.4f, 1.0f);
    GLfloat color[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
    EXPECT_FLOAT_EQ(color[0], 0.2f);
    EXPECT_FLOAT_EQ(color[1], 0.3f);
    EXPECT_FLOAT_EQ(color[2], 0.4f);
    EXPECT_FLOAT_EQ(color[3], 1.0f);
    renderer->Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
}

// ── Diagnostic: render a frame and check for GL errors ────────────────────
TEST_F(BackendTest, RenderDrawData_NoGLError) {
    ASSERT_TRUE(glfw_init_ok_);
    auto window = CreateHiddenWindow();
    ASSERT_NE(window, nullptr);
    glfwMakeContextCurrent(window);
    ASSERT_TRUE(gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) << "gladLoadGLLoader failed";

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::GetIO().DisplaySize = ImVec2(800, 600);

    auto renderer = unigui::CreateOpenGL3Renderer();
    ASSERT_TRUE(renderer->Init(nullptr)) << "renderer Init failed";

    ImGui::GetIO().Fonts->Build(); // Must be AFTER renderer Init (backend registers textures)

    // Frame 1 (warmup — first frame is always empty)
    ImGui::NewFrame();
    ImGui::Begin("T");
    ImGui::Text("H");
    ImGui::End();
    ImGui::Render();
    renderer->RenderDrawData(ImGui::GetDrawData());

    // Frame 2 — with glClear before RenderDrawData (matches app behavior)
    while (glGetError() != GL_NO_ERROR) {}
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::Render();

    renderer->SetClearColor(0.1f, 0.1f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    ImDrawData* dd = ImGui::GetDrawData();
    renderer->RenderDrawData(dd);

    // Check for GL errors
    GLenum err = glGetError();
    while (err != GL_NO_ERROR) {
        std::fprintf(stderr, "[TEST] GL error 0x%04x after RenderDrawData\n", (unsigned) err);
        err = glGetError();
    }
    EXPECT_EQ(err, (GLenum) GL_NO_ERROR) << "RenderDrawData produced GL errors";

    renderer->Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
}
