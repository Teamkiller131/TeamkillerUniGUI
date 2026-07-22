// UniGUI accessibility demo.
//
// Sets AppConfig::accessibility = true, which enables the a11y layer (keyboard nav +
// per-frame tree) and installs the platform screen-reader bridge once the window is up
// (UI Automation on Windows, NSAccessibility on macOS, ARIA on the web). Run it with a
// screen reader active (Windows: Narrator = Win+Ctrl+Enter) and Tab through the widgets —
// each focused control is announced, and the two buttons fire live-region announcements.
// The on-screen inspector mirrors the focus + tree + announcements.
#include <unigui/unigui.h>

#include <cstdlib>
#include <string_view>

namespace im = unigui::im;

static void Draw() {
    namespace im = unigui::im;
    namespace a11y = unigui::a11y;

    im::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_FirstUseEver);
    im::SetNextWindowSize(ImVec2(440, 460), ImGuiCond_FirstUseEver);
    if (unigui::WindowScope window{"Accessibility demo"}; window.visible()) {
        im::TextWrapped("Tab / Shift-Tab through these widgets with a screen reader running. "
                        "Each focused control is announced; the buttons below post live-region "
                        "announcements.");
        im::Spacing();

        static unigui::Button save("save", "Save");
        save.WithPrimary().WithAccessibleDescription("Saves the current settings");
        save.Render();

        static unigui::CheckBox notify("notify", "Enable notifications");
        notify.Render();

        static unigui::LineEdit name("name", "Name");
        name.Render();

        static unigui::ComboBox fruit("fruit", "Fruit", {"Apple", "Banana", "Cherry"}, 0);
        fruit.Render();

        static unigui::RadioGroup color("color", {"Red", "Green", "Blue"}, 0);
        color.Render();

        im::Separator();
        if (im::Button("Announce status"))
            a11y::Announce("Settings saved", a11y::Live::Polite);
        im::SameLine();
        if (im::Button("Announce error", im::ButtonVariant::Danger))
            a11y::Announce("Validation failed", a11y::Live::Assertive);
    }

    a11y::DrawInspector();
}

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        std::string_view a = argv[i];
        if (a == "--frames" && i + 1 < argc)
            max_frames = std::atoi(argv[++i]);
    }

    unigui::AppConfig cfg;
    cfg.title = "UniGUI Accessibility Demo";
    cfg.accessibility = true; // enable a11y + install the platform screen-reader bridge
    return unigui::RunApp(cfg, [] { Draw(); }, max_frames);
}
