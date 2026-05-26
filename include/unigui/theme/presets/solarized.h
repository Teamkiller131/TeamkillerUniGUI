#pragma once
#include <imgui.h>
namespace unigui::theme {
inline void ApplySolarizedDark(ImGuiStyle& s){auto&c=s.Colors;
c[ImGuiCol_WindowBg]=ImVec4(0.01f,0.17f,0.21f,1.0f);c[ImGuiCol_ChildBg]=ImVec4(0.01f,0.17f,0.21f,1.0f);
c[ImGuiCol_PopupBg]=ImVec4(0.03f,0.21f,0.26f,1.0f);c[ImGuiCol_Border]=ImVec4(0.06f,0.29f,0.36f,0.50f);
c[ImGuiCol_FrameBg]=ImVec4(0.06f,0.23f,0.28f,0.60f);c[ImGuiCol_FrameBgHovered]=ImVec4(0.09f,0.27f,0.33f,1.0f);
c[ImGuiCol_FrameBgActive]=ImVec4(0.11f,0.29f,0.35f,1.0f);c[ImGuiCol_TitleBg]=ImVec4(0.01f,0.17f,0.21f,1.0f);
c[ImGuiCol_TitleBgActive]=ImVec4(0.04f,0.20f,0.25f,1.0f);c[ImGuiCol_TitleBgCollapsed]=ImVec4(0.01f,0.17f,0.21f,1.0f);
c[ImGuiCol_ScrollbarBg]=ImVec4(0.00f,0.03f,0.04f,0.30f);c[ImGuiCol_ScrollbarGrab]=ImVec4(0.16f,0.37f,0.44f,1.0f);
c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.20f,0.42f,0.49f,1.0f);
c[ImGuiCol_CheckMark]=ImVec4(0.15f,0.55f,0.82f,1.0f);c[ImGuiCol_SliderGrab]=ImVec4(0.15f,0.55f,0.82f,1.0f);
c[ImGuiCol_SliderGrabActive]=ImVec4(0.17f,0.61f,0.90f,1.0f);c[ImGuiCol_Button]=ImVec4(0.15f,0.55f,0.82f,1.0f);
c[ImGuiCol_ButtonHovered]=ImVec4(0.17f,0.61f,0.90f,1.0f);c[ImGuiCol_ButtonActive]=ImVec4(0.13f,0.49f,0.74f,1.0f);
c[ImGuiCol_Header]=ImVec4(0.15f,0.55f,0.82f,0.45f);c[ImGuiCol_HeaderHovered]=ImVec4(0.15f,0.55f,0.82f,0.65f);
c[ImGuiCol_HeaderActive]=ImVec4(0.15f,0.55f,0.82f,0.85f);c[ImGuiCol_Separator]=ImVec4(0.06f,0.29f,0.36f,0.50f);
c[ImGuiCol_Tab]=ImVec4(0.03f,0.19f,0.23f,1.0f);c[ImGuiCol_TabHovered]=ImVec4(0.15f,0.55f,0.82f,0.45f);
c[ImGuiCol_TabActive]=ImVec4(0.15f,0.55f,0.82f,1.0f);c[ImGuiCol_TabUnfocused]=ImVec4(0.03f,0.19f,0.23f,1.0f);
c[ImGuiCol_DockingPreview]=ImVec4(0.15f,0.55f,0.82f,0.50f);c[ImGuiCol_Text]=ImVec4(0.51f,0.58f,0.59f,1.0f);
c[ImGuiCol_TextDisabled]=ImVec4(0.51f,0.58f,0.59f,0.38f);c[ImGuiCol_ResizeGrip]=ImVec4(0.15f,0.55f,0.82f,0.25f);
c[ImGuiCol_NavHighlight]=ImVec4(0.15f,0.55f,0.82f,1.0f);c[ImGuiCol_ModalWindowDimBg]=ImVec4(0.00f,0.00f,0.00f,0.45f);
s.WindowRounding=4.f;s.FrameRounding=3.f;s.GrabRounding=3.f;s.TabRounding=3.f;s.ChildRounding=3.f;
s.PopupRounding=3.f;s.ScrollbarRounding=6.f;s.GrabMinSize=8.f;s.WindowBorderSize=0.f;s.FrameBorderSize=0.f;
}
inline void ApplySolarizedLight(ImGuiStyle& s){auto&c=s.Colors;
c[ImGuiCol_WindowBg]=ImVec4(0.99f,0.96f,0.89f,1.0f);c[ImGuiCol_ChildBg]=ImVec4(0.99f,0.96f,0.89f,1.0f);
c[ImGuiCol_PopupBg]=ImVec4(0.97f,0.94f,0.87f,1.0f);c[ImGuiCol_Border]=ImVec4(0.86f,0.84f,0.79f,0.50f);
c[ImGuiCol_FrameBg]=ImVec4(0.93f,0.91f,0.84f,0.60f);c[ImGuiCol_FrameBgHovered]=ImVec4(0.89f,0.87f,0.80f,1.0f);
c[ImGuiCol_FrameBgActive]=ImVec4(0.87f,0.85f,0.78f,1.0f);c[ImGuiCol_TitleBg]=ImVec4(0.96f,0.93f,0.85f,1.0f);
c[ImGuiCol_TitleBgActive]=ImVec4(0.92f,0.89f,0.82f,1.0f);c[ImGuiCol_TitleBgCollapsed]=ImVec4(0.96f,0.93f,0.85f,1.0f);
c[ImGuiCol_ScrollbarBg]=ImVec4(0.98f,0.96f,0.90f,0.30f);c[ImGuiCol_ScrollbarGrab]=ImVec4(0.69f,0.67f,0.62f,1.0f);
c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.59f,0.57f,0.52f,1.0f);
c[ImGuiCol_CheckMark]=ImVec4(0.15f,0.55f,0.82f,1.0f);c[ImGuiCol_SliderGrab]=ImVec4(0.15f,0.55f,0.82f,1.0f);
c[ImGuiCol_Button]=ImVec4(0.15f,0.55f,0.82f,1.0f);c[ImGuiCol_ButtonHovered]=ImVec4(0.17f,0.61f,0.90f,1.0f);
c[ImGuiCol_ButtonActive]=ImVec4(0.13f,0.49f,0.74f,1.0f);c[ImGuiCol_Header]=ImVec4(0.15f,0.55f,0.82f,0.25f);
c[ImGuiCol_HeaderHovered]=ImVec4(0.15f,0.55f,0.82f,0.45f);c[ImGuiCol_HeaderActive]=ImVec4(0.15f,0.55f,0.82f,0.65f);
c[ImGuiCol_Separator]=ImVec4(0.86f,0.84f,0.79f,0.50f);c[ImGuiCol_Tab]=ImVec4(0.93f,0.91f,0.84f,1.0f);
c[ImGuiCol_TabHovered]=ImVec4(0.15f,0.55f,0.82f,0.30f);c[ImGuiCol_TabActive]=ImVec4(0.15f,0.55f,0.82f,1.0f);
c[ImGuiCol_TabUnfocused]=ImVec4(0.93f,0.91f,0.84f,1.0f);c[ImGuiCol_DockingPreview]=ImVec4(0.15f,0.55f,0.82f,0.50f);
c[ImGuiCol_Text]=ImVec4(0.40f,0.36f,0.31f,1.0f);c[ImGuiCol_TextDisabled]=ImVec4(0.40f,0.36f,0.31f,0.38f);
c[ImGuiCol_NavHighlight]=ImVec4(0.15f,0.55f,0.82f,1.0f);c[ImGuiCol_ModalWindowDimBg]=ImVec4(0.00f,0.00f,0.00f,0.30f);
s.WindowRounding=4.f;s.FrameRounding=3.f;s.GrabRounding=3.f;s.TabRounding=3.f;s.ChildRounding=3.f;
s.PopupRounding=3.f;s.ScrollbarRounding=6.f;s.GrabMinSize=8.f;s.WindowBorderSize=0.f;s.FrameBorderSize=0.f;
}
}
