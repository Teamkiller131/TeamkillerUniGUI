#include <unigui/unigui.h>

#include <cstdio>
#include <cstdlib>
#include <format>
#include <string_view>

namespace im = unigui::im;

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++) {
        std::string_view a = argv[i];
        if (a == "--frames" && i + 1 < argc)
            max_frames = std::atoi(argv[++i]);
    }

    unigui::AppConfig cfg;
    cfg.width = 1280;
    cfg.height = 800;
    cfg.title = "UniGUI v3.0 — Widget Gallery (55+ widgets, 10 themes)";
    if (!unigui::Init(cfg))
        return 1;
    std::printf("[gallery] Initialized\n");

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

        static auto win = std::make_shared<unigui::Window>("main", "Widget Gallery");
        static bool first = true;
        if (first) {
            win->SetMenuBarEnabled(true);

            auto addGroup = [&](const char* title, std::function<void()> cb) {
                auto p = std::make_shared<unigui::Panel>(title, title);
                p->SetContentCallback(std::move(cb));
                win->AddPanel(p);
            };

            addGroup("Buttons", []() {
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
                ib.Render();
            });

            addGroup("Inputs", []() {
                static unigui::InputInt ii("ii", "Count", 0, 0, 100);
                static unigui::InputFloat iif("iif", "Price", 0, 0, 1000);
                static unigui::LineEdit le("le", "Email");
                le.SetPlaceholder("user@example.com");
                static unigui::SpinBox<float> sb("sb", "Volume", 50, 0, 100, 1);
                ii.Render();
                iif.Render();
                le.Render();
                sb.Render();
                static unigui::PasswordInput pi("pi", "Password");
                pi.SetShowStrength(true);
                pi.Render();
                im::Text(std::format(" Strength: {}/4", pi.GetStrengthScore()));
            });

            addGroup("Toggles", []() {
                static unigui::CheckBox cb("cb", "Enable");
                static unigui::ToggleSwitch ts("ts", "Dark Mode", true);
                static unigui::RadioGroup rg("rg", {"A", "B", "C"}, 1);
                cb.Render();
                ts.Render();
                rg.Render();
            });

            addGroup("Selection", []() {
                static unigui::ComboBox cmb("cmb", "Fruit", {"Apple", "Banana", "Cherry"}, 0);
                static unigui::ListView lv("lv", {"Item1", "Item2", "Item3", "Item4"});
                cmb.Render();
                lv.Render();
            });

            addGroup("Progress & Feedback", []() {
                static unigui::ProgressBar pb("pb", 0.65f);
                pb.SetOverlayText("65%");
                static unigui::LoadingIndicator li("li", 16);
                static unigui::Slider<float> sl("sl", "Opacity", 0.5f, 0, 1);
                pb.Render();
                li.Render();
                sl.Render();
            });

            addGroup("Layout", []() {
                static unigui::Separator sep("sep", "Section");
                static unigui::GroupBox gb("gb", "Settings");
                static unigui::ScrollArea sa("sa", 0, 100);
                static unigui::Splitter sp("sp", unigui::Splitter::Horizontal, 0.5f);
                sep.Render();
                gb.SetContentCallback([] { im::Text("Group content"); });
                gb.Render();
                sa.SetContentCallback([] {
                    for (int i = 0; i < 5; i++)
                        im::Text(std::format("Line {}", i));
                });
                sa.Render();
                sp.SetContentA([] { im::Text("Left"); });
                sp.SetContentB([] { im::Text("Right"); });
                sp.Render();
            });

            addGroup("Navigation", []() {
                static unigui::TabWidget tabs("tabs");
                static unigui::Breadcrumb bc("bc");
                static unigui::TreeView tv("tv");
                static bool tabInit = false;
                if (!tabInit) {
                    tabs.AddTab({"t1", "Tab 1", [] { im::Text("Content 1"); }});
                    tabs.AddTab({"t2", "Tab 2", [] { im::Text("Content 2"); }});
                    bc.SetItems({"Home", "Settings", "Profile"});
                    unigui::TreeNode root{"Root",
                                          {{"Child1", {}}, {"Child2", {{"Grandchild", {}}}}}};
                    tv.SetRoot(root);
                    tabInit = true;
                }
                tabs.Render();
                bc.Render();
                tv.Render();
            });

            addGroup("Data", []() {
                static unigui::Table tbl("tbl", {"Name", "Age", "City"});
                static bool init = false;
                if (!init) {
                    tbl.AddRow({"Alice", "30", "NYC"});
                    tbl.AddRow({"Bob", "25", "LA"});
                    tbl.SetSortable(true);
                    init = true;
                }
                tbl.Render();
            });

            addGroup("File & Color", []() {
                static unigui::FilePath fp("fp", "File");
                fp.SetFilter("*.cpp;*.h");
                static unigui::DirPath dp("dp", "Folder");
                static unigui::ColorPicker cp("cp", "Color");
                fp.Render();
                dp.Render();
                cp.Render();
            });

            addGroup("Dialogs & Info", []() {
                static unigui::Dialog dlg("dlg", "Confirm", "Are you sure?");
                static unigui::Notification nf("nf");
                static unigui::StatusBar sb("sb", "Ready");
                static unigui::MenuBar mb("mb");
                static unigui::ToolBar tb("tb");
                static unigui::Label lbl("lbl", "A static label");
                static unigui::MultiLine ml("ml", "Line1\nLine2\nLine3");
                static unigui::Hyperlink hl("hl", "GitHub", "https://github.com");
                static unigui::Tag tag("tag", "v2.5");
                static unigui::DatePicker dpo("dpo", "Date");
                static bool initD = false;
                if (!initD) {
                    mb.SetMenus({{"File", {{"Exit", [] {}}}}});
                    tb.SetItems({{"Save", [] {}}, {"Load", [] {}}});
                    initD = true;
                }
                dlg.Render();
                nf.Render();
                sb.Render();
                mb.Render();
                tb.Render();
                lbl.Render();
                ml.Render();
                hl.Render();
                tag.Render();
                dpo.Render();
            });

            // ── v3.0 Theme Switcher ────────────────────────────────────
            auto themePanel = std::make_shared<unigui::Panel>("themes", "v3.0 Themes");
            themePanel->SetContentCallback([]() {
                static int selected = 0;
                auto names = unigui::theme::ThemeRegistry::Instance().List();
                if (im::BeginCombo("Theme", names.empty() ? "" : names[selected].c_str())) {
                    for (int i = 0; i < (int) names.size(); i++) {
                        bool isSel = (selected == i);
                        if (im::Selectable(names[i].c_str(), &isSel)) {
                            selected = i;
                            unigui::theme::ThemeRegistry::Instance().Apply(names[i]);
                        }
                    }
                    im::EndCombo();
                }
            });
            win->AddPanel(themePanel);

            // ── v3.0 Card Demo ──────────────────────────────────────────
            auto cardPanel = std::make_shared<unigui::Panel>("card", "Card (v3.0)");
            cardPanel->SetContentCallback([]() {
                static unigui::Card card("Elevated Card");
                card.SetContent([]() {
                    im::TextWrapped("This is a card with drop shadow and rounded corners. "
                                    "Cards can have titles, content, and footers.");
                    static int clicks = 0;
                    if (im::Button("Click Me"))
                        clicks++;
                    im::SameLine();
                    im::Text(std::format("Clicks: {}", clicks));
                });
                card.SetFooter([]() { im::TextDisabled("Card footer"); });
                card.Render();
            });
            win->AddPanel(cardPanel);

            // ── v3.0 Shimmer Demo ───────────────────────────────────────
            auto shimmerPanel =
                std::make_shared<unigui::Panel>("shimmer", "Shimmer Loading (v3.0)");
            shimmerPanel->SetContentCallback([]() {
                static unigui::Shimmer sh;
                static bool init = true;
                if (init) {
                    sh.AddBlock(300, 16, 0, 0);
                    sh.AddBlock(250, 16, 0, 22);
                    sh.AddBlock(200, 16, 0, 44);
                    sh.AddCircle(36, 0, 66);
                    sh.AddBlock(400, 40, 46, 66);
                    sh.Start();
                    init = false;
                }
                if (im::Button(sh.IsPlaying() ? "Stop" : "Start")) {
                    sh.IsPlaying() ? sh.Stop() : sh.Start();
                }
                im::SameLine();
                im::Text("(skeleton shimmer)");
                im::Dummy(0, 4);
                sh.Render();
            });
            win->AddPanel(shimmerPanel);

            // ── v3.0 Badge Demo ─────────────────────────────────────────
            auto badgePanel = std::make_shared<unigui::Panel>("badge", "Badge (v3.0)");
            badgePanel->SetContentCallback([]() {
                static unigui::Badge dot("");
                dot.SetVariant(unigui::Badge::Dot);
                static unigui::Badge cnt("");
                cnt.SetCount(7);
                cnt.SetColor(IM_COL32(233, 69, 96, 255));
                static unigui::Badge lbl("NEW");
                lbl.SetColor(IM_COL32(0, 180, 100, 255));

                im::Text("Dot:");
                im::SameLine();
                dot.Render();
                im::SameLine(80);
                im::Text("Count:");
                im::SameLine();
                cnt.Render();
                im::SameLine(170);
                im::Text("Label:");
                im::SameLine();
                lbl.Render();
            });
            win->AddPanel(badgePanel);

            first = false;
        }
        win->Render();

        unigui::Render();
        frame++;
        if (max_frames > 0 && frame >= max_frames)
            break;
    }
    unigui::Shutdown();
    std::printf("[gallery] Shutdown\n");
    return 0;
}
