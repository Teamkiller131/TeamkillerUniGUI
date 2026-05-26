#pragma once
#include <imgui.h>
namespace unigui::theme {
inline void ApplyFluentDark(ImGuiStyle& s) {
    auto& c = s.Colors;
    c[ImGuiCol_WindowBg]       =ImVec4(0.12f,0.12f,0.12f,1.00f);
    c[ImGuiCol_ChildBg]        =ImVec4(0.12f,0.12f,0.12f,1.00f);
    c[ImGuiCol_PopupBg]        =ImVec4(0.18f,0.18f,0.18f,1.00f);
    c[ImGuiCol_Border]         =ImVec4(0.25f,0.25f,0.25f,0.50f);
    c[ImGuiCol_FrameBg]        =ImVec4(0.22f,0.22f,0.22f,0.60f);
    c[ImGuiCol_FrameBgHovered] =ImVec4(0.28f,0.28f,0.28f,1.00f);
    c[ImGuiCol_FrameBgActive]  =ImVec4(0.30f,0.30f,0.30f,1.00f);
    c[ImGuiCol_TitleBg]        =ImVec4(0.12f,0.12f,0.12f,1.00f);
    c[ImGuiCol_TitleBgActive]  =ImVec4(0.18f,0.18f,0.18f,1.00f);
    c[ImGuiCol_TitleBgCollapsed]=ImVec4(0.12f,0.12f,0.12f,1.00f);
    c[ImGuiCol_ScrollbarBg]    =ImVec4(0.02f,0.02f,0.02f,0.30f);
    c[ImGuiCol_ScrollbarGrab]  =ImVec4(0.31f,0.31f,0.31f,1.00f);
    c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.41f,0.41f,0.41f,1.00f);
    c[ImGuiCol_CheckMark]      =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_SliderGrab]     =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_SliderGrabActive]=ImVec4(0.00f,0.56f,0.96f,1.00f);
    c[ImGuiCol_Button]         =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_ButtonHovered]  =ImVec4(0.06f,0.54f,0.92f,1.00f);
    c[ImGuiCol_ButtonActive]   =ImVec4(0.00f,0.40f,0.72f,1.00f);
    c[ImGuiCol_Header]         =ImVec4(0.00f,0.47f,0.84f,0.45f);
    c[ImGuiCol_HeaderHovered]  =ImVec4(0.00f,0.47f,0.84f,0.65f);
    c[ImGuiCol_HeaderActive]   =ImVec4(0.00f,0.47f,0.84f,0.85f);
    c[ImGuiCol_Separator]      =ImVec4(0.25f,0.25f,0.25f,0.50f);
    c[ImGuiCol_Tab]            =ImVec4(0.15f,0.15f,0.15f,1.00f);
    c[ImGuiCol_TabHovered]     =ImVec4(0.00f,0.47f,0.84f,0.45f);
    c[ImGuiCol_TabActive]      =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_TabUnfocused]   =ImVec4(0.15f,0.15f,0.15f,1.00f);
    c[ImGuiCol_DockingPreview] =ImVec4(0.00f,0.47f,0.84f,0.50f);
    c[ImGuiCol_Text]           =ImVec4(0.95f,0.95f,0.95f,1.00f);
    c[ImGuiCol_TextDisabled]   =ImVec4(0.95f,0.95f,0.95f,0.38f);
    c[ImGuiCol_ResizeGrip]     =ImVec4(0.00f,0.47f,0.84f,0.25f);
    c[ImGuiCol_NavHighlight]   =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_ModalWindowDimBg]=ImVec4(0.00f,0.00f,0.00f,0.45f);
    s.WindowRounding=8.f;s.FrameRounding=4.f;s.GrabRounding=4.f;
    s.TabRounding=4.f;s.ChildRounding=4.f;s.PopupRounding=4.f;
    s.ScrollbarRounding=4.f;s.GrabMinSize=8.f;
    s.WindowBorderSize=1.f;s.FrameBorderSize=1.f;
}
inline void ApplyFluentLight(ImGuiStyle& s) {
    auto& c = s.Colors;
    c[ImGuiCol_WindowBg]       =ImVec4(1.00f,1.00f,1.00f,1.00f);
    c[ImGuiCol_ChildBg]        =ImVec4(1.00f,1.00f,1.00f,1.00f);
    c[ImGuiCol_PopupBg]        =ImVec4(0.98f,0.98f,0.98f,1.00f);
    c[ImGuiCol_Border]         =ImVec4(0.85f,0.85f,0.85f,0.50f);
    c[ImGuiCol_FrameBg]        =ImVec4(0.88f,0.88f,0.88f,0.60f);
    c[ImGuiCol_FrameBgHovered] =ImVec4(0.82f,0.82f,0.82f,1.00f);
    c[ImGuiCol_FrameBgActive]  =ImVec4(0.80f,0.80f,0.80f,1.00f);
    c[ImGuiCol_TitleBg]        =ImVec4(0.96f,0.96f,0.96f,1.00f);
    c[ImGuiCol_TitleBgActive]  =ImVec4(0.92f,0.92f,0.92f,1.00f);
    c[ImGuiCol_TitleBgCollapsed]=ImVec4(0.96f,0.96f,0.96f,1.00f);
    c[ImGuiCol_ScrollbarBg]    =ImVec4(0.98f,0.98f,0.98f,0.30f);
    c[ImGuiCol_ScrollbarGrab]  =ImVec4(0.70f,0.70f,0.70f,1.00f);
    c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.60f,0.60f,0.60f,1.00f);
    c[ImGuiCol_CheckMark]      =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_SliderGrab]     =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_SliderGrabActive]=ImVec4(0.00f,0.56f,0.96f,1.00f);
    c[ImGuiCol_Button]         =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_ButtonHovered]  =ImVec4(0.06f,0.54f,0.92f,1.00f);
    c[ImGuiCol_ButtonActive]   =ImVec4(0.00f,0.40f,0.72f,1.00f);
    c[ImGuiCol_Header]         =ImVec4(0.00f,0.47f,0.84f,0.25f);
    c[ImGuiCol_HeaderHovered]  =ImVec4(0.00f,0.47f,0.84f,0.45f);
    c[ImGuiCol_HeaderActive]   =ImVec4(0.00f,0.47f,0.84f,0.65f);
    c[ImGuiCol_Separator]      =ImVec4(0.85f,0.85f,0.85f,0.50f);
    c[ImGuiCol_Tab]            =ImVec4(0.90f,0.90f,0.90f,1.00f);
    c[ImGuiCol_TabHovered]     =ImVec4(0.00f,0.47f,0.84f,0.30f);
    c[ImGuiCol_TabActive]      =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_TabUnfocused]   =ImVec4(0.90f,0.90f,0.90f,1.00f);
    c[ImGuiCol_DockingPreview] =ImVec4(0.00f,0.47f,0.84f,0.50f);
    c[ImGuiCol_Text]           =ImVec4(0.08f,0.08f,0.08f,1.00f);
    c[ImGuiCol_TextDisabled]   =ImVec4(0.08f,0.08f,0.08f,0.38f);
    c[ImGuiCol_ResizeGrip]     =ImVec4(0.00f,0.47f,0.84f,0.25f);
    c[ImGuiCol_NavHighlight]   =ImVec4(0.00f,0.47f,0.84f,1.00f);
    c[ImGuiCol_ModalWindowDimBg]=ImVec4(0.00f,0.00f,0.00f,0.30f);
    s.WindowRounding=8.f;s.FrameRounding=4.f;s.GrabRounding=4.f;
    s.TabRounding=4.f;s.ChildRounding=4.f;s.PopupRounding=4.f;
    s.ScrollbarRounding=4.f;s.GrabMinSize=8.f;
    s.WindowBorderSize=1.f;s.FrameBorderSize=1.f;
}
}
