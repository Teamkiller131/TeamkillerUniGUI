#include <unigui/theme/theme.h>
#include <unigui/core/log.h>
#include <sstream>
#include <regex>
#include <cstdio>
#include <cerrno>
#ifdef _WIN32
#include <windows.h>
#endif

namespace unigui {

float DetectDPIScale(void* native_window) {
#ifdef _WIN32
    HWND hwnd = (HWND)native_window;
    if (hwnd) {
        // Try per-monitor DPI (Windows 10+)
        using GetDpiForWindow_t = UINT(WINAPI*)(HWND);
        HMODULE user32 = GetModuleHandleA("user32.dll");
        auto pGetDpiForWindow = (GetDpiForWindow_t)GetProcAddress(user32, "GetDpiForWindow");
        if (pGetDpiForWindow) {
            UINT dpi = pGetDpiForWindow(hwnd);
            if (dpi > 0) return dpi / 96.0f;
        }
    }
    // Fallback: system DPI
    HDC screen = GetDC(nullptr);
    if (screen) {
        int dpi = GetDeviceCaps(screen, LOGPIXELSX);
        ReleaseDC(nullptr, screen);
        if (dpi > 0) return dpi / 96.0f;
    }
#endif
    return 1.0f;
}

void LoadDefaultFont(float size_pixels, const char* ttf_path) {
    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig cfg;
    cfg.OversampleH = 2;
    cfg.OversampleV = 2;

    // Try loading via memory (bypasses ImFileOpen issues)
    auto tryLoad = [&](const char* path) -> ImFont* {
        FILE* fp = fopen(path, "rb");
        if (!fp) { UNIGUI_LOG_DEBUG("Font fopen failed: {} errno={}", path, errno); return nullptr; }
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (sz <= 0) { fclose(fp); return nullptr; }
        void* data = IM_ALLOC(sz);
        fread(data, 1, sz, fp);
        fclose(fp);
        ImFont* f = io.Fonts->AddFontFromMemoryTTF(data, (int)sz, size_pixels, &cfg);
        if (!f) { IM_FREE(data); UNIGUI_LOG_DEBUG("Font AddFontFromMemoryTTF failed: {}", path); }
        return f;
    };

    bool loaded = false;
    const char* base_paths[] = {
        "C:/Windows/Fonts/segoeui.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        nullptr
    };

    // Try custom path first if provided
    if (ttf_path && tryLoad(ttf_path)) loaded = true;

    for (int i = 0; base_paths[i] && !loaded; i++) {
        ImFont* f = tryLoad(base_paths[i]);
        if (f) {
            UNIGUI_LOG_INFO("Font: {} ({}px)", base_paths[i], (int)size_pixels);
            loaded = true;
        }
    }

    // Fallback: built-in font + FontGlobalScale to match target size
    if (!loaded) {
        io.Fonts->AddFontDefault(&cfg);
        io.FontGlobalScale = size_pixels / 13.0f;
        UNIGUI_LOG_WARN("Built-in font scaled to {}px (scale={:.2f})",
            (int)size_pixels, io.FontGlobalScale);
        return;
    }

    UNIGUI_LOG_INFO("Base font loaded at {}px", (int)size_pixels);

    // Merge CJK glyphs
    cfg.MergeMode = true;
    const ImWchar* cjk = io.Fonts->GetGlyphRangesChineseFull();
    const char* cjk_paths[] = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/simhei.ttf",
        nullptr
    };
    for (int i = 0; cjk_paths[i]; i++) {
        if (io.Fonts->AddFontFromFileTTF(cjk_paths[i], size_pixels, &cfg, cjk)) {
            UNIGUI_LOG_INFO("CJK font merged: {}", cjk_paths[i]);
            break;
        }
    }
}

void BeginTextWrap(float width) {
    if (width <= 0) width = ImGui::GetContentRegionAvail().x;
    ImGui::PushTextWrapPos(ImGui::GetCursorPosX() + width);
}
void EndTextWrap() {
    ImGui::PopTextWrapPos();
}

void ApplyTheme(const ThemeConfig& config) {
    auto& io = ImGui::GetIO();
    auto& style = ImGui::GetStyle();
    auto& colors = style.Colors;

    // ── DPI scaling ──────────────────────────────────────────────────────
    float dpi = config.dpi_scale;
    if (dpi <= 0) dpi = 1.0f; // caller should have set dpi before calling
    UNIGUI_LOG_INFO("Theme: preset={} dpi={:.2f} font={}px",
        (int)config.preset, dpi, (int)(config.font_size * dpi));
    // ── Style base values ─────────────────────────────────────────────
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

    // ── Color palette ────────────────────────────────────────────────────
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

    // Scale all sizes by DPI (AFTER setting values so they get scaled)
    style.ScaleAllSizes(dpi);
}

static const char* kColorNames[] = {"Text","TextDisabled","WindowBg","ChildBg","PopupBg","Border","BorderShadow","FrameBg","FrameBgHovered","FrameBgActive","TitleBg","TitleBgActive","TitleBgCollapsed","MenuBarBg","ScrollbarBg","ScrollbarGrab","ScrollbarGrabHovered","ScrollbarGrabActive","CheckMark","SliderGrab","SliderGrabActive","Button","ButtonHovered","ButtonActive","Header","HeaderHovered","HeaderActive","Separator","SeparatorHovered","SeparatorActive","ResizeGrip","ResizeGripHovered","ResizeGripActive","Tab","TabHovered","TabActive","TabUnfocused","TabUnfocusedActive","DockingPreview","DockingEmptyBg","PlotLines","PlotLinesHovered","PlotHistogram","PlotHistogramHovered","TableHeaderBg","TableBorderStrong","TableBorderLight","TableRowBg","TableRowBgAlt","TextLink","TreeLines","TextSelectedBg","DragDropTarget","DragDropTargetBg","UnsavedMarker","NavCursor","NavWindowingHighlight","NavWindowingDimBg","ModalWindowDimBg"};

std::string ExportThemeJSON() {
    auto& colors = ImGui::GetStyle().Colors;
    constexpr int N = sizeof(kColorNames)/sizeof(kColorNames[0]);
    std::ostringstream ss; ss << "{";
    for (int i = 0; i < ImGuiCol_COUNT && i < N; i++) {
        if (i > 0) ss << ",";
        ss << "\"" << kColorNames[i] << "\":[" << colors[i].x << "," << colors[i].y << "," << colors[i].z << "," << colors[i].w << "]";
    }
    ss << "}"; return ss.str();
}

bool ImportThemeJSON(const std::string& json) {
    auto& colors = ImGui::GetStyle().Colors;
    constexpr int N = sizeof(kColorNames)/sizeof(kColorNames[0]);
    for (int i = 0; i < ImGuiCol_COUNT && i < N; i++) {
        std::regex re("\"" + std::string(kColorNames[i]) + "\"\\s*:\\s*\\[([^\\]]+)\\]");
        std::smatch m;
        if (std::regex_search(json, m, re)) {
            float vals[4]={0,0,0,1}; int vi=0; size_t pos=0;
            std::string arr = m[1].str();
            while (pos<arr.size()&&vi<4) {
                size_t comma=arr.find(',',pos);
                try{vals[vi++]=std::stof(comma!=std::string::npos?arr.substr(pos,comma-pos):arr.substr(pos));}catch(...){break;}
                pos=comma!=std::string::npos?comma+1:arr.size();
            }
            colors[i]=ImVec4(vals[0],vals[1],vals[2],vals[3]);
        }
    }
    return true;
}

} // namespace unigui
