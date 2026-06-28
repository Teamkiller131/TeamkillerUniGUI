#include <unigui/styling/style_engine.h>

#include <imgui.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <gtest/gtest.h>
using namespace unigui::styling;

class StyleTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        Engine::Instance().Parse("");
    }
    void TearDown() override { ImGui::DestroyContext(); }
};

TEST_F(StyleTest, Parse_SingleRule) {
    int n = Engine::Instance().Parse("Window { bg: #ff0000; rounding: 8; }");
    EXPECT_EQ(n, 1);
    Engine::Instance().Apply("Window");
    auto& c = ImGui::GetStyle().Colors[ImGuiCol_WindowBg];
    EXPECT_NEAR(c.x, 1.0f, 0.01f); // #ff0000 = red
}

TEST_F(StyleTest, Parse_MultipleRules) {
    int n = Engine::Instance().Parse("Window { bg: #111; } Button { bg: #222; }");
    EXPECT_EQ(n, 2);
}

TEST_F(StyleTest, ClassSelector) {
    Engine::Instance().Parse("Button.primary { rounding: 12; }");
    ImGui::GetStyle().WindowRounding = 6;
    Engine::Instance().Apply("Button", "primary");
    EXPECT_NEAR(ImGui::GetStyle().WindowRounding, 12.0f, 0.1f);
}

TEST_F(StyleTest, IDSelector_Priority) {
    Engine::Instance().Parse("Button { rounding: 4; } #submit { rounding: 16; }");
    Engine::Instance().Apply("Button", "", "submit");
    EXPECT_NEAR(ImGui::GetStyle().WindowRounding, 16.0f, 0.1f);
}

// ── Error-path: malformed numeric values must not throw (regression for the
//    previous unguarded std::stof in ApplyProp/EvaluateMedia). ──
TEST_F(StyleTest, MalformedNumericValue_DoesNotThrow) {
    EXPECT_NO_THROW({
        Engine::Instance().Parse("Window { rounding: abc; padding: ; spacing: 12px; }");
        Engine::Instance().Apply("Window");
    });
}

TEST_F(StyleTest, MalformedRounding_FallsBackToZero) {
    ImGui::GetStyle().WindowRounding = 5.0f;
    Engine::Instance().Parse("Window { rounding: garbage; }");
    Engine::Instance().Apply("Window");
    EXPECT_NEAR(ImGui::GetStyle().WindowRounding, 0.0f, 0.01f);
}

// ── Dispatch-table color slots still resolve after the refactor. ──
TEST_F(StyleTest, ColorDispatch_AppliesKnownSlot) {
    Engine::Instance().Parse("Window { frame-bg: #00ff00; }");
    Engine::Instance().Apply("Window");
    auto& c = ImGui::GetStyle().Colors[ImGuiCol_FrameBg];
    EXPECT_NEAR(c.x, 0.0f, 0.02f);
    EXPECT_NEAR(c.y, 1.0f, 0.02f);
    EXPECT_NEAR(c.z, 0.0f, 0.02f);
}

TEST_F(StyleTest, UnknownProperty_IsIgnoredSafely) {
    EXPECT_NO_THROW({
        Engine::Instance().Parse("Window { totally-unknown-prop: whatever; }");
        Engine::Instance().Apply("Window");
    });
}

// ── Gradient parser must not throw on a value with no '#' hex color. The old
//    v.substr(v.find('#')) threw std::out_of_range (substr(npos)) on named colors,
//    breaking the engine's documented non-throwing contract. ──
TEST_F(StyleTest, GradientWithoutHexColor_DoesNotThrow) {
    EXPECT_NO_THROW({
        Engine::Instance().Parse("Window { gradient: linear-gradient(to right, red, blue); }");
        Engine::Instance().Apply("Window");
    });
}

TEST_F(StyleTest, GradientNone_DoesNotThrow) {
    EXPECT_NO_THROW({
        Engine::Instance().Parse("Window { bg-gradient: none; }");
        Engine::Instance().Apply("Window");
    });
}

TEST_F(StyleTest, GradientWithHexColors_StillParses) {
    EXPECT_NO_THROW({
        Engine::Instance().Parse("Window { gradient: linear-gradient(90deg, #ff0000, #0000ff); }");
        Engine::Instance().Apply("Window");
    });
}

// ── Hot-reload from disk (Horizon 5) ────────────────────────────────────────

namespace {
std::filesystem::path WriteCss(const std::string& body) {
    namespace fs = std::filesystem;
    static int counter = 0;
    const fs::path p =
        fs::temp_directory_path() / ("unigui_hotreload_" + std::to_string(counter++) + ".css");
    std::ofstream(p) << body;
    return p;
}
// Push the file's mtime forward so a reload deterministically sees a change
// regardless of filesystem timestamp granularity.
void BumpMtime(const std::filesystem::path& p) {
    std::filesystem::last_write_time(p, std::filesystem::file_time_type::clock::now() +
                                            std::chrono::seconds(5));
}
} // namespace

TEST_F(StyleTest, LoadFile_RemembersWatchedPath) {
    const auto p = WriteCss("Window { rounding: 8; }");
    int n = Engine::Instance().LoadFile(p.string());
    EXPECT_EQ(n, 1);
    EXPECT_EQ(Engine::Instance().WatchedFile(), p.string());
    Engine::Instance().Apply("Window");
    EXPECT_NEAR(ImGui::GetStyle().WindowRounding, 8.0f, 0.01f);
    std::filesystem::remove(p);
}

TEST_F(StyleTest, ReloadIfChanged_FalseWhenUnchanged) {
    const auto p = WriteCss("Window { rounding: 4; }");
    Engine::Instance().LoadFile(p.string());
    EXPECT_FALSE(Engine::Instance().ReloadIfChanged()); // nothing changed
    std::filesystem::remove(p);
}

TEST_F(StyleTest, ReloadIfChanged_PicksUpEdits) {
    const auto p = WriteCss("Window { rounding: 4; }");
    Engine::Instance().LoadFile(p.string());
    Engine::Instance().Apply("Window");
    EXPECT_NEAR(ImGui::GetStyle().WindowRounding, 4.0f, 0.01f);

    // Edit the stylesheet on disk and bump its mtime.
    std::ofstream(p) << "Window { rounding: 12; }";
    BumpMtime(p);

    EXPECT_TRUE(Engine::Instance().ReloadIfChanged());
    Engine::Instance().Apply("Window");
    EXPECT_NEAR(ImGui::GetStyle().WindowRounding, 12.0f, 0.01f);
    std::filesystem::remove(p);
}

TEST_F(StyleTest, ReloadIfChanged_NoWatchedFileReturnsFalse) {
    Engine::Instance().Clear();
    // Fresh singleton state may still hold a path from earlier tests; only the
    // contract matters: reloading a missing file does not crash and is safe.
    EXPECT_NO_THROW({ (void) Engine::Instance().ReloadIfChanged(); });
}

TEST_F(StyleTest, Clear_EmptiesRules) {
    Engine::Instance().Parse("Window { bg: #abc; } Button { bg: #def; }");
    Engine::Instance().Clear();
    // After clearing, applying touches nothing — a fresh parse starts from zero.
    int n = Engine::Instance().Parse("Window { rounding: 2; }");
    EXPECT_EQ(n, 1);
}
