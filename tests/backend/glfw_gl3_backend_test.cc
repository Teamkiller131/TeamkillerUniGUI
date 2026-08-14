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
    void SetUp() override {
        glfw_init_ok_ = glfwInit();
        if (!glfw_init_ok_)
            GTEST_SKIP() << "GLFW could not initialize (headless CI without a display)";
        // Probe whether a real OpenGL 3.3 core context is obtainable. GPU-less CI
        // runners expose only a generic GL 1.1 driver, so a 3.3-core window fails
        // to create — skip the GL-dependent suite there instead of hard-failing
        // (mirrors render_integration_test.cc's headless skip).
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        GLFWwindow* probe = glfwCreateWindow(64, 48, "probe", nullptr, nullptr);
        if (!probe)
            GTEST_SKIP() << "No OpenGL 3.3 core context available (headless / GPU-less CI)";
        glfwDestroyWindow(probe);
    }
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

TEST_F(BackendTest, GLFWPlatform_GetMonitors_ReportsConsistentDisplays) {
    ASSERT_TRUE(glfw_init_ok_);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto platform = unigui::CreateGLFWPlatform();
    ASSERT_TRUE(platform->Init(nullptr));

    const auto monitors = platform->GetMonitors();
    ASSERT_FALSE(monitors.empty()) << "a desktop session must report at least one monitor";
    for (const auto& m : monitors) {
        EXPECT_GT(m.width, 0);
        EXPECT_GT(m.height, 0);
        EXPECT_GT(m.dpiScale, 0.0f) << "content scale must be positive";
        // The work area must be a sub-rect of the display rect.
        EXPECT_GE(m.workX, m.x - 1);
        EXPECT_GE(m.workY, m.y - 1);
        EXPECT_LE(m.workX + m.workWidth, m.x + m.width + 1);
        EXPECT_LE(m.workY + m.workHeight, m.y + m.height + 1);
    }
    platform->Shutdown();
    ImGui::DestroyContext();
}

TEST_F(BackendTest, GLFWPlatform_GetMonitors_BeforeInit_Empty) {
    auto platform = unigui::CreateGLFWPlatform();
    EXPECT_TRUE(platform->GetMonitors().empty())
        << "monitor enumeration needs an initialized GLFW library";
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

// ── Fractional-DPI wiring (runtime content-scale batch) ───────────────────────

TEST_F(BackendTest, GLFWPlatform_ReportsFramebufferScaleToIO) {
    ASSERT_TRUE(glfw_init_ok_);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto platform = unigui::CreateGLFWPlatform();
    ASSERT_TRUE(platform->Init(nullptr));
    // The platform must tell ImGui the physical/logical ratio: without it the render
    // backends rasterize at the wrong physical size on any non-1.0 monitor.
    const ImVec2 fs = ImGui::GetIO().DisplayFramebufferScale;
    EXPECT_GT(fs.x, 0.0f);
    EXPECT_FLOAT_EQ(fs.x, fs.y);
    EXPECT_FLOAT_EQ(fs.x, platform->GetContentScale());
    platform->Shutdown();
    ImGui::DestroyContext();
}

TEST_F(BackendTest, GLFWPlatform_ContentScaleCallback_DoesNotFireOnSteadyScale) {
    // Headless CI cannot move the window across monitors, so the change itself isn't
    // simulated — the contract pinned here is registration + no-fire while the scale
    // is steady (the polling lives in NewFrame).
    ASSERT_TRUE(glfw_init_ok_);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    auto platform = unigui::CreateGLFWPlatform();
    ASSERT_TRUE(platform->Init(nullptr));
    int fired = 0;
    platform->SetContentScaleCallback([&](float) { ++fired; });
    platform->NewFrame();
    platform->NewFrame();
    EXPECT_EQ(fired, 0);
    platform->Shutdown();
    ImGui::DestroyContext();
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
