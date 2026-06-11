#include <unigui/unigui.h>
#include <unigui/widgets/image.h>

#include <imgui.h>

#include <gtest/gtest.h>
class ImageTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};
TEST_F(ImageTest, Render_NoTexture_DoesNotCrash) {
    unigui::Image img("img");
    img.Render();
}
TEST_F(ImageTest, Render_WithDummyTexture_DoesNotCrash) {
    unigui::Image img("img", (void*) 1, 100, 100);
    img.Render();
}
