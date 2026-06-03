#pragma once
#include <imgui.h>
#include <unigui/theme/style_tokens.h>
namespace unigui::theme {
inline void ApplyCatppuccinMocha(ImGuiStyle& s){auto&c=s.Colors;
c[ImGuiCol_WindowBg]=ImVec4(0.12f,0.12f,0.18f,1.0f);c[ImGuiCol_ChildBg]=ImVec4(0.12f,0.12f,0.18f,1.0f);
c[ImGuiCol_PopupBg]=ImVec4(0.16f,0.16f,0.23f,1.0f);c[ImGuiCol_Border]=ImVec4(0.34f,0.33f,0.44f,0.50f);
c[ImGuiCol_FrameBg]=ImVec4(0.23f,0.22f,0.30f,0.60f);c[ImGuiCol_FrameBgHovered]=ImVec4(0.29f,0.28f,0.37f,1.0f);
c[ImGuiCol_FrameBgActive]=ImVec4(0.31f,0.30f,0.40f,1.0f);c[ImGuiCol_TitleBg]=ImVec4(0.12f,0.12f,0.18f,1.0f);
c[ImGuiCol_TitleBgActive]=ImVec4(0.18f,0.18f,0.25f,1.0f);c[ImGuiCol_TitleBgCollapsed]=ImVec4(0.12f,0.12f,0.18f,1.0f);
c[ImGuiCol_ScrollbarBg]=ImVec4(0.02f,0.02f,0.03f,0.30f);c[ImGuiCol_ScrollbarGrab]=ImVec4(0.41f,0.40f,0.53f,1.0f);
c[ImGuiCol_ScrollbarGrabHovered]=ImVec4(0.51f,0.50f,0.63f,1.0f);
c[ImGuiCol_CheckMark]=ImVec4(0.80f,0.65f,0.96f,1.0f);c[ImGuiCol_SliderGrab]=ImVec4(0.80f,0.65f,0.96f,1.0f);
c[ImGuiCol_SliderGrabActive]=ImVec4(0.87f,0.72f,1.00f,1.0f);c[ImGuiCol_Button]=ImVec4(0.80f,0.65f,0.96f,1.0f);
c[ImGuiCol_ButtonHovered]=ImVec4(0.85f,0.70f,1.00f,1.0f);c[ImGuiCol_ButtonActive]=ImVec4(0.72f,0.58f,0.86f,1.0f);
c[ImGuiCol_Header]=ImVec4(0.80f,0.65f,0.96f,0.45f);c[ImGuiCol_HeaderHovered]=ImVec4(0.80f,0.65f,0.96f,0.65f);
c[ImGuiCol_HeaderActive]=ImVec4(0.80f,0.65f,0.96f,0.85f);c[ImGuiCol_Separator]=ImVec4(0.34f,0.33f,0.44f,0.50f);
c[ImGuiCol_Tab]=ImVec4(0.17f,0.17f,0.24f,1.0f);c[ImGuiCol_TabHovered]=ImVec4(0.80f,0.65f,0.96f,0.45f);
c[ImGuiCol_TabActive]=ImVec4(0.80f,0.65f,0.96f,1.0f);c[ImGuiCol_TabUnfocused]=ImVec4(0.17f,0.17f,0.24f,1.0f);
c[ImGuiCol_DockingPreview]=ImVec4(0.80f,0.65f,0.96f,0.50f);c[ImGuiCol_Text]=ImVec4(0.82f,0.84f,0.88f,1.0f);
c[ImGuiCol_TextDisabled]=ImVec4(0.82f,0.84f,0.88f,0.38f);c[ImGuiCol_ResizeGrip]=ImVec4(0.80f,0.65f,0.96f,0.25f);
c[ImGuiCol_NavHighlight]=ImVec4(0.80f,0.65f,0.96f,1.0f);c[ImGuiCol_ModalWindowDimBg]=ImVec4(0.00f,0.00f,0.00f,0.45f);
ApplyStyleTokens(s);
}
}
