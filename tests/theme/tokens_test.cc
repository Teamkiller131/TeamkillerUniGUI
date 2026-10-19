#include <unigui/theme/tokens.h>

#include <imgui.h>

#include <gtest/gtest.h>

#include <cmath>
#include <cstring>
#include <iterator>

using namespace unigui::theme::tokens;

// ─────────────────────────────────────────────────────────────────────────────
// Compile-time contract: token values, scales and relationships. These run on
// every compiler invocation, so a re-skin that changes a token to a nonsensical
// value fails the build, not just a test run.
// ─────────────────────────────────────────────────────────────────────────────

// Version / palette identity (strcmp is not constexpr; check the literal chars).
static_assert(kTokenVersion[0] == 'v' && kTokenVersion[1] == '0' && kTokenVersion[2] == '\0',
              "v0 token set must be labelled v0");
static_assert(kIsDarkPalette, "v0 default skin is dark");
static_assert(kColorCount == 12, "v0 defines exactly 12 colour roles");

// Colour role values (v0 default skin; values may change per skin, but these are
// pinned so an accidental edit is caught). Structural checks follow.
static_assert(bg.x == 0.100f && bg.y == 0.100f && bg.z == 0.120f && bg.w == 1.0f);
static_assert(surface.w == 1.0f && surfaceAlt.w == 1.0f && text.w == 1.0f);
static_assert(textMuted.w == 1.0f && border.w == 1.0f && focus.w == 1.0f);
static_assert(accent.x == 0.400f && accent.y == 0.580f && accent.z == 0.930f);
static_assert(danger.x > danger.y && danger.x > danger.z, "danger reads as red");
static_assert(warning.x > warning.z && warning.y > warning.z, "warning reads as amber");
static_assert(success.y > success.x && success.y > success.z, "success reads as green");

// Named colour table is a constexpr map name → same object → usage text.
static_assert(kColors[0].value == &bg && kColors[1].value == &surface);
static_assert(kColors[2].value == &surfaceAlt && kColors[3].value == &text);
static_assert(kColors[4].value == &textMuted && kColors[5].value == &border);
static_assert(kColors[6].value == &accent && kColors[7].value == &accentHover);
static_assert(kColors[8].value == &danger && kColors[9].value == &warning);
static_assert(kColors[10].value == &success && kColors[11].value == &focus);
static_assert(kColors[6].name != nullptr && kColors[6].usage != nullptr);

// Spacing: 4 px base scale, strictly increasing.
static_assert(s4 == 4.0f && s8 == 8.0f && s12 == 12.0f && s16 == 16.0f && s24 == 24.0f);
static_assert(s4 * 2.0f == s8 && s4 * 3.0f == s12 && s4 * 6.0f == s24);
static_assert(s4 < s8 && s8 < s12 && s12 < s16 && s16 < s24);

// Radius: controls 4, cards/panels 6.
static_assert(radius4 == 4.0f && radius6 == 6.0f && radius4 < radius6);

// Typography: font sizes strictly increasing from hint → caption → body → title.
static_assert(hint12 == 12.0f && caption13 == 13.0f && body16 == 16.0f && title20 == 20.0f);
static_assert(hint12 < caption13 && caption13 < body16 && body16 < title20);
static_assert(body16 == 16.0f, "body token is 16 px by definition of the name");

// Controls: standard 32, primary-action bar 36.
static_assert(control32 == 32.0f && control36 == 36.0f && control32 < control36);
static_assert(control32 >= s24 + s4, "control must fit a 24 px icon + padding");

// Motion: ms-based tokens and the ImGui seconds conversion.
static_assert(duration120 == 120 && duration180 == 180);
static_assert(duration120 < duration180);
static_assert(SecondsFromMillis(duration120) == 120.0f / 1000.0f);
static_assert(SecondsFromMillis(duration180) == 180.0f / 1000.0f);

// constexpr ImU32 conversion matches ImGui's runtime conversion semantics.
static_assert(ColorToU32(bg) == IM_COL32(26, 26, 31, 255));
static_assert(ColorToU32(ImVec4(1.0f, 1.0f, 1.0f, 1.0f)) == IM_COL32(255, 255, 255, 255));
static_assert(ColorToU32(ImVec4(-1.0f, 2.0f, 0.5f, 1.0f)) == IM_COL32(0, 255, 128, 255));

// ─────────────────────────────────────────────────────────────────────────────
// Runtime checks: name mapping and the WCAG AA minimum-contrast contract from
// docs/design-tokens.md. These are what a re-skin is allowed to change values
// for — the structure below must stay.
// ─────────────────────────────────────────────────────────────────────────────

namespace {

// WCAG 2.x relative luminance / contrast ratio (sRGB).
float LinearChannel(float c) {
    return c <= 0.03928f ? c / 12.92f : std::pow((c + 0.055f) / 1.055f, 2.4f);
}
float RelativeLuminance(const ImVec4& c) {
    return 0.2126f * LinearChannel(c.x) + 0.7152f * LinearChannel(c.y) +
           0.0722f * LinearChannel(c.z);
}
float ContrastRatio(const ImVec4& a, const ImVec4& b) {
    const float la = RelativeLuminance(a);
    const float lb = RelativeLuminance(b);
    return la > lb ? (la + 0.05f) / (lb + 0.05f) : (lb + 0.05f) / (la + 0.05f);
}

// Expected token-name → variable mapping, in declaration order. The test pins
// both the names (API stability for skins/docs) and the values (mapping is not
// silently swapped).
constexpr const char* kExpectedNames[] = {
    "bg",    "surface",    "surfaceAlt", "text",   "textMuted", "border",
    "accent", "accentHover", "danger",   "warning", "success",  "focus",
};
constexpr ImVec4 kExpectedValues[] = {
    bg,     surface,    surfaceAlt, text,    textMuted, border,
    accent, accentHover, danger,    warning, success,   focus,
};

} // namespace

TEST(tokens, ColourNameMapping_IsCompleteAndStable) {
    struct Expected {
        const char* name;
        const ImVec4* value;
    };
    const Expected expected[] = {
        {"bg", &bg},           {"surface", &surface},
        {"surfaceAlt", &surfaceAlt}, {"text", &text},
        {"textMuted", &textMuted},   {"border", &border},
        {"accent", &accent},   {"accentHover", &accentHover},
        {"danger", &danger},   {"warning", &warning},
        {"success", &success}, {"focus", &focus},
    };
    ASSERT_EQ(kColorCount, static_cast<int>(std::size(expected)));
    for (int i = 0; i < kColorCount; ++i) {
        EXPECT_STREQ(kColors[i].name, expected[i].name) << "index " << i;
        EXPECT_EQ(kColors[i].value, expected[i].value) << "name " << expected[i].name;
        EXPECT_STREQ(kColors[i].name, kExpectedNames[i]) << "index " << i;
        ASSERT_NE(kColors[i].usage, nullptr) << "usage text missing for " << expected[i].name;
        EXPECT_NE(std::strlen(kColors[i].usage), 0u) << "usage text empty for " << expected[i].name;
        // Same object, so value equality is exact: no rounding drift between the
        // named table and the standalone token.
        EXPECT_EQ(kColors[i].value->x, kExpectedValues[i].x);
        EXPECT_EQ(kColors[i].value->y, kExpectedValues[i].y);
        EXPECT_EQ(kColors[i].value->z, kExpectedValues[i].z);
        EXPECT_EQ(kColors[i].value->w, kExpectedValues[i].w);
    }
}

TEST(tokens, ColourRoles_AreSolidAndUnique) {
    for (int i = 0; i < kColorCount; ++i) {
        EXPECT_EQ(kColors[i].value->w, 1.0f) << kColors[i].name << " must be opaque";
        for (int j = i + 1; j < kColorCount; ++j) {
            const ImVec4& a = *kColors[i].value;
            const ImVec4& b = *kColors[j].value;
            EXPECT_FALSE(a.x == b.x && a.y == b.y && a.z == b.z)
                << kColors[i].name << " and " << kColors[j].name << " are identical";
        }
    }
}

TEST(tokens, AccentHover_IsBrighterThanAccent) {
    EXPECT_GT(accentHover.x, accent.x);
    EXPECT_GT(accentHover.y, accent.y);
    EXPECT_GE(accentHover.z, accent.z);
    EXPECT_EQ(accentHover.w, accent.w);
}

TEST(tokens, MinimumContrast_BodyTextMeetsAA) {
    // 4.5:1 — body text on every background surface (docs/design-tokens.md).
    EXPECT_GE(ContrastRatio(text, bg), 4.5f);
    EXPECT_GE(ContrastRatio(text, surface), 4.5f);
    EXPECT_GE(ContrastRatio(text, surfaceAlt), 4.5f);
    EXPECT_GE(ContrastRatio(textMuted, bg), 4.5f);
    EXPECT_GE(ContrastRatio(textMuted, surface), 4.5f);
}

TEST(tokens, MinimumContrast_StatusAndFocusAreReadable) {
    // Status text is body text when shown as a line (AA 4.5:1 on bg).
    EXPECT_GE(ContrastRatio(danger, bg), 4.5f);
    EXPECT_GE(ContrastRatio(warning, bg), 4.5f);
    EXPECT_GE(ContrastRatio(success, bg), 4.5f);
    // ... and always at least the 3:1 UI-component minimum on raised surfaces.
    EXPECT_GE(ContrastRatio(danger, surface), 3.0f);
    EXPECT_GE(ContrastRatio(warning, surface), 3.0f);
    EXPECT_GE(ContrastRatio(success, surface), 3.0f);
}

TEST(tokens, MinimumContrast_ComponentsAndFocusRingMeetThreeToOne) {
    // WCAG 2.2 AA non-text contrast: interactive component boundaries and the
    // focus indicator need >= 3:1 against their adjacent surface.
    EXPECT_GE(ContrastRatio(accent, bg), 3.0f);
    EXPECT_GE(ContrastRatio(accent, surface), 3.0f);
    EXPECT_GE(ContrastRatio(focus, surface), 3.0f);
    EXPECT_GE(ContrastRatio(focus, bg), 3.0f);
}

TEST(tokens, MinimumContrast_PrimaryActionLabelOnAccent) {
    // v0 primary buttons use the accent fill with a near-black label (the `bg`
    // token): 5.7:1. A skin that changes `accent` must re-check this pair, or
    // switch the label to a colour with >= 4.5:1 against the new accent.
    EXPECT_GE(ContrastRatio(bg, accent), 4.5f);
}

TEST(tokens, BorderIsDecorativeOnly) {
    // Deliberately below the 3:1 non-text minimum: hairline separators are
    // decorative and must never be the only signal. Pinned so nobody "fixes"
    // the value without also updating the docs/test contract.
    EXPECT_LT(ContrastRatio(border, bg), 3.0f);
    EXPECT_GT(ContrastRatio(border, bg), 1.5f);
}

TEST(tokens, SecondsFromMillis_UsesMillisecondTokens) {
    EXPECT_FLOAT_EQ(SecondsFromMillis(duration120), 0.120f);
    EXPECT_FLOAT_EQ(SecondsFromMillis(duration180), 0.180f);
    EXPECT_FLOAT_EQ(SecondsFromMillis(0), 0.0f);
}

TEST(tokens, ColorToU32_MatchesImGuiConversion) {
    for (int i = 0; i < kColorCount; ++i) {
        EXPECT_EQ(ColorToU32(*kColors[i].value), ImGui::ColorConvertFloat4ToU32(*kColors[i].value))
            << kColors[i].name;
    }
}

TEST(tokens, TokenGroups_UseLogicalPixelsAndMilliseconds) {
    // The token groups are semantic and unit-consistent; these assertions are the
    // doc contract checked at runtime (wide ranges catch accidental unit swaps
    // like writing 16 instead of 0.016 for seconds, or px instead of ms).
    EXPECT_GE(title20, body16);
    EXPECT_GE(body16, caption13);
    EXPECT_GE(caption13, hint12);
    EXPECT_GE(control36, control32);
    EXPECT_GE(control32, 24.0f);   // must fit an icon row
    EXPECT_LE(control36, 64.0f);   // not a page-height token
    EXPECT_GE(s24, s16);
    EXPECT_GE(s16, s12);
    EXPECT_LE(s24, 48.0f);
    EXPECT_GE(duration120, 60);    // below this it is not perceivable
    EXPECT_LE(duration180, 500);   // above this it feels sluggish for micro UI
}
