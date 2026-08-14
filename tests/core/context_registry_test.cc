#include <unigui/fonts/font_manager.h>
#include <unigui/fx/animation.h>
#include <unigui/styling/style_engine.h>
#include <unigui/widgets/toast.h>

#include <imgui.h>

#include <gtest/gtest.h>

// Multi-context: the singletons migrated to ContextRegistry must resolve through
// the CURRENT ImGui context — two independent surfaces in one process get
// independent instances (the first increment of the roadmap's multi-context item).

using namespace unigui;

class ContextRegistryTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ctxA_ = ImGui::CreateContext();
        ImGui::GetIO().Fonts->Build();
        ctxB_ = ImGui::CreateContext();
        ImGui::GetIO().Fonts->Build();
        ImGui::SetCurrentContext(nullptr); // start on the no-context default
    }
    void TearDown() override {
        ImGui::SetCurrentContext(ctxA_);
        ImGui::DestroyContext(ctxA_);
        ImGui::SetCurrentContext(ctxB_);
        ImGui::DestroyContext(ctxB_);
        ImGui::SetCurrentContext(nullptr);
    }
    ImGuiContext* ctxA_ = nullptr;
    ImGuiContext* ctxB_ = nullptr;
};

TEST_F(ContextRegistryTest, FontManager_InstancesArePerContext) {
    ImGui::SetCurrentContext(ctxA_);
    fonts::Manager& a = fonts::Manager::Instance();
    ImGui::SetCurrentContext(ctxB_);
    fonts::Manager& b = fonts::Manager::Instance();
    EXPECT_NE(&a, &b) << "two contexts must get independent font managers";

    ImGui::SetCurrentContext(ctxA_);
    EXPECT_EQ(&fonts::Manager::Instance(), &a) << "the per-context instance must be stable";
    ImGui::SetCurrentContext(ctxB_);
    EXPECT_EQ(&fonts::Manager::Instance(), &b);
}

TEST_F(ContextRegistryTest, Toast_FactoryKeepsItsIdName) {
    ImGui::SetCurrentContext(ctxA_);
    Toast& a = Toast::Instance();
    ImGui::SetCurrentContext(ctxB_);
    Toast& b = Toast::Instance();
    EXPECT_NE(&a, &b);
    // The migrated factory must preserve the original singleton's widget name so
    // the toast's ImGui ID (and thus any saved state) is unchanged.
    EXPECT_EQ(a.GetName(), "_toast");
    EXPECT_EQ(b.GetName(), "_toast");
}

TEST_F(ContextRegistryTest, NoContext_FallsBackToSharedDefault) {
    ImGui::SetCurrentContext(nullptr);
    fonts::Manager& d1 = fonts::Manager::Instance();
    fonts::Manager& d2 = fonts::Manager::Instance();
    EXPECT_EQ(&d1, &d2) << "the no-context default stays a single shared instance";
    ImGui::SetCurrentContext(ctxA_);
    EXPECT_NE(&fonts::Manager::Instance(), &d1);
}

TEST_F(ContextRegistryTest, StylingAndAnimation_ArePerContext) {
    ImGui::SetCurrentContext(ctxA_);
    styling::Engine& seA = styling::Engine::Instance();
    fx::AnimationManager& amA = fx::AnimationManager::Instance();
    ImGui::SetCurrentContext(ctxB_);
    EXPECT_NE(&styling::Engine::Instance(), &seA);
    EXPECT_NE(&fx::AnimationManager::Instance(), &amA);
}
