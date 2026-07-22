// UniGUI on the Web — Widget Gallery (WebAssembly + WebGL2 / WebGPU).
//
// A single-window, tabbed showcase of the UniGUI widget library running in the browser:
// retained-mode widgets, the immediate `unigui::im` helpers, runtime theme switching,
// and v3 effects (Card / Shimmer / Badge). It builds to a .html via Emscripten and hands
// the frame loop to the browser through emscripten_set_main_loop (inside unigui::Run).
// Open the generated web_demo.html through a local web server (WebGL/WebGPU need
// http(s), not file://).
#include <unigui/unigui.h>

#include <format>
#include <memory>

namespace im = unigui::im;

static void DrawGallery() {
    namespace im = unigui::im;

    im::SetNextWindowPos(ImVec2(30, 30), ImGuiCond_FirstUseEver);
    im::SetNextWindowSize(ImVec2(920, 660), ImGuiCond_FirstUseEver);
    // WindowScope always balances End on both the early-return and normal path.
    unigui::WindowScope window{"UniGUI on the Web — Widget Gallery"};
    if (!window.visible())
        return;

    im::TextWrapped("UniGUI v" UNIGUI_VERSION_STRING
                    " compiled to WebAssembly — the same C++23 widget library, theme "
                    "engine, and immediate-mode helpers as the desktop, rendered through "
                    "WebGL2 (GLES3) or WebGPU.");
    im::Spacing();

    if (im::BeginTabBar("gallery", ImGuiTabBarFlags_FittingPolicyScroll)) {
        if (im::BeginTabItem("Buttons")) {
            static unigui::Button b1("b1", "Default");
            static unigui::Button b2("b2", "Primary");
            b2.SetColorVariant(unigui::Button::Primary);
            static unigui::Button b3("b3", "Danger");
            b3.SetColorVariant(unigui::Button::Danger);
            static unigui::IconButton ib("ib", "★", "Star");
            b1.Render();
            im::SameLine();
            b2.Render();
            im::SameLine();
            b3.Render();
            im::SameLine();
            ib.Render();
            im::Separator();
            if (im::Button("im::Primary", im::ButtonVariant::Primary)) {}
            im::SameLine();
            if (im::Button("im::Success", im::ButtonVariant::Success)) {}
            im::SameLine();
            if (im::Button("im::Warning", im::ButtonVariant::Warning)) {}
            im::EndTabItem();
        }

        if (im::BeginTabItem("Inputs")) {
            static unigui::InputInt ii("ii", "Count", 0, 0, 100);
            static unigui::InputFloat iif("iif", "Price", 0, 0, 1000);
            static unigui::LineEdit le("le", "Email");
            le.SetPlaceholder("user@example.com");
            static unigui::SpinBox<float> sb("sb", "Volume", 50, 0, 100, 1);
            static unigui::PasswordInput pi("pi", "Password");
            pi.SetShowStrength(true);
            ii.Render();
            iif.Render();
            le.Render();
            sb.Render();
            pi.Render();
            im::Text(std::format(" Strength: {}/4", pi.GetStrengthScore()));
            im::EndTabItem();
        }

        if (im::BeginTabItem("Toggles")) {
            static unigui::CheckBox cb("cb", "Enable");
            static unigui::ToggleSwitch ts("ts", "Dark Mode", true);
            static unigui::RadioGroup rg("rg", {"A", "B", "C"}, 1);
            cb.Render();
            ts.Render();
            rg.Render();
            im::EndTabItem();
        }

        if (im::BeginTabItem("Selection")) {
            static unigui::ComboBox cmb("cmb", "Fruit", {"Apple", "Banana", "Cherry"}, 0);
            static unigui::ListView lv("lv", {"Item 1", "Item 2", "Item 3", "Item 4"});
            cmb.Render();
            lv.Render();
            im::EndTabItem();
        }

        if (im::BeginTabItem("Progress")) {
            static unigui::ProgressBar pb("pb", 0.65f);
            pb.SetOverlayText("65%");
            static unigui::LoadingIndicator li("li", 16);
            static unigui::Slider<float> sl("sl", "Opacity", 0.5f, 0, 1);
            pb.Render();
            li.Render();
            sl.Render();
            im::EndTabItem();
        }

        if (im::BeginTabItem("Layout")) {
            static unigui::Separator sep("sep", "Section");
            static unigui::GroupBox gb("gb", "Settings");
            static unigui::ScrollArea sa("sa", 0, 100);
            static unigui::Splitter sp("sp", unigui::Splitter::Horizontal, 0.5f);
            sep.Render();
            gb.SetContentCallback([] { im::Text("Group content"); });
            gb.Render();
            sa.SetContentCallback([] {
                for (int i = 0; i < 6; i++)
                    im::Text(std::format("Scrollable line {}", i));
            });
            sa.Render();
            sp.SetContentA([] { im::Text("Left"); });
            sp.SetContentB([] { im::Text("Right"); });
            sp.Render();
            im::EndTabItem();
        }

        if (im::BeginTabItem("Navigation")) {
            static unigui::TabWidget tabs("tabs");
            static unigui::Breadcrumb bc("bc");
            static unigui::TreeView tv("tv");
            static bool tabInit = false;
            if (!tabInit) {
                tabs.AddTab({"t1", "Tab 1", [] { im::Text("Content 1"); }});
                tabs.AddTab({"t2", "Tab 2", [] { im::Text("Content 2"); }});
                bc.SetItems({"Home", "Settings", "Profile"});
                unigui::TreeNode root{"Root", {{"Child 1", {}}, {"Child 2", {{"Grandchild", {}}}}}};
                tv.SetRoot(root);
                tabInit = true;
            }
            tabs.Render();
            bc.Render();
            tv.Render();
            im::EndTabItem();
        }

        if (im::BeginTabItem("Data")) {
            static unigui::Table tbl("tbl", {"Name", "Age", "City"});
            static bool init = false;
            if (!init) {
                tbl.AddRow({"Alice", "30", "NYC"});
                tbl.AddRow({"Bob", "25", "LA"});
                tbl.AddRow({"Carol", "28", "SF"});
                tbl.SetSortable(true);
                init = true;
            }
            tbl.Render();
            im::EndTabItem();
        }

        if (im::BeginTabItem("File & Color")) {
            static unigui::FilePath fp("fp", "File");
            fp.SetFilter("*.cpp;*.h");
            static unigui::DirPath dp("dp", "Folder");
            static unigui::ColorPicker cp("cp", "Color");
            fp.Render();
            dp.Render();
            cp.Render();
            im::TextDisabled("(native file dialogs are desktop-only; no-op on the web)");
            im::EndTabItem();
        }

        if (im::BeginTabItem("Info")) {
            static unigui::Label lbl("lbl", "A static label");
            static unigui::MultiLine ml("ml", "Line 1\nLine 2\nLine 3");
            static unigui::Hyperlink hl("hl", "GitHub", "https://github.com");
            static unigui::Tag tag("tag", "v" UNIGUI_VERSION_STRING);
            static unigui::DatePicker dpo("dpo", "Date");
            lbl.Render();
            ml.Render();
            hl.Render();
            tag.Render();
            dpo.Render();
            im::EndTabItem();
        }

        if (im::BeginTabItem("Cards & Effects")) {
            static unigui::Card card("Elevated Card");
            card.SetContent([] {
                im::TextWrapped("A card with drop shadow and rounded corners.");
                static int clicks = 0;
                if (im::Button("Click Me"))
                    clicks++;
                im::SameLine();
                im::Text(std::format("Clicks: {}", clicks));
            });
            card.SetFooter([] { im::TextDisabled("Card footer"); });
            card.Render();

            im::Spacing();
            static unigui::Badge dot("");
            dot.SetVariant(unigui::Badge::Dot);
            static unigui::Badge cnt("");
            cnt.SetCount(7);
            cnt.SetColor(IM_COL32(233, 69, 96, 255));
            static unigui::Badge nbl("NEW");
            nbl.SetColor(IM_COL32(0, 180, 100, 255));
            im::Text("Badges:");
            im::SameLine();
            dot.Render();
            im::SameLine(120);
            cnt.Render();
            im::SameLine(180);
            nbl.Render();

            im::Spacing();
            static unigui::Shimmer sh;
            static bool shInit = true;
            if (shInit) {
                sh.AddBlock(300, 16, 0, 0);
                sh.AddBlock(250, 16, 0, 22);
                sh.AddCircle(28, 0, 50);
                sh.AddBlock(360, 36, 40, 50);
                sh.Start();
                shInit = false;
            }
            if (im::Button(sh.IsPlaying() ? "Stop shimmer" : "Start shimmer"))
                sh.IsPlaying() ? sh.Stop() : sh.Start();
            im::Dummy(0, 4);
            sh.Render();
            im::EndTabItem();
        }

        if (im::BeginTabItem("Themes")) {
            im::TextWrapped("Switch the active theme at runtime — UniGUI's theme engine ships "
                            "13 presets and restyles every widget live.");
            im::Spacing();
            static int selected = 0;
            auto names = unigui::theme::ThemeRegistry::Instance().List();
            if (!names.empty() &&
                im::BeginCombo("Theme", names[selected % (int) names.size()].c_str())) {
                for (int i = 0; i < (int) names.size(); i++) {
                    bool isSel = (selected == i);
                    if (im::Selectable(names[i].c_str(), isSel)) {
                        selected = i;
                        unigui::theme::ThemeRegistry::Instance().Apply(names[i]);
                    }
                }
                im::EndCombo();
            }
            im::EndTabItem();
        }

        if (im::BeginTabItem("Accessibility")) {
            namespace a11y = unigui::a11y;
            im::TextWrapped("UniGUI exposes a semantic accessibility layer over Dear ImGui: a "
                            "per-frame element tree, focus tracking, and ARIA-style live "
                            "announcements that a screen-reader bridge can consume. Keyboard "
                            "navigation (Tab / arrows) is on by default.");
            im::Spacing();

            bool enabled = a11y::IsEnabled();
            if (im::Checkbox("Enable accessibility", &enabled))
                a11y::SetEnabled(enabled);
            im::SameLine();
            if (im::SmallButton("Log to console"))
                a11y::InstallLoggingBridge(); // reference bridge → browser console

            if (im::Button("Announce (polite)"))
                a11y::Announce("Polite announcement from UniGUI", a11y::Live::Polite);
            im::SameLine();
            if (im::Button("Announce (assertive)"))
                a11y::Announce("Assertive announcement!", a11y::Live::Assertive);

            im::TextDisabled("Tab through the Widgets tab, then watch the inspector below.");
            static bool show_inspector = true;
            im::Checkbox("Show the accessibility inspector window", &show_inspector);
            if (show_inspector)
                a11y::DrawInspector(&show_inspector);
            im::EndTabItem();
        }

        if (im::BeginTabItem("About")) {
            im::TextWrapped("This gallery is the examples/web_demo target, built with "
                            "`emcmake cmake -DUNIGUI_WEB_WEBGPU=ON/OFF` and "
                            "`cmake --build … --target web_demo`.");
            im::SeparatorText("Fonts");
            im::TextWrapped("The web build embeds only the Latin JetBrains Mono Nerd Font and "
                            "has no system fonts, so CJK / emoji show as missing glyphs — load "
                            "a CJK font via the font manager to render them (the desktop build "
                            "merges system CJK ranges automatically).");
            im::SeparatorText("Dear ImGui interop");
            static bool show_demo = false;
            im::Checkbox("Show the stock Dear ImGui demo window", &show_demo);
            if (show_demo)
                ImGui::ShowDemoWindow(&show_demo);
            im::EndTabItem();
        }

        im::EndTabBar();
    }
}

int main() {
    unigui::AppConfig config;
    config.width = 1280;
    config.height = 720;
    config.title = "UniGUI Web Demo — Widget Gallery";
#if defined(__EMSCRIPTEN__) && defined(UNIGUI_HAS_WEBGPU)
    // GLFW (NO_API) canvas + WebGPU renderer (built with -DUNIGUI_WEB_WEBGPU=ON).
    config.backend = unigui::BackendType::WebGPU;
#elif defined(__EMSCRIPTEN__)
    // emscripten platform (emscripten's GLFW) + OpenGL3/WebGL2 renderer.
    config.backend = unigui::BackendType::Emscripten;
#endif

    // On the web Run() never returns — the browser owns the loop (emscripten_set_main_loop),
    // so maxFrames is ignored and there is no post-loop Shutdown.
    return unigui::RunApp(config, [] { DrawGallery(); }, /*maxFrames=*/0);
}
