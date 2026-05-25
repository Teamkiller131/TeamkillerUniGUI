#include <unigui/theme/theme.h>
#include <sstream>
#include <regex>

namespace unigui {

void ApplyTheme(const ThemeConfig& config) {
    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    // ── Rounding & Spacing (shared) ────────────────────────────────────────
    style.WindowRounding = 6.0f;
    style.FrameRounding = 4.0f;
    style.GrabRounding = 4.0f;
    style.TabRounding = 4.0f;
    style.ScrollbarRounding = 9.0f;
    style.FramePadding = ImVec2(8.0f, 6.0f);
    style.ItemSpacing = ImVec2(8.0f, 6.0f);
    style.ItemInnerSpacing = ImVec2(6.0f, 6.0f);
    style.WindowPadding = ImVec2(12.0f, 12.0f);
    style.ScrollbarSize = 14.0f;
    style.WindowMenuButtonPosition = ImGuiDir_None;

    // ── Color palette ─────────────────────────────────────────────────────
    if (config.preset == ThemePreset::Dark) {
        colors[ImGuiCol_Text]                  = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
        colors[ImGuiCol_Border]                = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.35f, 0.35f, 0.40f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.50f, 0.68f, 1.00f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.30f, 0.30f, 0.35f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_DockingPreview]        = ImVec4(0.40f, 0.58f, 0.93f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.14f, 0.14f, 0.16f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.24f, 0.24f, 0.28f, 1.00f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.18f, 0.18f, 0.22f, 1.00f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.10f, 0.10f, 0.12f, 1.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.12f, 0.12f, 0.14f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.40f, 0.58f, 0.93f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.40f, 0.58f, 0.93f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.90f, 0.90f, 0.92f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.50f, 0.50f, 0.55f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.50f);
    } else {
        // Light theme (white/gray backgrounds, dark text, blue accent)
        colors[ImGuiCol_Text]                  = ImVec4(0.15f, 0.15f, 0.18f, 1.00f);
        colors[ImGuiCol_TextDisabled]          = ImVec4(0.55f, 0.55f, 0.60f, 1.00f);
        colors[ImGuiCol_WindowBg]              = ImVec4(0.96f, 0.96f, 0.97f, 1.00f);
        colors[ImGuiCol_ChildBg]               = ImVec4(0.94f, 0.94f, 0.95f, 1.00f);
        colors[ImGuiCol_PopupBg]               = ImVec4(0.98f, 0.98f, 0.99f, 1.00f);
        colors[ImGuiCol_Border]                = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
        colors[ImGuiCol_BorderShadow]          = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg]               = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]        = ImVec4(0.82f, 0.82f, 0.85f, 1.00f);
        colors[ImGuiCol_FrameBgActive]         = ImVec4(0.78f, 0.78f, 0.82f, 1.00f);
        colors[ImGuiCol_TitleBg]               = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        colors[ImGuiCol_TitleBgActive]         = ImVec4(0.82f, 0.82f, 0.85f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed]      = ImVec4(0.90f, 0.90f, 0.92f, 1.00f);
        colors[ImGuiCol_MenuBarBg]             = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
        colors[ImGuiCol_ScrollbarBg]           = ImVec4(0.94f, 0.94f, 0.95f, 1.00f);
        colors[ImGuiCol_ScrollbarGrab]         = ImVec4(0.70f, 0.70f, 0.73f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabHovered]  = ImVec4(0.60f, 0.60f, 0.64f, 1.00f);
        colors[ImGuiCol_ScrollbarGrabActive]   = ImVec4(0.50f, 0.50f, 0.55f, 1.00f);
        colors[ImGuiCol_CheckMark]             = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_SliderGrab]            = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_SliderGrabActive]      = ImVec4(0.10f, 0.35f, 0.75f, 1.00f);
        colors[ImGuiCol_Button]                = ImVec4(0.78f, 0.78f, 0.82f, 1.00f);
        colors[ImGuiCol_ButtonHovered]         = ImVec4(0.70f, 0.70f, 0.75f, 1.00f);
        colors[ImGuiCol_ButtonActive]          = ImVec4(0.65f, 0.65f, 0.70f, 1.00f);
        colors[ImGuiCol_Header]                = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
        colors[ImGuiCol_HeaderHovered]         = ImVec4(0.78f, 0.78f, 0.82f, 1.00f);
        colors[ImGuiCol_HeaderActive]          = ImVec4(0.72f, 0.72f, 0.76f, 1.00f);
        colors[ImGuiCol_Separator]             = ImVec4(0.80f, 0.80f, 0.83f, 1.00f);
        colors[ImGuiCol_SeparatorHovered]      = ImVec4(0.60f, 0.60f, 0.64f, 1.00f);
        colors[ImGuiCol_SeparatorActive]       = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_ResizeGrip]            = ImVec4(0.78f, 0.78f, 0.82f, 1.00f);
        colors[ImGuiCol_ResizeGripHovered]     = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_ResizeGripActive]      = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_Tab]                   = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
        colors[ImGuiCol_TabHovered]            = ImVec4(0.70f, 0.70f, 0.75f, 1.00f);
        colors[ImGuiCol_TabActive]             = ImVec4(0.82f, 0.82f, 0.85f, 1.00f);
        colors[ImGuiCol_TabUnfocused]          = ImVec4(0.88f, 0.88f, 0.90f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]    = ImVec4(0.82f, 0.82f, 0.85f, 1.00f);
        colors[ImGuiCol_DockingPreview]        = ImVec4(0.15f, 0.40f, 0.80f, 0.70f);
        colors[ImGuiCol_DockingEmptyBg]        = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
        colors[ImGuiCol_PlotLines]             = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered]      = ImVec4(0.90f, 0.30f, 0.20f, 1.00f);
        colors[ImGuiCol_PlotHistogram]         = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered]  = ImVec4(0.90f, 0.30f, 0.20f, 1.00f);
        colors[ImGuiCol_TableHeaderBg]         = ImVec4(0.85f, 0.85f, 0.88f, 1.00f);
        colors[ImGuiCol_TableBorderStrong]     = ImVec4(0.70f, 0.70f, 0.73f, 1.00f);
        colors[ImGuiCol_TableBorderLight]      = ImVec4(0.82f, 0.82f, 0.85f, 1.00f);
        colors[ImGuiCol_TableRowBg]            = ImVec4(0.96f, 0.96f, 0.97f, 1.00f);
        colors[ImGuiCol_TableRowBgAlt]         = ImVec4(0.92f, 0.92f, 0.94f, 1.00f);
        colors[ImGuiCol_TextSelectedBg]        = ImVec4(0.15f, 0.40f, 0.80f, 0.35f);
        colors[ImGuiCol_DragDropTarget]        = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_NavHighlight]          = ImVec4(0.15f, 0.40f, 0.80f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(0.15f, 0.15f, 0.18f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg]     = ImVec4(0.50f, 0.50f, 0.55f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg]      = ImVec4(0.00f, 0.00f, 0.00f, 0.35f);
    }

    // ── DPI Scaling ───────────────────────────────────────────────────────
    ImGui::GetIO().FontGlobalScale = config.dpi_scale;
}

static const char* kColorNames[] = {
    "Text","TextDisabled","WindowBg","ChildBg","PopupBg","Border","BorderShadow",
    "FrameBg","FrameBgHovered","FrameBgActive","TitleBg","TitleBgActive","TitleBgCollapsed",
    "MenuBarBg","ScrollbarBg","ScrollbarGrab","ScrollbarGrabHovered","ScrollbarGrabActive",
    "CheckMark","SliderGrab","SliderGrabActive","Button","ButtonHovered","ButtonActive",
    "Header","HeaderHovered","HeaderActive","Separator","SeparatorHovered","SeparatorActive",
    "ResizeGrip","ResizeGripHovered","ResizeGripActive","Tab","TabHovered","TabActive",
    "TabUnfocused","TabUnfocusedActive","DockingPreview","DockingEmptyBg",
    "PlotLines","PlotLinesHovered","PlotHistogram","PlotHistogramHovered",
    "TableHeaderBg","TableBorderStrong","TableBorderLight","TableRowBg","TableRowBgAlt",
    "TextLink","TreeLines","TextSelectedBg","DragDropTarget","DragDropTargetBg",
    "UnsavedMarker","NavCursor","NavWindowingHighlight","NavWindowingDimBg","ModalWindowDimBg"
};

std::string ExportThemeJSON() {
    auto& colors = ImGui::GetStyle().Colors;
    constexpr int N = sizeof(kColorNames) / sizeof(kColorNames[0]);
    std::ostringstream ss;
    ss << "{";
    for (int i = 0; i < ImGuiCol_COUNT && i < N; i++) {
        if (i > 0) ss << ",";
        ss << "\"" << kColorNames[i] << "\":[" << colors[i].x << "," << colors[i].y << "," << colors[i].z << "," << colors[i].w << "]";
    }
    ss << "}";
    return ss.str();
}

bool ImportThemeJSON(const std::string& json) {
    auto& colors = ImGui::GetStyle().Colors;
    constexpr int N = sizeof(kColorNames) / sizeof(kColorNames[0]);
    for (int i = 0; i < ImGuiCol_COUNT && i < N; i++) {
        std::string name = kColorNames[i];
        std::regex re("\"" + name + "\"\\s*:\\s*\\[([^\\]]+)\\]");
        std::smatch m;
        if (std::regex_search(json, m, re)) {
            std::string arr = m[1].str();
            float vals[4] = {0,0,0,1};
            int vi = 0;
            size_t pos = 0;
            while (pos < arr.size() && vi < 4) {
                size_t comma = arr.find(',', pos);
                std::string num = (comma != std::string::npos) ? arr.substr(pos, comma-pos) : arr.substr(pos);
                try { vals[vi++] = std::stof(num); } catch(...) { break; }
                pos = (comma != std::string::npos) ? comma + 1 : arr.size();
            }
            colors[i] = ImVec4(vals[0], vals[1], vals[2], vals[3]);
        }
    }
    return true;
}

} // namespace unigui
