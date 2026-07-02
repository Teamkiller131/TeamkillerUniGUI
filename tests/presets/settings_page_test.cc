#include <unigui/core/accessibility.h>
#include <unigui/presets/settings_page.h>

#include <imgui.h>

#include <gtest/gtest.h>
#include <string>

class SettingsPageTest : public ::testing::Test {
protected:
    void SetUp() override {
        ImGui::CreateContext();
        ImGui::GetIO().DisplaySize = ImVec2(800, 600);
        ImGui::GetIO().Fonts->Build();
        ImGui::NewFrame();
    }
    void TearDown() override {
        ImGui::Render();
        ImGui::DestroyContext();
    }
};

TEST_F(SettingsPageTest, Defaults_EmptySchema) {
    unigui::presets::SettingsPage page("settings");
    EXPECT_EQ(page.GetSectionCount(), 0);
    EXPECT_EQ(page.GetRowCount(), 0);
    EXPECT_EQ(page.GetActiveSection(), 0);
}

TEST_F(SettingsPageTest, RowsWithoutSection_GoIntoImplicitSection) {
    unigui::presets::SettingsPage page("settings");
    bool flag = false;
    page.AddToggle("Dark mode", [&] { return flag; }, [&](bool v) { flag = v; });
    EXPECT_EQ(page.GetSectionCount(), 1); // implicit "General"
    EXPECT_EQ(page.GetRowCount(), 1);
}

TEST_F(SettingsPageTest, FluentChaining_BuildsSchema) {
    unigui::presets::SettingsPage page("settings");
    bool b = true;
    int i = 5, c = 0;
    float f = 0.5f;
    std::string s = "host";
    // WithTooltip is a CRTP base helper — it must return SettingsPage& so the
    // preset-specific Add* chain can follow it.
    page.WithTooltip("app settings")
        .AddSection("General")
        .AddToggle(
            "Dark mode", [&] { return b; }, [&](bool v) { b = v; })
        .AddInt(
            "Font size", [&] { return i; }, [&](int v) { i = v; }, 8, 32)
        .AddFloat(
            "Opacity", [&] { return f; }, [&](float v) { f = v; }, 0.f, 1.f)
        .AddSection("Network")
        .AddCombo(
            "Protocol", {"http", "https"}, [&] { return c; }, [&](int v) { c = v; })
        .AddText(
            "Proxy", [&] { return s; }, [&](const std::string& v) { s = v; })
        .AddAction("Reset", [] {});
    EXPECT_EQ(page.GetSectionCount(), 2);
    EXPECT_EQ(page.GetRowCount(), 6);
}

TEST_F(SettingsPageTest, SetActiveSection_IgnoresOutOfRange) {
    unigui::presets::SettingsPage page("settings");
    page.AddSection("General").AddSection("Network");
    page.SetActiveSection(1);
    EXPECT_EQ(page.GetActiveSection(), 1);
    page.SetActiveSection(5); // out of range — ignored
    EXPECT_EQ(page.GetActiveSection(), 1);
    page.SetActiveSection(-1); // out of range — ignored
    EXPECT_EQ(page.GetActiveSection(), 1);
}

TEST_F(SettingsPageTest, Getters_ReadDuringRender_SettersUntouched) {
    unigui::presets::SettingsPage page("settings");
    int reads = 0, writes = 0;
    page.AddToggle(
        "Dark mode",
        [&] {
            ++reads;
            return true;
        },
        [&](bool) { ++writes; });
    page.Render();
    EXPECT_GT(reads, 0);  // the model is polled every frame
    EXPECT_EQ(writes, 0); // no control change reported -> setter never fired
}

// ── Accessibility ────────────────────────────────────────────────────────────
class SettingsPageA11yTest : public SettingsPageTest {
protected:
    void SetUp() override {
        SettingsPageTest::SetUp();
        unigui::a11y::SetEnabled(true);
        unigui::a11y::BeginFrame();
        unigui::a11y::DrainAnnouncements();
    }
    void TearDown() override {
        unigui::a11y::SetEnabled(false);
        SettingsPageTest::TearDown();
    }
};

TEST_F(SettingsPageA11yTest, SectionChange_IsAnnounced) {
    unigui::presets::SettingsPage page("settings");
    page.AddSection("General").AddSection("Appearance");
    page.SetActiveSection(1);
    auto msgs = unigui::a11y::DrainAnnouncements();
    ASSERT_EQ(msgs.size(), 1u);
    EXPECT_NE(msgs[0].message.find("Appearance"), std::string::npos);
    page.SetActiveSection(1); // no change -> no announcement
    EXPECT_TRUE(unigui::a11y::DrainAnnouncements().empty());
}

TEST_F(SettingsPageA11yTest, Render_RegistersRowsAndGroupInTree) {
    unigui::presets::SettingsPage page("settings");
    bool dark = true;
    page.AddToggle(
            "Dark mode", [&] { return dark; }, [&](bool v) { dark = v; })
        .AddAction("Reset", [] {});
    page.Render();
    bool sawToggle = false, sawButton = false, sawGroup = false;
    for (const auto& n : unigui::a11y::Tree()) {
        if (n.role == unigui::a11y::Role::Toggle && n.name == "Dark mode" && n.value == "on")
            sawToggle = true;
        if (n.role == unigui::a11y::Role::Button && n.name == "Reset")
            sawButton = true;
        if (n.role == unigui::a11y::Role::Group && n.name == "settings")
            sawGroup = true;
    }
    EXPECT_TRUE(sawToggle);
    EXPECT_TRUE(sawButton);
    EXPECT_TRUE(sawGroup);
}

TEST_F(SettingsPageTest, Render_FullSchema_DoesNotCrash) {
    unigui::presets::SettingsPage page("settings");
    bool b = false;
    int i = 10, c = 1;
    float f = 0.25f;
    std::string s = "value";
    page.AddSection("General")
        .AddToggle(
            "Toggle", [&] { return b; }, [&](bool v) { b = v; })
        .AddInt(
            "Int", [&] { return i; }, [&](int v) { i = v; }, 0, 100)
        .AddFloat(
            "Float", [&] { return f; }, [&](float v) { f = v; }, 0.f, 1.f)
        .AddSection("Advanced")
        .AddCombo(
            "Combo", {"a", "b", "c"}, [&] { return c; }, [&](int v) { c = v; })
        .AddText(
            "Text", [&] { return s; }, [&](const std::string& v) { s = v; })
        .AddAction("Action", [] {});
    page.Render(); // multi-section path: section list + rows pane
    page.Hide();
    page.Render(); // hidden -> early-out, still safe
}
