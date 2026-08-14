// Multi-monitor runtime proof (real displays, not simulated geometry).
//
// The DPI & visual-proof phase closed the single-monitor fractional-DPI story;
// its remaining tail was "per-monitor scale inheritance across monitors needs a
// multi-monitor runner". This machine has several, so the app-level paths are
// now pinned end to end:
//   1. the wrapper's unigui::GetMonitors() agrees with what the GLFW backend
//      reported to ImGui (count, rects, per-monitor DpiScale);
//   2. popping a window OUT to a SECOND monitor lands it there, its viewport
//      inherits that monitor's DpiScale, and the main window keeps rendering;
//   3. the inheritance mechanism is deterministic: overriding a monitor's
//      table scale and popping into it must move viewport->DpiScale, and
//      restoring the scale must move it back (the exact code path a real
//      150% monitor exercises).
//
// Self-gated: skips when DX11 can't bring up or fewer than two monitors exist
// (single-monitor CI runners still exercise the monitor-table test).
#if defined(_WIN32) && defined(UNIGUI_HAS_DX11)
#define IMGUI_DEFINE_MATH_OPERATORS // ImVec2/ImRect arithmetic for the rect math below
#include <unigui/app/app.h>
#include <unigui/backend/dx11_renderer.h>
#include <unigui/im/im.h>

#include <GLFW/glfw3.h> // glfwGetWindowPos on viewport platform handles
#include <d3d11.h>      // DXGI_SWAP_CHAIN_DESC for the physical-size assertion
#include <imgui.h>
#include <imgui_internal.h> // ImRect/ImVec2 operators + ImGuiViewportP::PlatformMonitor
#include <windows.h>        // SetProcessDpiAwarenessContext

#include <cstdlib>
#include <gtest/gtest.h>
#include <string>
#include <vector>

namespace {

struct AppGuard {
    unigui::DX11Renderer* renderer = nullptr;
    bool up = false;

    bool BringUp(bool multiViewport) {
        SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
        _putenv_s("UNIGUI_RENDER_VERIFY", "1");
        unigui::AppConfig cfg;
        cfg.width = 480;
        cfg.height = 360;
        cfg.backend = unigui::BackendType::DX11;
        cfg.multiViewport = multiViewport;
        cfg.theme.font_size = 13.0f;
        if (!unigui::Init(cfg))
            return false;
        if (std::string(ImGui::GetIO().BackendRendererName) != "imgui_impl_dx11") {
            unigui::Shutdown();
            return false;
        }
        ImGui::GetIO().IniFilename = nullptr; // never persist window state into ctest's CWD
        renderer = static_cast<unigui::DX11Renderer*>(unigui::GetActiveRenderer());
        up = true;
        return true;
    }
    ~AppGuard() {
        if (up)
            unigui::Shutdown();
    }

    void Frame(const ImVec2* homePos = nullptr, const ImVec2* panelPos = nullptr,
               bool pinBoth = false) {
        ASSERT_TRUE(unigui::NewFrame());
        Draw(homePos, panelPos, pinBoth);
        unigui::Render();
    }

    static void Draw(const ImVec2* homePos, const ImVec2* panelPos, bool pinBoth) {
        // Two windows so the main viewport has something to draw after a pop-out.
        if (pinBoth)
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        if (homePos)
            ImGui::SetNextWindowPos(*homePos, ImGuiCond_Always);
        ImGui::SetNextWindowSize({80, 40}, ImGuiCond_Always);
        if (unigui::im::Begin("home")) {
            unigui::im::Text("home");
        }
        unigui::im::End();
        if (pinBoth)
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        if (panelPos)
            ImGui::SetNextWindowPos(*panelPos, ImGuiCond_Always);
        if (unigui::im::Begin("panel")) {
            unigui::im::Text("panel content");
        }
        unigui::im::End();
    }

    static ImGuiViewport* FindSecondary() {
        for (ImGuiViewport* vp : ImGui::GetPlatformIO().Viewports)
            if (vp != ImGui::GetMainViewport() && !(vp->Flags & ImGuiViewportFlags_IsMinimized))
                return vp;
        return nullptr;
    }

    static int SecondaryCount() {
        int n = 0;
        for (ImGuiViewport* vp : ImGui::GetPlatformIO().Viewports)
            if (vp != ImGui::GetMainViewport() && !(vp->Flags & ImGuiViewportFlags_IsMinimized))
                ++n;
        return n;
    }
};

} // namespace

TEST(DXMultiMonitorSmoke, Monitors_AppApiMatchesImGuiPlatformTable) {
    AppGuard app;
    if (!app.BringUp(false))
        GTEST_SKIP() << "app bring-up failed (headless runner?)";

    app.Frame(); // ImGui_ImplGlfw_NewFrame refreshes the monitor table

    const auto appMonitors = unigui::GetMonitors();
    const auto& table = ImGui::GetPlatformIO().Monitors;
    ASSERT_FALSE(appMonitors.empty()) << "a desktop session must report at least one monitor";
    ASSERT_EQ(appMonitors.size(), table.Size)
        << "the wrapper's monitor list must match what the backend gave ImGui";

    for (std::size_t i = 0; i < appMonitors.size(); ++i) {
        SCOPED_TRACE(i);
        const auto& m = appMonitors[i];
        const auto& t = table[i];
        EXPECT_NEAR(static_cast<float>(m.x), t.MainPos.x, 1.0f);
        EXPECT_NEAR(static_cast<float>(m.y), t.MainPos.y, 1.0f);
        EXPECT_NEAR(static_cast<float>(m.width), t.MainSize.x, 1.0f);
        EXPECT_NEAR(static_cast<float>(m.height), t.MainSize.y, 1.0f);
        EXPECT_NEAR(m.dpiScale, t.DpiScale, 0.01f) << "per-monitor DpiScale must agree";
    }

    // The main window's render projection must match the DPI of the monitor it
    // is on (DisplayFramebufferScale — applied at bring-up from the window's
    // real content scale, and re-asserted every frame against the GLFW
    // backend's framebuffer-ratio overwrite). FontScaleDpi stays 1.0 at
    // bring-up by design (the atlas is baked at dpi×font-size); it only moves
    // on runtime changes.
    const int mainMon = static_cast<ImGuiViewportP*>(ImGui::GetMainViewport())->PlatformMonitor;
    EXPECT_NEAR(ImGui::GetIO().DisplayFramebufferScale.x, table[mainMon].DpiScale, 0.01f)
        << "the framebuffer scale must inherit the main window's monitor scale";

    // The DX11 swapchain must be sized at PHYSICAL pixels (client × scale):
    // the OS scales the 480×360 client to 720×540 on a 150% monitor, and a
    // client-sized swapchain would render stretched/blurry.
    auto* dxr = static_cast<unigui::DX11Renderer*>(unigui::GetActiveRenderer());
    ASSERT_NE(dxr, nullptr);
    ASSERT_NE(dxr->swapchain_, nullptr);
    DXGI_SWAP_CHAIN_DESC sd{};
    ASSERT_TRUE(SUCCEEDED(dxr->swapchain_->GetDesc(&sd)));
    EXPECT_EQ(sd.BufferDesc.Width, static_cast<UINT>(480 * table[mainMon].DpiScale + 0.5f))
        << "the swapchain width must be the client width times the monitor scale";
}

TEST(DXMultiMonitorSmoke, PoppedOutWindow_LandsOnSecondMonitor_InheritsMonitorScale) {
    AppGuard app;
    if (!app.BringUp(true))
        GTEST_SKIP() << "app bring-up failed (headless runner?)";
    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
        GTEST_SKIP() << "multi-viewport unsupported by this backend pair";
    }

    app.Frame(); // monitors table + settled main viewport position
    const auto& table = ImGui::GetPlatformIO().Monitors;
    if (table.Size < 2)
        GTEST_SKIP() << "fewer than two monitors — cross-monitor pop-out is untestable";

    // Pick a monitor that does NOT contain the main window (the pop must cross
    // a monitor boundary or it stays in the main viewport).
    const ImVec2 mainCenter = ImGui::GetMainViewport()->GetCenter();
    int target = -1;
    for (int i = 0; i < table.Size; ++i) {
        const ImRect r(table[i].MainPos, table[i].MainPos + table[i].MainSize);
        if (!r.Contains(mainCenter)) {
            target = i;
            break;
        }
    }
    if (target < 0)
        GTEST_SKIP() << "the main window spans every monitor — cannot pick a distinct target";

    const ImVec2 mainPos = ImGui::GetMainViewport()->Pos;
    const ImVec2 homePos(mainPos.x + 40.0f, mainPos.y + 40.0f);
    const ImVec2 targetCenter = table[target].MainPos + table[target].MainSize * 0.5f;

    const ImVec2 panelPos = homePos + ImVec2(0, 150.0f);
    app.Frame(&homePos, &panelPos, true); // settle both inside
    EXPECT_EQ(app.SecondaryCount(), 0);

    // Pop the panel into the target monitor's center (containment rule, the same
    // path a mouse drag across monitors takes).
    app.Frame(&homePos, &targetCenter, false);
    app.Frame(&homePos, &targetCenter, false);

    ImGuiViewport* vp = app.FindSecondary();
    ASSERT_NE(vp, nullptr) << "the panel must get its own OS viewport on the target monitor";
    const int vpMon = static_cast<ImGuiViewportP*>(vp)->PlatformMonitor;
    ASSERT_GE(vpMon, 0) << "the viewport must be assigned a monitor";
    ASSERT_LT(vpMon, table.Size);
    EXPECT_EQ(vpMon, target) << "the viewport must land on the TARGET monitor";
    EXPECT_NEAR(vp->DpiScale, table[target].DpiScale, 0.01f)
        << "the viewport must inherit its monitor's DpiScale";

    // The platform window must physically sit inside the target monitor's rect.
    if (GLFWwindow* win = static_cast<GLFWwindow*>(vp->PlatformHandle)) {
        int x = 0, y = 0;
        glfwGetWindowPos(win, &x, &y);
        const ImRect r(table[target].MainPos, table[target].MainPos + table[target].MainSize);
        EXPECT_TRUE(r.Contains(ImVec2(static_cast<float>(x), static_cast<float>(y))))
            << "the secondary OS window must be positioned inside the target monitor";
    }

    EXPECT_EQ(app.renderer->LastVerifyDrawn(), 1)
        << "the main window must keep rendering after the cross-monitor pop-out";
}

TEST(DXMultiMonitorSmoke, SimulatedFractionalDpi_SecondaryViewportInheritsMonitorScale) {
    AppGuard app;
    if (!app.BringUp(true))
        GTEST_SKIP() << "app bring-up failed (headless runner?)";
    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
        GTEST_SKIP() << "multi-viewport unsupported by this backend pair";

    app.Frame(); // monitors table + settled main viewport position
    auto& table = ImGui::GetPlatformIO().Monitors;
    if (table.Size < 2)
        GTEST_SKIP() << "fewer than two monitors — the simulated-DPI path is untestable";

    // Pick a monitor that does not contain the main window (as above).
    const ImVec2 mainCenter = ImGui::GetMainViewport()->GetCenter();
    int target = -1;
    for (int i = 0; i < table.Size; ++i) {
        const ImRect r(table[i].MainPos, table[i].MainPos + table[i].MainSize);
        if (!r.Contains(mainCenter)) {
            target = i;
            break;
        }
    }
    if (target < 0)
        GTEST_SKIP() << "the main window spans every monitor — cannot pick a distinct target";

    // RAII: always restore the real monitor scale, even on ASSERT failure.
    const float realScale = table[target].DpiScale;
    struct ScaleRestore {
        ImGuiPlatformMonitor* m;
        float scale;
        ~ScaleRestore() { m->DpiScale = scale; }
    } restore{&table[target], realScale};

    table[target].DpiScale = 1.5f; // simulate a 150% monitor

    const ImVec2 mainPos = ImGui::GetMainViewport()->Pos;
    const ImVec2 homePos(mainPos.x + 40.0f, mainPos.y + 40.0f);
    const ImVec2 targetCenter = table[target].MainPos + table[target].MainSize * 0.5f;

    const ImVec2 panelPos = homePos + ImVec2(0, 150.0f);
    app.Frame(&homePos, &panelPos, true);
    app.Frame(&homePos, &targetCenter, false);
    app.Frame(&homePos, &targetCenter, false);

    ImGuiViewport* vp = app.FindSecondary();
    ASSERT_NE(vp, nullptr);
    const int vpMon = static_cast<ImGuiViewportP*>(vp)->PlatformMonitor;
    ASSERT_EQ(vpMon, target);
    EXPECT_NEAR(vp->DpiScale, 1.5f, 0.01f)
        << "the viewport must inherit the simulated 150% monitor scale";

    // Restoring the monitor scale must flow back into the viewport on the next
    // platform-window update — the same path a move between differently-scaled
    // monitors takes.
    restore.m->DpiScale = realScale;
    app.Frame(&homePos, &targetCenter, false);
    EXPECT_NEAR(vp->DpiScale, realScale, 0.01f)
        << "restoring the monitor scale must update the viewport's inherited scale";
}
#endif // _WIN32 && UNIGUI_HAS_DX11
