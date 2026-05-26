#pragma once
#include <imgui.h>

namespace unigui::theme {

inline void ApplyMaterialDark(ImGuiStyle& s) {
    auto& c = s.Colors;
    c[ImGuiCol_WindowBg]          = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
    c[ImGuiCol_ChildBg]           = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
    c[ImGuiCol_PopupBg]           = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
    c[ImGuiCol_Border]            = ImVec4(0.28f, 0.27f, 0.31f, 0.60f);
    c[ImGuiCol_FrameBg]           = ImVec4(0.20f, 0.19f, 0.23f, 0.60f);
    c[ImGuiCol_FrameBgHovered]    = ImVec4(0.26f, 0.25f, 0.30f, 1.00f);
    c[ImGuiCol_FrameBgActive]     = ImVec4(0.28f, 0.27f, 0.31f, 1.00f);
    c[ImGuiCol_TitleBg]           = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
    c[ImGuiCol_TitleBgActive]     = ImVec4(0.18f, 0.17f, 0.20f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed]  = ImVec4(0.11f, 0.11f, 0.14f, 1.00f);
    c[ImGuiCol_ScrollbarBg]       = ImVec4(0.02f, 0.02f, 0.02f, 0.30f);
    c[ImGuiCol_ScrollbarGrab]     = ImVec4(0.31f, 0.31f, 0.31f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.41f, 0.41f, 0.41f, 1.00f);
    c[ImGuiCol_CheckMark]         = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_SliderGrab]        = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_SliderGrabActive]  = ImVec4(0.50f, 0.39f, 0.80f, 1.00f);
    c[ImGuiCol_Button]            = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_ButtonHovered]     = ImVec4(0.48f, 0.37f, 0.77f, 1.00f);
    c[ImGuiCol_ButtonActive]      = ImVec4(0.38f, 0.28f, 0.60f, 1.00f);
    c[ImGuiCol_Header]            = ImVec4(0.40f, 0.31f, 0.64f, 0.50f);
    c[ImGuiCol_HeaderHovered]     = ImVec4(0.40f, 0.31f, 0.64f, 0.70f);
    c[ImGuiCol_HeaderActive]      = ImVec4(0.40f, 0.31f, 0.64f, 0.90f);
    c[ImGuiCol_Separator]         = ImVec4(0.28f, 0.27f, 0.31f, 0.60f);
    c[ImGuiCol_Tab]               = ImVec4(0.15f, 0.14f, 0.18f, 1.00f);
    c[ImGuiCol_TabHovered]        = ImVec4(0.40f, 0.31f, 0.64f, 0.50f);
    c[ImGuiCol_TabActive]         = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_TabUnfocused]      = ImVec4(0.15f, 0.14f, 0.18f, 1.00f);
    c[ImGuiCol_DockingPreview]    = ImVec4(0.40f, 0.31f, 0.64f, 0.50f);
    c[ImGuiCol_Text]              = ImVec4(0.90f, 0.89f, 0.93f, 1.00f);
    c[ImGuiCol_TextDisabled]      = ImVec4(0.90f, 0.89f, 0.93f, 0.38f);
    c[ImGuiCol_ResizeGrip]        = ImVec4(0.40f, 0.31f, 0.64f, 0.25f);
    c[ImGuiCol_NavHighlight]      = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_ModalWindowDimBg]  = ImVec4(0.00f, 0.00f, 0.00f, 0.45f);
    // Rounded corners
    s.WindowRounding = 12.f; s.FrameRounding = 6.f; s.GrabRounding = 12.f;
    s.TabRounding = 8.f; s.ChildRounding = 8.f; s.PopupRounding = 8.f;
    s.ScrollbarRounding = 9.f; s.GrabMinSize = 10.f;
    s.WindowBorderSize = 1.f; s.FrameBorderSize = 1.f;
}

inline void ApplyMaterialLight(ImGuiStyle& s) {
    auto& c = s.Colors;
    c[ImGuiCol_WindowBg]          = ImVec4(0.98f, 0.97f, 0.95f, 1.00f);
    c[ImGuiCol_ChildBg]           = ImVec4(0.98f, 0.97f, 0.95f, 1.00f);
    c[ImGuiCol_PopupBg]           = ImVec4(0.95f, 0.94f, 0.92f, 1.00f);
    c[ImGuiCol_Border]            = ImVec4(0.80f, 0.79f, 0.78f, 0.40f);
    c[ImGuiCol_FrameBg]           = ImVec4(0.89f, 0.88f, 0.86f, 0.60f);
    c[ImGuiCol_FrameBgHovered]    = ImVec4(0.84f, 0.83f, 0.81f, 1.00f);
    c[ImGuiCol_FrameBgActive]     = ImVec4(0.82f, 0.81f, 0.79f, 1.00f);
    c[ImGuiCol_TitleBg]           = ImVec4(0.93f, 0.92f, 0.90f, 1.00f);
    c[ImGuiCol_TitleBgActive]     = ImVec4(0.89f, 0.88f, 0.86f, 1.00f);
    c[ImGuiCol_TitleBgCollapsed]  = ImVec4(0.93f, 0.92f, 0.90f, 1.00f);
    c[ImGuiCol_ScrollbarBg]       = ImVec4(0.98f, 0.97f, 0.95f, 0.30f);
    c[ImGuiCol_ScrollbarGrab]     = ImVec4(0.69f, 0.68f, 0.66f, 1.00f);
    c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.59f, 0.58f, 0.56f, 1.00f);
    c[ImGuiCol_CheckMark]         = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_SliderGrab]        = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_SliderGrabActive]  = ImVec4(0.50f, 0.39f, 0.80f, 1.00f);
    c[ImGuiCol_Button]            = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_ButtonHovered]     = ImVec4(0.48f, 0.37f, 0.77f, 1.00f);
    c[ImGuiCol_ButtonActive]      = ImVec4(0.38f, 0.28f, 0.60f, 1.00f);
    c[ImGuiCol_Header]            = ImVec4(0.40f, 0.31f, 0.64f, 0.30f);
    c[ImGuiCol_HeaderHovered]     = ImVec4(0.40f, 0.31f, 0.64f, 0.50f);
    c[ImGuiCol_HeaderActive]      = ImVec4(0.40f, 0.31f, 0.64f, 0.70f);
    c[ImGuiCol_Separator]         = ImVec4(0.80f, 0.79f, 0.78f, 0.40f);
    c[ImGuiCol_Tab]               = ImVec4(0.89f, 0.88f, 0.86f, 1.00f);
    c[ImGuiCol_TabHovered]        = ImVec4(0.40f, 0.31f, 0.64f, 0.30f);
    c[ImGuiCol_TabActive]         = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_TabUnfocused]      = ImVec4(0.89f, 0.88f, 0.86f, 1.00f);
    c[ImGuiCol_DockingPreview]    = ImVec4(0.40f, 0.31f, 0.64f, 0.50f);
    c[ImGuiCol_Text]              = ImVec4(0.10f, 0.09f, 0.08f, 1.00f);
    c[ImGuiCol_TextDisabled]      = ImVec4(0.10f, 0.09f, 0.08f, 0.38f);
    c[ImGuiCol_ResizeGrip]        = ImVec4(0.40f, 0.31f, 0.64f, 0.25f);
    c[ImGuiCol_NavHighlight]      = ImVec4(0.40f, 0.31f, 0.64f, 1.00f);
    c[ImGuiCol_ModalWindowDimBg]  = ImVec4(0.00f, 0.00f, 0.00f, 0.30f);
    s.WindowRounding = 12.f; s.FrameRounding = 6.f; s.GrabRounding = 12.f;
    s.TabRounding = 8.f; s.ChildRounding = 8.f; s.PopupRounding = 8.f;
    s.ScrollbarRounding = 9.f; s.GrabMinSize = 10.f;
    s.WindowBorderSize = 1.f; s.FrameBorderSize = 1.f;
}

} // namespace unigui::theme
