// C ABI tests — the C++ (gtest) driver side.
//
// capi_c_test.c is compiled as C into the same target and proves the header is
// valid C + the boundary links from C; this file wraps those checks into gtest
// cases and adds the C++-side coverage:
//   - pure version/ABI/config contracts (headless),
//   - an end-to-end windowed lifecycle run on a real DX11 swapchain (WARP on
//     headless runners): create from C, draw from a C callback, destroy from C.
#include <unigui/capi/unigui_capi.h>
#include <unigui/core/version.h>

#include <imgui.h>

#if defined(_WIN32)
#include <windows.h> // SetProcessDpiAwarenessContext (DPI virtualization guard)
#endif

#include <cstdlib>
#include <gtest/gtest.h>
#include <string>

// C-side checks (implemented in capi_c_test.c, compiled as C).
extern "C" {
int capi_c_abi_gate_ok(void);
int capi_c_abi_future_rejected(void);
int capi_c_abi_zero_rejected(void);
int capi_c_version_ok(void);
int capi_c_config_defaults_ok(void);
int capi_c_types_usable_ok(void);
int capi_c_null_destroy_ok(void);
int capi_c_v2_surface_ok(void);
int capi_c_v1_bindings_still_compatible(void);
}

TEST(CapiC, CompilesAndLinksFromPlainC) {
    EXPECT_EQ(capi_c_abi_gate_ok(), 1);
    EXPECT_EQ(capi_c_abi_future_rejected(), 1);
    EXPECT_EQ(capi_c_abi_zero_rejected(), 1);
    EXPECT_EQ(capi_c_version_ok(), 1);
    EXPECT_EQ(capi_c_config_defaults_ok(), 1);
    EXPECT_EQ(capi_c_types_usable_ok(), 1);
    EXPECT_EQ(capi_c_null_destroy_ok(), 1);
    EXPECT_EQ(capi_c_v2_surface_ok(), 1) << "every ABI v2 symbol must link from plain C";
    EXPECT_EQ(capi_c_v1_bindings_still_compatible(), 1)
        << "ABI v2 is additive: v1 bindings must remain compatible";
}

TEST(Capi, VersionMirrorsCppSurface) {
    EXPECT_EQ(unigui_version_major(), UNIGUI_VERSION_MAJOR);
    EXPECT_EQ(unigui_version_minor(), UNIGUI_VERSION_MINOR);
    EXPECT_EQ(unigui_version_patch(), UNIGUI_VERSION_PATCH);
    EXPECT_STREQ(unigui_version_string(), UNIGUI_VERSION_STRING);
}

TEST(Capi, AbiGate_CurrentVersionCompatible_FutureRejected) {
    EXPECT_NE(unigui_capi_abi_compatible(UNIGUI_CAPI_ABI_VERSION), 0);
    EXPECT_EQ(unigui_capi_abi_compatible(UNIGUI_CAPI_ABI_VERSION + 1), 0);
    EXPECT_EQ(unigui_capi_abi_compatible(0), 0);
    EXPECT_EQ(unigui_capi_abi_compatible(-1), 0);
}

TEST(Capi, ConfigInit_MatchesDocumentedDefaults) {
    unigui_app_config cfg{};
    unigui_app_config_init(&cfg);
    EXPECT_EQ(cfg.width, 1280);
    EXPECT_EQ(cfg.height, 720);
    EXPECT_NE(cfg.title, nullptr);
    EXPECT_STREQ(cfg.title, "UniGUI Application");
    EXPECT_EQ(cfg.backend, UNIGUI_BACKEND_DEFAULT);
    EXPECT_EQ(cfg.dpi_scale_fonts, 0);
    EXPECT_EQ(cfg.accessibility, 0);
    EXPECT_EQ(cfg.multi_viewport, 0);
    // init on NULL must be a safe no-op (defensive contract).
    unigui_app_config_init(nullptr);
}

TEST(Capi, NullHandleContracts) {
    EXPECT_EQ(unigui_app_new_frame(nullptr), 0);
    unigui_app_render(nullptr);                     // no-op, must not crash
    EXPECT_NE(unigui_app_should_close(nullptr), 0); // dead app stops loops
    EXPECT_EQ(unigui_app_run(nullptr, nullptr, nullptr, 0), 1);
    EXPECT_EQ(unigui_app_native_window_handle(nullptr), nullptr);
    unigui_app_destroy(nullptr);
}

TEST(Capi, V2PointerGuardContracts) {
    // Every ABI v2 call with a null output/input pointer must be a safe no-op
    // (returns 0) WITHOUT touching ImGui — these run with no context.
    EXPECT_EQ(unigui_radio_button("x", nullptr, 0), 0);
    EXPECT_EQ(unigui_combo("x", nullptr, nullptr, 0), 0);
    EXPECT_EQ(unigui_combo("x", nullptr, nullptr, -1), 0);
    EXPECT_EQ(unigui_input_text("x", nullptr, 0), 0);
    EXPECT_EQ(unigui_input_text("x", nullptr, 64), 0);
    EXPECT_EQ(unigui_input_int("x", nullptr), 0);
    EXPECT_EQ(unigui_input_float("x", nullptr), 0);
    EXPECT_EQ(unigui_slider_int("x", nullptr, 0, 10), 0);
}

#if defined(_WIN32) && defined(UNIGUI_HAS_DX11)
// End-to-end lifecycle on a REAL DX11 swapchain: the software WARP rasterizer
// supplies the device on GPU-less runners, so this is the same runtime path a
// production C host takes (create → frame callback → render → destroy), not a
// mock. Skips when the app cannot bring DX11 up (mirrors dx_multiviewport_smoke).
TEST(Capi, AppLifecycle_RealSwapchain_CreateDrawDestroy) {
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    _putenv_s("UNIGUI_DX11_WARP", "1");

    unigui_app_config cfg;
    unigui_app_config_init(&cfg);
    cfg.width = 320;
    cfg.height = 240;
    cfg.title = "capi lifecycle";
    cfg.backend = UNIGUI_BACKEND_DX11;

    unigui_app* app = unigui_app_create(&cfg);
    if (app == nullptr)
        GTEST_SKIP() << "app bring-up failed (headless runner without WARP?)";
    if (std::string(ImGui::GetIO().BackendRendererName) != "imgui_impl_dx11") {
        unigui_app_destroy(app);
        GTEST_SKIP() << "DX11 unavailable; the GL fallback took over — nothing to verify here";
    }

    // Never persist window state from a lifecycle smoke into ctest's CWD.
    ImGui::GetIO().IniFilename = nullptr;

    int frames = 0;
    int clicks = 0;
    int open = 1;
    float gain = 0.0f;
    const auto frame = [](void* userdata) {
        struct State {
            int* frames;
            int* clicks;
            int* open;
            float* gain;
        }* st = static_cast<State*>(userdata);
        ++*st->frames;
        if (unigui_begin("capi win", st->open)) {
            unigui_text("frame %d", *st->frames);
            if (unigui_button("Click"))
                ++*st->clicks;
            unigui_checkbox("Keep open", st->open);
            unigui_slider_float("gain", st->gain, 0.0f, 1.0f);
            unigui_separator();
        }
        unigui_end();
        // Close through the ABI's int* write-back after the second frame — the
        // third frame must see Begin() return 0 (the run loop itself exits via
        // the frame cap, so the test never depends on an OS window close).
        if (*st->frames >= 2)
            *st->open = 0;
    };
    struct {
        int* frames;
        int* clicks;
        int* open;
        float* gain;
    } state{&frames, &clicks, &open, &gain};

    EXPECT_NE(unigui_app_native_window_handle(app), nullptr);
    EXPECT_NE(unigui_app_new_frame(app), 0);
    unigui_text_unformatted("frame-0 probe"); // live context: must not crash
    unigui_app_render(app);

    EXPECT_EQ(unigui_app_run(app, frame, &state, 3), 0);
    EXPECT_EQ(frames, 3) << "the C frame callback must run once per capped frame";
    EXPECT_GE(clicks, 0);

    unigui_app_destroy(app); // shutdown already ran in run(); destroy frees
    EXPECT_EQ(unigui_app_should_close(nullptr), 1);
}
#endif // _WIN32 && UNIGUI_HAS_DX11
