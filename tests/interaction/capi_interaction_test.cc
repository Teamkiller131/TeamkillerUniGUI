// Interaction tests for the C ABI's immediate-mode drawing subset.
//
// The engine clicks and types against widgets drawn through the C boundary, so
// a real C host's per-frame callback path (unigui_begin/button/checkbox/...)
// is proven to behave identically to the C++ im layer — not just compile.
// Compiled only when UNIGUI_TEST_ENGINE=ON.
#include <unigui/capi/unigui_capi.h>

#include "interaction_harness.h"

class CapiInteractionTest : public itest::InteractionFixture {};

TEST_F(CapiInteractionTest, Button_ClickThroughCAPI_ReturnsTrue) {
    int clicked = 0;
    const auto st = Run(
        "capi_button",
        [&] {
            if (unigui_begin("##capiw", nullptr)) {
                if (unigui_button("Save"))
                    clicked = 1;
            }
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("//**/Save");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(clicked, 1) << "a click through the C ABI must reach the im layer and back";
}

TEST_F(CapiInteractionTest, Checkbox_TogglesValueThroughCAPI) {
    int value = 0;
    const auto st = Run(
        "capi_checkbox",
        [&] {
            if (unigui_begin("##capiw", nullptr)) {
                unigui_checkbox("Enable", &value);
            }
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("//**/Enable");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(value, 1) << "the int* write-back across the ABI must toggle to 1";
}

TEST_F(CapiInteractionTest, SliderFloat_ClickMovesValueThroughCAPI) {
    float value = 0.0f;
    const auto st = Run(
        "capi_slider",
        [&] {
            if (unigui_begin("##capiw", nullptr)) {
                unigui_slider_float("gain", &value, 0.0f, 10.0f);
            }
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("//**/gain");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_GT(value, 0.0f) << "clicking the slider track must move the float* through the ABI";
    EXPECT_LE(value, 10.0f);
}

TEST_F(CapiInteractionTest, Begin_OpenFlagWriteBack_NeverResurrected) {
    // *p_open is a close hook, not a visibility switch: in this ImGui
    // generation Begin() still returns 1 (the window is visible) after the
    // flag goes 0 — the caller stops drawing on subsequent frames, exactly
    // like the C++ contract. What the ABI must guarantee is the exact write-
    // back: once 0, the flag never comes back 1.
    int frames = 0;
    int open = 1;
    int drawn = 0;
    const auto st = Run(
        "capi_begin_open",
        [&] {
            ++frames;
            if (frames == 2)
                open = 0; // caller "closes" the window through the int*
            if (unigui_begin("##capiw", &open))
                ++drawn;
            unigui_text("frame %d", frames);
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            for (int i = 0; i < 3; ++i)
                ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_GE(frames, 4);
    EXPECT_EQ(drawn, frames) << "a visible window's Begin returns 1 every frame";
    EXPECT_EQ(open, 0) << "the ABI must never resurrect a closed window's flag";
}

TEST_F(CapiInteractionTest, Text_FormattedAndUnformatted_RenderWithoutError) {
    int frames = 0;
    const auto st = Run(
        "capi_text",
        [&] {
            ++frames;
            if (unigui_begin("##capiw", nullptr)) {
                unigui_text_unformatted("plain");
                unigui_text("count %d pct %.0f%%", 7, 42.0);
                unigui_separator();
            }
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_GE(frames, 1);
}

// ── ABI v2: form & layout tranche ─────────────────────────────────────────────

TEST_F(CapiInteractionTest, RadioButton_ClickSelectsThroughCAPI) {
    int current = 0;
    const auto st = Run(
        "capi_radio",
        [&] {
            if (unigui_begin("##capiw", nullptr)) {
                unigui_radio_button("Alpha", &current, 0);
                unigui_radio_button("Beta", &current, 1);
            }
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("//**/Beta");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(current, 1) << "clicking the second radio must write the int* through the ABI";
}

TEST_F(CapiInteractionTest, Combo_SelectItemThroughCAPI) {
    int current = 0;
    const char* kItems[] = {"Alpha", "Beta", "Gamma"};
    const auto st = Run(
        "capi_combo", [&] { unigui_combo("city", &current, kItems, 3); },
        [&](ImGuiTestContext* ctx) {
            // ComboClick splits combo-vs-item at the first '/': it resolves the
            // BeginCombo by its label ("city") and then the popup item ("Beta").
            ctx->SetRef("TW");
            ctx->ComboClick("city/Beta");
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(current, 1) << "selecting a combo item must write the index through the ABI";
}

TEST_F(CapiInteractionTest, Selectable_ClickReturnsTrueThroughCAPI) {
    int clicked = 0;
    const auto st = Run(
        "capi_selectable",
        [&] {
            if (unigui_begin("##capiw", nullptr)) {
                if (unigui_selectable("Row", 0))
                    clicked = 1;
            }
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("//**/Row");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_EQ(clicked, 1) << "a selectable click must return 1 through the ABI";
}

TEST_F(CapiInteractionTest, InputText_TypingWritesIntoCallerBuffer) {
    char buf[64] = "";
    const auto st = Run(
        "capi_input_text",
        [&] {
            if (unigui_begin("##capiw", nullptr)) {
                unigui_input_text("name", buf, sizeof(buf));
            }
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->ItemClick("//**/name");
            ctx->Yield(); // the frame acquiring keyboard focus swallows one char — idle past it
            ctx->KeyChars("abc");
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_STREQ(buf, "abc") << "typed text must land in the CALLER-OWNED buffer via the ABI";
}

TEST_F(CapiInteractionTest, FormTranche_RenderSmoke_NoError) {
    int iv = 3;
    float fv = 0.5f;
    int frames = 0;
    const auto st = Run(
        "capi_form_tranche",
        [&] {
            ++frames;
            if (unigui_begin("##capiw", nullptr)) {
                unigui_input_int("count", &iv);
                unigui_same_line();
                unigui_input_float("gain", &fv);
                unigui_slider_int("level", &iv, 0, 10);
                unigui_progress_bar(0.4f);
                unigui_spacing();
                unigui_selectable("Hover me", 0);
                unigui_set_tooltip("tip text");
            }
            unigui_end();
        },
        [&](ImGuiTestContext* ctx) {
            ctx->SetRef("TW");
            ctx->Yield();
            ctx->Yield();
        });
    EXPECT_EQ(st, ImGuiTestStatus_Success);
    EXPECT_GE(frames, 1);
}
