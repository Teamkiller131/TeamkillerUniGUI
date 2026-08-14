// RTL mirroring geometry tests for the im text primitives.
//
// Increment 1 of RTL layout: with LayoutDirection::RightToLeft, single-line
// text blocks (Text/TextDisabled/TextColored/LabelText) must right-align —
// their item rect ends at the window's right edge and starts left of it by
// the measured width, while in LTR the same text starts at the left edge.
// Asserted headlessly via ImGui::GetItemRectMin/Max inside a fixed-size
// window (no render backend needed — only the layout math).
#include <unigui/core/layout_direction.h>
#include <unigui/im/im.h>

#include <imgui.h>
#include <imgui_internal.h> // ImRect for the geometry assertions

#include <gtest/gtest.h>

namespace im = unigui::im;

class ImRtlTest : public ::testing::Test {
protected:
    void SetUp() override {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
        unigui::SetLayoutDirection(unigui::LayoutDirection::LeftToRight); // leave the world LTR
    }

    // Draw `text` at the top of a fixed 300-px-wide window and return its item
    // rect relative to the window's content region.
    ImRect DrawInFixedWindow(std::string_view text) {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
        ImGui::Begin("rtl_win", nullptr, ImGuiWindowFlags_NoSavedSettings);
        im::Text(text);
        const ImRect rect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
        ImGui::End();
        return rect;
    }
};

TEST_F(ImRtlTest, Ltr_TextStartsAtLeftEdge) {
    unigui::SetLayoutDirection(unigui::LayoutDirection::LeftToRight);
    const ImRect rect = DrawInFixedWindow("abc");
    // The text must hug the left edge, not the right: its center sits far left
    // of the 300-px window's center.
    const float center = (rect.Min.x + rect.Max.x) * 0.5f;
    EXPECT_LT(center, 100.0f) << "LTR text must start near the window's left edge";
    EXPECT_GT(rect.GetWidth(), 10.0f);
}

TEST_F(ImRtlTest, Rtl_TextEndsAtRightEdge) {
    unigui::SetLayoutDirection(unigui::LayoutDirection::RightToLeft);
    const ImRect rect = DrawInFixedWindow("abc");
    // The text block must END at the right edge of the 300-px window content
    // (window spans x 10..310; content right edge ~ 310 - padding) and START
    // left of it — i.e. it hugs the right, not the left.
    EXPECT_GT(rect.Max.x, 280.0f) << "RTL text must end at the window's right edge";
    // "abc" is a short string: its left edge must sit just left of the right
    // edge (right-aligned) and far from the window's left edge (~18 px).
    EXPECT_GT(rect.Min.x, 240.0f) << "RTL text must NOT start at the left edge";
    EXPECT_LT(rect.Min.x, 300.0f) << "RTL text must start left of its right-aligned end";
}

TEST_F(ImRtlTest, Rtl_MirrorsAllSingleLinePrimitives) {
    unigui::SetLayoutDirection(unigui::LayoutDirection::RightToLeft);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 160), ImGuiCond_Always);
    ImGui::Begin("rtl_win2", nullptr, ImGuiWindowFlags_NoSavedSettings);
    const float rightEdge = ImGui::GetWindowContentRegionMax().x;

    im::Text("plain");
    EXPECT_GT(ImGui::GetItemRectMax().x, rightEdge - 40.0f) << "Text";
    im::TextDisabled("disabled");
    EXPECT_GT(ImGui::GetItemRectMax().x, rightEdge - 40.0f) << "TextDisabled";
    im::TextColored(ImVec4(1, 0, 0, 1), "colored");
    EXPECT_GT(ImGui::GetItemRectMax().x, rightEdge - 40.0f) << "TextColored";
    im::LabelText("Label", "Value");
    EXPECT_GT(ImGui::GetItemRectMax().x, rightEdge - 40.0f) << "LabelText";
    ImGui::End();
}

TEST_F(ImRtlTest, Ltr_Default_DoesNotRightAlign) {
    // Fresh world (TearDown resets, but be explicit): the default direction is
    // LTR and text must NOT jump to the right edge.
    unigui::SetLayoutDirection(unigui::LayoutDirection::LeftToRight);
    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_Always);
    ImGui::Begin("rtl_win3", nullptr, ImGuiWindowFlags_NoSavedSettings);
    const float rightEdge = ImGui::GetWindowContentRegionMax().x;
    im::Text("plain");
    EXPECT_LT(ImGui::GetItemRectMax().x, rightEdge - 200.0f)
        << "default LTR text must stay left, far from the right edge";
    ImGui::End();
}
