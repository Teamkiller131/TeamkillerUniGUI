// App-level multi-viewport smoke on a real DX11 swapchain (hardware or the GLFW
// fallback). This is the runtime proof for the 2026-08 client-suite work: a window
// popped out into its own OS viewport must not break the MAIN window's rendering —
// the exact regression the per-frame RTV rebind fixed, plus the backdrop-clear
// contract for secondary viewports (§7 docs/BACKENDS.md).
//
// UNIGUI_RENDER_VERIFY=1 turns on the DX11 renderer's back-buffer readback; the
// renderer's LastVerifyDrawn() then reports per frame whether real pixels landed on
// the main surface. Self-gated: skips when DX11 can't bring up (headless CI falls
// back to GL, where this test is a no-op).
//
// The scene has TWO windows: "home" (always stays in the main window — its pixels are
// what the readback asserts on) and "panel" (popped out and back). Without the home
// window the main viewport's draw data is legitimately empty after the pop-out, and
// "main went blank" could not be distinguished from "nothing to draw".
//
// Notes on the viewport machinery this test documents (docking branch):
//   - The main viewport's Pos reads (0,0) until the platform reports the real window
//     position, so a default-positioned window can pop out on its very first frame;
//     both windows are pinned to the main viewport (SetNextWindowViewport) until
//     settled, and coordinates are only computed after that.
//   - SetNextWindow* must be called AFTER NewFrame (NewFrame clears that state).
//   - Pop-out is containment-based: a non-owned window whose rect leaves the main
//     viewport gets its own OS viewport (the same rule the mouse-drag path uses) —
//     this test drives it with SetNextWindowPos rather than a synthetic cursor drag,
//     which is deterministic and exercises the same machinery.
//   - Orphaned secondary viewports are destroyed only after ~3 inactive frames; the
//     app-loop backdrop fill deliberately skips them (filling their background draw
//     list would count as activity and leak the viewport forever).

#if defined(_WIN32) && defined(UNIGUI_HAS_DX11)
#include <unigui/app/app.h>
#include <unigui/backend/dx11_renderer.h>
#include <unigui/im/im.h>

#include <imgui.h>

#include <windows.h> // SetProcessDpiAwarenessContext

#include <cstdlib>
#include <gtest/gtest.h>
#include <string>

TEST(DXMultiViewportSmoke, PoppedOutWindow_MainViewportStillDrawn) {
    // Per-monitor DPI awareness: without it Windows virtualizes coordinates for this
    // process and window/viewport coordinates disagree with the physical screen.
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    _putenv_s("UNIGUI_RENDER_VERIFY", "1");

    unigui::AppConfig cfg;
    cfg.width = 480;
    cfg.height = 360;
    cfg.backend = unigui::BackendType::DX11;
    cfg.multiViewport = true;
    // Fixed 1.0 DPI: a fractional monitor scale mixes physical (window/OS) and logical
    // (client) coordinate spaces, which would corrupt the pop-out coordinate math.
    cfg.theme.dpi_scale = 1.0f;
    if (!unigui::Init(cfg))
        GTEST_SKIP() << "app bring-up failed (headless runner?)";
    if (std::string(ImGui::GetIO().BackendRendererName) != "imgui_impl_dx11") {
        unigui::Shutdown();
        GTEST_SKIP() << "DX11 unavailable; the GL fallback took over — nothing to verify here";
    }
    if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable)) {
        unigui::Shutdown();
        GTEST_SKIP() << "multi-viewport unsupported by this backend pair";
    }

    auto* dxr = static_cast<unigui::DX11Renderer*>(unigui::GetActiveRenderer());
    ASSERT_NE(dxr, nullptr);
    // This test pops windows around and runs under ctest's working directory — never
    // let the app persist window/table state to imgui.ini there: later tests create
    // their own contexts in the same directory and would inherit the popped-out
    // layout (observed: a DataTable a11y test's dimensions broke after this ran).
    ImGui::GetIO().IniFilename = nullptr;

    auto draw = [&](bool pinBoth, const ImVec2* homePos = nullptr, const ImVec2* panelPos = nullptr) {
        if (pinBoth)
            ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID);
        if (homePos)
            ImGui::SetNextWindowPos(*homePos, ImGuiCond_Always);
        // Fixed small size: auto-size is one-shot, and a window that protrudes past the
        // main viewport's edge pops itself out the moment it is no longer pinned.
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
    };
    auto frame = [&](bool pinBoth = false, const ImVec2* homePos = nullptr,
                     const ImVec2* panelPos = nullptr) {
        ASSERT_TRUE(unigui::NewFrame());
        draw(pinBoth, homePos, panelPos);
        unigui::Render();
    };
    auto countSecondaries = [] {
        int n = 0;
        for (ImGuiViewport* vp : ImGui::GetPlatformIO().Viewports)
            if (vp != ImGui::GetMainViewport() && !(vp->Flags & ImGuiViewportFlags_IsMinimized))
                ++n;
        return n;
    };

    // F0 — settle with both windows pinned to the main viewport: the platform reports
    // the real window position (it reads (0,0) before that).
    frame(true);
    const ImVec2 mainPos = ImGui::GetMainViewport()->Pos;
    const ImVec2 mainSize = ImGui::GetMainViewport()->Size;
    // Distinct, non-overlapping spots inside the settled main viewport (the pin keeps
    // them there even if the math is a few pixels off).
    const ImVec2 homePos(mainPos.x + 40.0f, mainPos.y + 40.0f);
    const ImVec2 panelPos(mainPos.x + 40.0f, mainPos.y + 200.0f);
    const ImVec2 outside(mainPos.x + mainSize.x + 120.0f, mainPos.y + 60.0f);

    // F1 — steady state: one viewport, both windows inside, the main window renders.
    frame(true, &homePos, &panelPos);
    EXPECT_EQ(countSecondaries(), 0);
    EXPECT_EQ(dxr->LastVerifyDrawn(), 1) << "main window must be drawn with one viewport";

    // F2 — pop the panel out by moving it fully outside the main viewport (the same
    // containment rule the mouse-drag path uses). The pop-out lands in the same
    // frame's UpdatePlatformWindows (runs inside Render()).
    frame(false, nullptr, &outside);
    frame();
    EXPECT_EQ(countSecondaries(), 1) << "the popped-out window must get its own OS viewport";
    EXPECT_EQ(dxr->LastVerifyDrawn(), 1)
        << "main window went blank after the pop-out (per-frame RTV rebind regression)";

    // F3/F4 — stability with the window popped out: not a one-frame fluke.
    for (int i = 0; i < 2; ++i) {
        frame();
        EXPECT_EQ(dxr->LastVerifyDrawn(), 1) << "main window blank on stability frame " << i;
        EXPECT_EQ(countSecondaries(), 1) << "secondary viewport must persist on frame " << i;
    }

    // F5 — merge back through the public API (SetNextWindowViewport + position): the
    // panel returns to the main viewport and the orphaned secondary viewport is
    // destroyed — the programmatic path apps use to re-dock a floating panel.
    {
        ASSERT_TRUE(unigui::NewFrame());
        if (unigui::im::Begin("home")) {
            unigui::im::Text("home");
        }
        unigui::im::End();
        ImGui::SetNextWindowViewport(ImGui::GetMainViewport()->ID); // applies to the PANEL
        ImGui::SetNextWindowPos(panelPos, ImGuiCond_Always);
        if (unigui::im::Begin("panel")) {
            unigui::im::Text("panel content");
        }
        unigui::im::End();
        unigui::Render();
    }
    // The orphaned viewport is destroyed only after ~3 frames of inactivity (imgui:
    // LastFrameActive < FrameCount - 2), so let it settle first.
    for (int k = 0; k < 3; ++k)
        frame();
    EXPECT_EQ(countSecondaries(), 0) << "re-pinning the window must destroy its OS viewport";
    EXPECT_EQ(dxr->LastVerifyDrawn(), 1) << "main window must stay drawn after merge-back";

    // F6 — no flap: the merged window must stay merged.
    frame();
    EXPECT_EQ(countSecondaries(), 0) << "merged window must not pop back out";
    EXPECT_EQ(dxr->LastVerifyDrawn(), 1) << "main window must stay drawn after the merge settles";

    unigui::Shutdown();
}
#endif // _WIN32 && UNIGUI_HAS_DX11
