#pragma once
#include <imgui.h>
namespace unigui::theme {
inline void ApplyNord(ImGuiStyle& s){auto&c=s.Colors;
c[ImGuiCol_WindowBg]=ImVec4(0.18f,0.20f,0.25f,1.0f);c[ImGuiCol_ChildBg]=ImVec4(0.18f,0.20f,0.25f,1.0f);
c[ImGuiCol_PopupBg]=ImVec4(0.23f,0.25f,0.30f,1.0f);c[ImGuiCol_Border]=ImVec4(0.30f,0.33f,0.40f,0.50f);
c[ImGuiCol_FrameBg]=ImVec4(0.27f,0.29f,0.35f,0.60f);c[ImGuiCol_FrameBgHovered]=ImVec4(0.33f,0.35f,0.42f,1.0f);
c[ImGuiCol_FrameBgActive]=ImVec4(0.35f,0.37f,0.44f,1.0f);c[ImGuiCol_TitleBg]=ImVec4(0.18f,0.20f,0.25f,1.0f);
c[ImGuiCol_TitleBgActive]=ImVec4(0.24f,0.26f,0.32f,1.0f);c[ImGuiCol_TitleBgCollapsed]=ImVec4(0.18f,0.20f,0.25f,1.0f);
c[ImGuiCol_ScrollbarBg]=ImVec4(0.02f,0.03f,0.04f,0.30f);c[ImGuiCol_ScrollbarGrab]=ImVec4(0.41f,0.44f,0.53f,1.0f);
c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.51f,0.54f,0.63f,1.0f);
c[ImGuiCol_CheckMark]=ImVec4(0.53f,0.75f,0.82f,1.0f);c[ImGuiCol_SliderGrab]=ImVec4(0.53f,0.75f,0.82f,1.0f);
c[ImGuiCol_SliderGrabActive]=ImVec4(0.60f,0.82f,0.89f,1.0f);c[ImGuiCol_Button]=ImVec4(0.53f,0.75f,0.82f,1.0f);
c[ImGuiCol_ButtonHovered]=ImVec4(0.58f,0.80f,0.88f,1.0f);c[ImGuiCol_ButtonActive]=ImVec4(0.47f,0.67f,0.73f,1.0f);
c[ImGuiCol_Header]=ImVec4(0.53f,0.75f,0.82f,0.45f);c[ImGuiCol_HeaderHovered]=ImVec4(0.53f,0.75f,0.82f,0.65f);
c[ImGuiCol_HeaderActive]=ImVec4(0.53f,0.75f,0.82f,0.85f);c[ImGuiCol_Separator]=ImVec4(0.30f,0.33f,0.40f,0.50f);
c[ImGuiCol_Tab]=ImVec4(0.23f,0.25f,0.30f,1.0f);c[ImGuiCol_TabHovered]=ImVec4(0.53f,0.75f,0.82f,0.45f);
c[ImGuiCol_TabActive]=ImVec4(0.53f,0.75f,0.82f,1.0f);c[ImGuiCol_TabUnfocused]=ImVec4(0.23f,0.25f,0.30f,1.0f);
c[ImGuiCol_DockingPreview]=ImVec4(0.53f,0.75f,0.82f,0.50f);c[ImGuiCol_Text]=ImVec4(0.85f,0.87f,0.91f,1.0f);
c[ImGuiCol_TextDisabled]=ImVec4(0.85f,0.87f,0.91f,0.38f);c[ImGuiCol_ResizeGrip]=ImVec4(0.53f,0.75f,0.82f,0.25f);
c[ImGuiCol_NavHighlight]=ImVec4(0.53f,0.75f,0.82f,1.0f);c[ImGuiCol_ModalWindowDimBg]=ImVec4(0.00f,0.00f,0.00f,0.45f);
s.WindowRounding=6.f;s.FrameRounding=4.f;s.GrabRounding=4.f;s.TabRounding=4.f;s.ChildRounding=4.f;
s.PopupRounding=4.f;s.ScrollbarRounding=9.f;s.GrabMinSize=10.f;s.WindowBorderSize=0.f;s.FrameBorderSize=0.f;
}
}
