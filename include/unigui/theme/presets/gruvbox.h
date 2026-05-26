#pragma once
#include <imgui.h>
namespace unigui::theme {
inline void ApplyGruvbox(ImGuiStyle& s){auto&c=s.Colors;
c[ImGuiCol_WindowBg]=ImVec4(0.16f,0.16f,0.16f,1.0f);c[ImGuiCol_ChildBg]=ImVec4(0.16f,0.16f,0.16f,1.0f);
c[ImGuiCol_PopupBg]=ImVec4(0.20f,0.20f,0.20f,1.0f);c[ImGuiCol_Border]=ImVec4(0.33f,0.31f,0.27f,0.50f);
c[ImGuiCol_FrameBg]=ImVec4(0.25f,0.24f,0.22f,0.60f);c[ImGuiCol_FrameBgHovered]=ImVec4(0.31f,0.30f,0.27f,1.0f);
c[ImGuiCol_FrameBgActive]=ImVec4(0.33f,0.32f,0.29f,1.0f);c[ImGuiCol_TitleBg]=ImVec4(0.16f,0.16f,0.16f,1.0f);
c[ImGuiCol_TitleBgActive]=ImVec4(0.22f,0.22f,0.22f,1.0f);c[ImGuiCol_TitleBgCollapsed]=ImVec4(0.16f,0.16f,0.16f,1.0f);
c[ImGuiCol_ScrollbarBg]=ImVec4(0.02f,0.02f,0.02f,0.30f);c[ImGuiCol_ScrollbarGrab]=ImVec4(0.40f,0.37f,0.32f,1.0f);
c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.50f,0.47f,0.42f,1.0f);
c[ImGuiCol_CheckMark]=ImVec4(0.84f,0.60f,0.13f,1.0f);c[ImGuiCol_SliderGrab]=ImVec4(0.84f,0.60f,0.13f,1.0f);
c[ImGuiCol_SliderGrabActive]=ImVec4(0.92f,0.66f,0.15f,1.0f);c[ImGuiCol_Button]=ImVec4(0.84f,0.60f,0.13f,0.90f);
c[ImGuiCol_ButtonHovered]=ImVec4(0.84f,0.60f,0.13f,1.0f);c[ImGuiCol_ButtonActive]=ImVec4(0.76f,0.54f,0.12f,1.0f);
c[ImGuiCol_Header]=ImVec4(0.84f,0.60f,0.13f,0.45f);c[ImGuiCol_HeaderHovered]=ImVec4(0.84f,0.60f,0.13f,0.65f);
c[ImGuiCol_HeaderActive]=ImVec4(0.84f,0.60f,0.13f,0.85f);c[ImGuiCol_Separator]=ImVec4(0.33f,0.31f,0.27f,0.50f);
c[ImGuiCol_Tab]=ImVec4(0.21f,0.21f,0.21f,1.0f);c[ImGuiCol_TabHovered]=ImVec4(0.84f,0.60f,0.13f,0.45f);
c[ImGuiCol_TabActive]=ImVec4(0.84f,0.60f,0.13f,1.0f);c[ImGuiCol_TabUnfocused]=ImVec4(0.21f,0.21f,0.21f,1.0f);
c[ImGuiCol_DockingPreview]=ImVec4(0.84f,0.60f,0.13f,0.50f);c[ImGuiCol_Text]=ImVec4(0.92f,0.90f,0.85f,1.0f);
c[ImGuiCol_TextDisabled]=ImVec4(0.92f,0.90f,0.85f,0.38f);c[ImGuiCol_ResizeGrip]=ImVec4(0.84f,0.60f,0.13f,0.25f);
c[ImGuiCol_NavHighlight]=ImVec4(0.84f,0.60f,0.13f,1.0f);c[ImGuiCol_ModalWindowDimBg]=ImVec4(0.00f,0.00f,0.00f,0.45f);
s.WindowRounding=4.f;s.FrameRounding=3.f;s.GrabRounding=3.f;s.TabRounding=3.f;s.ChildRounding=3.f;
s.PopupRounding=3.f;s.ScrollbarRounding=6.f;s.GrabMinSize=8.f;s.WindowBorderSize=0.f;s.FrameBorderSize=0.f;
}
}
