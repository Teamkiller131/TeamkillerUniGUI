#pragma once
#include <imgui.h>
namespace unigui::theme {
inline void ApplyDracula(ImGuiStyle& s){auto&c=s.Colors;
c[ImGuiCol_WindowBg]=ImVec4(0.16f,0.16f,0.22f,1.0f);c[ImGuiCol_ChildBg]=ImVec4(0.16f,0.16f,0.22f,1.0f);
c[ImGuiCol_PopupBg]=ImVec4(0.20f,0.20f,0.27f,1.0f);c[ImGuiCol_Border]=ImVec4(0.38f,0.35f,0.50f,0.50f);
c[ImGuiCol_FrameBg]=ImVec4(0.27f,0.26f,0.36f,0.60f);c[ImGuiCol_FrameBgHovered]=ImVec4(0.33f,0.32f,0.42f,1.0f);
c[ImGuiCol_FrameBgActive]=ImVec4(0.35f,0.34f,0.45f,1.0f);c[ImGuiCol_TitleBg]=ImVec4(0.16f,0.16f,0.22f,1.0f);
c[ImGuiCol_TitleBgActive]=ImVec4(0.22f,0.22f,0.29f,1.0f);c[ImGuiCol_TitleBgCollapsed]=ImVec4(0.16f,0.16f,0.22f,1.0f);
c[ImGuiCol_ScrollbarBg]=ImVec4(0.02f,0.02f,0.03f,0.30f);c[ImGuiCol_ScrollbarGrab]=ImVec4(0.44f,0.40f,0.55f,1.0f);
c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.54f,0.50f,0.65f,1.0f);
c[ImGuiCol_CheckMark]=ImVec4(0.74f,0.60f,0.98f,1.0f);c[ImGuiCol_SliderGrab]=ImVec4(0.74f,0.60f,0.98f,1.0f);
c[ImGuiCol_SliderGrabActive]=ImVec4(0.82f,0.67f,1.0f,1.0f);c[ImGuiCol_Button]=ImVec4(0.74f,0.60f,0.98f,1.0f);
c[ImGuiCol_ButtonHovered]=ImVec4(0.80f,0.66f,1.0f,1.0f);c[ImGuiCol_ButtonActive]=ImVec4(0.66f,0.53f,0.88f,1.0f);
c[ImGuiCol_Header]=ImVec4(0.74f,0.60f,0.98f,0.50f);c[ImGuiCol_HeaderHovered]=ImVec4(0.74f,0.60f,0.98f,0.70f);
c[ImGuiCol_HeaderActive]=ImVec4(0.74f,0.60f,0.98f,0.90f);c[ImGuiCol_Separator]=ImVec4(0.38f,0.35f,0.50f,0.50f);
c[ImGuiCol_Tab]=ImVec4(0.21f,0.21f,0.28f,1.0f);c[ImGuiCol_TabHovered]=ImVec4(0.74f,0.60f,0.98f,0.50f);
c[ImGuiCol_TabActive]=ImVec4(0.74f,0.60f,0.98f,1.0f);c[ImGuiCol_TabUnfocused]=ImVec4(0.21f,0.21f,0.28f,1.0f);
c[ImGuiCol_DockingPreview]=ImVec4(0.74f,0.60f,0.98f,0.50f);c[ImGuiCol_Text]=ImVec4(0.97f,0.97f,0.99f,1.0f);
c[ImGuiCol_TextDisabled]=ImVec4(0.97f,0.97f,0.99f,0.38f);c[ImGuiCol_ResizeGrip]=ImVec4(0.74f,0.60f,0.98f,0.25f);
c[ImGuiCol_NavHighlight]=ImVec4(0.74f,0.60f,0.98f,1.0f);c[ImGuiCol_ModalWindowDimBg]=ImVec4(0.00f,0.00f,0.00f,0.45f);
s.WindowRounding=6.f;s.FrameRounding=4.f;s.GrabRounding=4.f;s.TabRounding=4.f;s.ChildRounding=4.f;
s.PopupRounding=4.f;s.ScrollbarRounding=9.f;s.GrabMinSize=10.f;s.WindowBorderSize=1.f;s.FrameBorderSize=1.f;
}
}
