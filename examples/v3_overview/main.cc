/// v3_overview — showcases all v3.0 features using UniGUI's own widget API
#include <unigui/unigui.h>
#include <cstdio>
#include <cstdlib>
#include <cmath>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--frames" && i+1 < argc)
            max_frames = std::atoi(argv[++i]);

    unigui::AppConfig cfg;
    cfg.width = 1400; cfg.height = 900;
    cfg.title = "UniGUI v3 — Feature Overview (pure UniGUI widgets)";
    if (!unigui::Init(cfg)) return 1;
    std::printf("[v3_overview] Init OK\n");
    fflush(stdout);

    // ── Build UI once ────────────────────────────────────────────────────
    auto win = std::make_shared<unigui::Window>("main", "UniGUI v3 Features");
    win->SetPosition(20, 20);
    win->SetSize(1360, 850);

    // ── Panel: Theme + Easing Curves ─────────────────────────────────────
    auto mainPanel = std::make_shared<unigui::Panel>("mainPanel", "Theme Switcher + Easing Curves");
    mainPanel->SetContentCallback([]() {
        // Theme switcher combo
        static int selTheme = 0;
        auto names = unigui::theme::ThemeRegistry::Instance().List();
        if (ImGui::BeginCombo("Theme", names.empty() ? "" : names[selTheme].c_str())) {
            for (int i = 0; i < (int)names.size(); i++) {
                bool isSel = (selTheme == i);
                if (ImGui::Selectable(names[i].c_str(), &isSel)) {
                    selTheme = i;
                    unigui::theme::ThemeRegistry::Instance().Apply(names[i]);
                }
            }
            ImGui::EndCombo();
        }
        ImGui::Separator();

        static float t = 0.f;
        ImGui::SliderFloat("Time", &t, 0.f, 1.f, "%.3f");
        ImGui::SameLine();
        static unigui::Button animBtn("animBtn", "Animate");
        animBtn.Render();
        if (animBtn.WasClicked()) t = 0.f;

        static std::pair<unigui::fx::EasingCurve, const char*> curves[] = {
            {unigui::fx::EasingCurve::Linear,    "Linear"},
            {unigui::fx::EasingCurve::QuadIn,    "QuadIn"},
            {unigui::fx::EasingCurve::QuadOut,   "QuadOut"},
            {unigui::fx::EasingCurve::QuadInOut, "QuadInOut"},
            {unigui::fx::EasingCurve::CubicIn,   "CubicIn"},
            {unigui::fx::EasingCurve::CubicOut,  "CubicOut (ease)"},
            {unigui::fx::EasingCurve::ExpoIn,    "ExpoIn"},
            {unigui::fx::EasingCurve::ExpoOut,   "ExpoOut"},
            {unigui::fx::EasingCurve::ElasticOut,"ElasticOut"},
            {unigui::fx::EasingCurve::BounceOut, "BounceOut"},
        };

        auto* dl = ImGui::GetWindowDrawList();
        ImVec2 orig = ImGui::GetCursorScreenPos();
        float graphW = ImGui::GetContentRegionAvail().x - 20;
        float graphH = 36.f;

        for (int i = 0; i < 10; i++) {
            ImVec2 off(orig.x + 120, orig.y + i * 44.f + 6);
            float y0 = off.y + graphH;

            // Curve preview — custom draw
            for (float x = 0; x <= graphW; x += 2.f) {
                float px = x / graphW;
                float py = unigui::fx::Ease(px, curves[i].first);
                dl->AddRectFilled(ImVec2(off.x + x, y0 - py * graphH),
                                 ImVec2(off.x + x + 2.f, y0 - py * graphH + 2.f),
                                 IM_COL32(40, 49, 237, 200));
            }
            float mx = t * graphW;
            float my = unigui::fx::Ease(t, curves[i].first) * graphH;
            dl->AddCircleFilled(ImVec2(off.x + mx, y0 - my), 4.f, IM_COL32(255, 255, 255, 255));

            ImGui::SetCursorScreenPos(ImVec2(orig.x, orig.y + i * 44.f + 4));
            static unigui::Label labels[10] = {
                unigui::Label("l0", ""), unigui::Label("l1", ""), unigui::Label("l2", ""),
                unigui::Label("l3", ""), unigui::Label("l4", ""), unigui::Label("l5", ""),
                unigui::Label("l6", ""), unigui::Label("l7", ""), unigui::Label("l8", ""),
                unigui::Label("l9", ""),
            };
            ImGui::SetCursorScreenPos(orig);
            ImGui::Dummy(ImVec2(graphW, graphH + 8));
        }
        t += ImGui::GetIO().DeltaTime * 0.4f;
        if (t > 1.f) t -= 1.f;
    });
    win->AddPanel(mainPanel);

    // ── Panel: Visual Effects ─────────────────────────────────────────────
    auto effectsPanel = std::make_shared<unigui::Panel>("effects", "Visual Effects (Shadow / Glow / Gradient / Glass)");
    effectsPanel->SetContentCallback([]() {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float pw = (avail.x - 20) / 2.f;

        // shadow + glow side by side
        ImGui::BeginChild("fxRow", ImVec2(avail.x - 10, 90), ImGuiChildFlags_AutoResizeY);
        auto* dl = ImGui::GetWindowDrawList();
        ImVec2 c = ImGui::GetCursorScreenPos();

        unigui::fx::ShadowEffect sh(6.f, 2, 2, IM_COL32(0,0,0,80), 3);
        sh.SetRect(c, ImVec2(c.x + pw - 10, c.y + 60));
        sh.Push(dl);
        dl->AddRectFilled(c, ImVec2(c.x + pw - 10, c.y + 60), IM_COL32(45,45,55,255), 6.f);
        sh.Pop();
        dl->AddText(ImVec2(c.x + 8, c.y + 22), IM_COL32_WHITE, "ShadowEffect");

        c.x += pw + 10;
        unigui::fx::GlowEffect gw(8.f, IM_COL32(100,149,237,100), 3);
        gw.SetRect(c, ImVec2(c.x + pw - 10, c.y + 60));
        gw.Push(dl);
        dl->AddRectFilled(c, ImVec2(c.x + pw - 10, c.y + 60), IM_COL32(45,45,55,255), 6.f);
        gw.Pop();
        dl->AddText(ImVec2(c.x + 8, c.y + 22), IM_COL32_WHITE, "GlowEffect");
        ImGui::Dummy(ImVec2(avail.x - 10, 72));
        ImGui::EndChild();

        // gradient
        ImGui::BeginChild("fxGrad", ImVec2(avail.x - 10, 50), ImGuiChildFlags_AutoResizeY);
        c = ImGui::GetCursorScreenPos();
        unigui::fx::GradientBrush::Horizontal(dl, c, ImVec2(c.x + avail.x - 20, c.y + 30),
            IM_COL32(40,49,237,255), IM_COL32(233,69,96,255));
        dl->AddText(ImVec2(c.x + 8, c.y + 8), IM_COL32_WHITE, "GradientBrush::Horizontal");
        ImGui::Dummy(ImVec2(avail.x - 10, 40));
        ImGui::EndChild();

        // glass
        ImGui::BeginChild("fxGlass", ImVec2(avail.x - 10, 70), ImGuiChildFlags_AutoResizeY);
        c = ImGui::GetCursorScreenPos();
        dl->AddRectFilled(c, ImVec2(c.x + avail.x - 20, c.y + 50), IM_COL32(40,49,237,180), 8.f);
        unigui::fx::BlurEffect g(10.f, 0.15f, IM_COL32(255,255,255,30));
        g.SetRect(ImVec2(c.x + 15, c.y + 5), ImVec2(c.x + avail.x - 35, c.y + 45));
        g.Push(dl); g.Pop();
        dl->AddText(ImVec2(c.x + 25, c.y + 18), IM_COL32(255,255,255,200), "BlurEffect / GlassPanel");
        ImGui::Dummy(ImVec2(avail.x - 10, 60));
        ImGui::EndChild();
    });
    win->AddPanel(effectsPanel);

    // ── Panel: Card + Shimmer + Badge + Skeleton ──────────────────────────
    auto widgetsPanel = std::make_shared<unigui::Panel>("widgets", "New Widgets (Card, Shimmer, Badge, Skeleton)");
    widgetsPanel->SetContentCallback([]() {
        ImVec2 avail = ImGui::GetContentRegionAvail();
        float w = (avail.x - 30) / 3.f;

        // Card
        ImGui::BeginChild("card-col", ImVec2(w, 170), ImGuiChildFlags_Borders);
        static unigui::Card card("Feature Card");
        card.SetShadowRadius(5.f);
        card.SetContent([]() {
            ImGui::TextWrapped("Cards with shadow, rounded corners, and footers.");
            static int n = 0;
            static unigui::Button cb("cb", "Count");
            cb.Render();
            if (cb.WasClicked()) n++;
            ImGui::SameLine(); ImGui::Text("%d clicks", n);
        });
        card.SetFooter([]() { ImGui::TextDisabled("Card v3.0"); });
        card.Render();
        ImGui::EndChild();
        ImGui::SameLine();

        // Shimmer
        ImGui::BeginChild("shim-col", ImVec2(w, 170), ImGuiChildFlags_Borders);
        static unigui::Shimmer sh;
        static bool init = true;
        if (init) {
            sh.AddBlock(180, 12, 0, 0); sh.AddBlock(140, 12, 0, 16);
            sh.AddBlock(100, 12, 0, 32); sh.AddCircle(24, 0, 48);
            sh.AddBlock(150, 26, 30, 48); sh.SetSpeed(1.2f); sh.Start(); init = false;
        }
        static unigui::Button sBtn("sBtn", sh.IsPlaying() ? "Stop" : "Start");
        sBtn.Render();
        if (sBtn.WasClicked()) sh.IsPlaying() ? sh.Stop() : sh.Start();
        ImGui::Dummy(ImVec2(0, 4)); sh.Render();
        ImGui::EndChild();
        ImGui::SameLine();

        // Badge + Skeleton
        ImGui::BeginChild("badge-col", ImVec2(w, 170), ImGuiChildFlags_Borders);
        static unigui::Badge dot(""); dot.SetVariant(unigui::Badge::Dot);
        static unigui::Badge cnt(""); cnt.SetCount(12);
        cnt.SetColor(IM_COL32(233,69,96,255));
        static unigui::Badge lbl("NEW"); lbl.SetColor(IM_COL32(0,180,100,255));
        ImGui::Text("Dot:"); ImGui::SameLine(); dot.Render();
        ImGui::SameLine(70); ImGui::Text("Count:"); ImGui::SameLine(); cnt.Render();
        ImGui::Text("Label:"); ImGui::SameLine(); lbl.Render();
        ImGui::Spacing();
        static unigui::SkeletonScreen sk = unigui::SkeletonScreen::FromSize(160, 80, 2);
        sk.Render();
        ImGui::EndChild();
    });
    win->AddPanel(widgetsPanel);

    // ── Panel: Buttons + Toast ────────────────────────────────────────────
    auto interactPanel = std::make_shared<unigui::Panel>("interact", "Animated Buttons, GradientText, Toast");
    interactPanel->SetContentCallback([]() {
        unigui::GradientText::RenderHex("UniGUI v3 — Feature Overview",
                                        40, 49, 237, 233, 69, 96);
        ImGui::Spacing();

        static unigui::Button btn1("b1", "Primary");  btn1.SetColorVariant(unigui::Button::Primary);
        static unigui::Button btn2("b2", "Danger");   btn2.SetColorVariant(unigui::Button::Danger);
        static unigui::Button btn3("b3", "Success");  btn3.SetColorVariant(unigui::Button::Success);
        static unigui::Button btn4("b4", "Default");

        btn1.Render(); ImGui::SameLine(); btn2.Render(); ImGui::SameLine();
        btn3.Render(); ImGui::SameLine(); btn4.Render();

        if (btn1.WasClicked()) unigui::Toast::Info("Primary button — UniGUI widget, not raw ImGui!");
        if (btn2.WasClicked()) unigui::Toast::Error("Danger — animated hover + Toast!");
        if (btn3.WasClicked()) unigui::Toast::Success("Success — gradient text + buttons!");
        if (btn4.WasClicked()) unigui::Toast::Warn("Default — smooth hover transition!");

        ImGui::Spacing();
        static unigui::Button ti("ti", "Toast Info");    ti.Render(); ImGui::SameLine();
        static unigui::Button ts("ts", "Toast Success"); ts.Render(); ImGui::SameLine();
        static unigui::Button tw("tw", "Toast Warn");    tw.Render(); ImGui::SameLine();
        static unigui::Button te("te", "Toast Error");   te.Render();
        if (ti.WasClicked()) unigui::Toast::Info("Info toast");
        if (ts.WasClicked()) unigui::Toast::Success("Success toast");
        if (tw.WasClicked()) unigui::Toast::Warn("Warning toast");
        if (te.WasClicked()) unigui::Toast::Error("Error toast");
    });
    win->AddPanel(interactPanel);

    // ── Panel: DataTable + Button Interaction ────────────────────────────
    auto tablePanel = std::make_shared<unigui::Panel>("tablePanel", "DataTable<T> — Rows, Filter, Action Buttons");
    tablePanel->SetContentCallback([]() {
        struct Position {
            std::string symbol; int longVol, shortVol; double pnl; double cost;
        };
        static std::vector<Position> positions;
        static bool initData = true;
        if (initData) {
            for (int i = 0; i < 15; ++i) {
                positions.push_back({
                    std::string(1, 'A' + (i % 4)) + std::to_string(6000 + i) + ".CF",
                    (i % 3 == 0) ? (i + 1) * 100 : 0,
                    (i % 3 != 0) ? (i + 1) * 100 : 0,
                    (i % 2) ? (i + 1) * 500.0 : -(i + 1) * 300.0,
                    5000.0 + i * 100.0
                });
            }
            initData = false;
        }

        static unigui::DataTable<Position> table("持仓", {
            {"合约", 100}, {"多头手", 60}, {"空头手", 60}, {"盈亏", 80}, {"成本", 80}
        });
        table.SetDataSource(&positions);
        table.SetCellFormatter([](int, int col, const Position& p) -> std::string {
            switch (col) {
                case 0: return p.symbol;
                case 1: return p.longVol > 0 ? std::to_string(p.longVol) : "-";
                case 2: return p.shortVol > 0 ? std::to_string(p.shortVol) : "-";
                case 3: { char buf[32]; snprintf(buf, 32, "%.2f", p.pnl); return buf; }
                case 4: { char buf[32]; snprintf(buf, 32, "%.2f", p.cost); return buf; }
            }
            return "-";
        });
        table.SetRowColor([](int, const Position& p) {
            return p.pnl >= 0 ? IM_COL32(0, 160, 80, 80)
                              : IM_COL32(220, 60, 60, 80);
        });

        // ── Filter bar ────────────────────────────────────────────────
        static char filterBuf[64] = "";
        ImGui::InputTextWithHint("##filter", "Filter...", filterBuf, sizeof(filterBuf));
        if (filterBuf[0]) table.SetFilterText(filterBuf);
        else table.SetFilterText("");

        ImGui::SameLine();
        static unigui::Button addBtn("addBtn", "+ Add");
        static unigui::Button delBtn("delBtn", "- Del Sel");
        addBtn.Render(); ImGui::SameLine(); delBtn.Render();

        if (addBtn.WasClicked()) {
            int n = (int)positions.size();
            positions.push_back({"NEW" + std::to_string(6001 + n), 500, 0, 1200.0, 5100.0});
        }
        if (delBtn.WasClicked()) {
            int sel = table.GetSelectedRow();
            if (sel >= 0 && sel < (int)positions.size())
                positions.erase(positions.begin() + sel);
        }

        // ── Table ─────────────────────────────────────────────────────
        table.SetVirtualScroll(true);
        table.Render();

        // ── Row action buttons ────────────────────────────────────────
        int sel = table.GetSelectedRow();
        if (sel >= 0 && sel < (int)positions.size()) {
            ImGui::Text("Selected: %s | PnL: %.2f", positions[sel].symbol.c_str(), positions[sel].pnl);
            static unigui::Button upBtn("upBtn", "+ 多");
            static unigui::Button dnBtn("dnBtn", "- 空");
            upBtn.Render(); ImGui::SameLine(); dnBtn.Render();
            if (upBtn.WasClicked()) { positions[sel].longVol += 100; positions[sel].pnl += 50; }
            if (dnBtn.WasClicked()) { positions[sel].shortVol += 100; positions[sel].pnl -= 50; }
        }

        static int addCount = 0;
        ImGui::Text("Rows: %zu | Filter: '%s'", positions.size(), filterBuf);
    });
    win->AddPanel(tablePanel);

    // ── Panel: Animation Demo ─────────────────────────────────────────────
    auto animPanel = std::make_shared<unigui::Panel>("anim", "Animation State + Transition");
    animPanel->SetContentCallback([]() {
        static unigui::fx::AnimationState fadeAnim, slideAnim, bounceAnim;
        float dt = ImGui::GetIO().DeltaTime;

        static unigui::Button fb("fb", "Fade In");  fb.Render(); ImGui::SameLine();
        static unigui::Button sb("sb", "Slide In"); sb.Render(); ImGui::SameLine();
        static unigui::Button bb("bb", "Bounce");   bb.Render();
        if (fb.WasClicked()) fadeAnim.Play(0.5f, unigui::fx::EasingCurve::CubicOut);
        if (sb.WasClicked()) slideAnim.Play(0.5f, unigui::fx::EasingCurve::ExpoOut);
        if (bb.WasClicked()) bounceAnim.Play(0.8f, unigui::fx::EasingCurve::BounceOut);

        float a = fadeAnim.Update(dt);
        float s = unigui::fx::Transition::SlideIn(slideAnim, -100.f, 0.3f, unigui::fx::EasingCurve::ExpoOut, dt);
        float sc = unigui::fx::Transition::Scale(bounceAnim, 0.3f, 1.0f, 0.8f, unigui::fx::EasingCurve::BounceOut, dt);

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
        ImGui::TextColored(ImVec4(1,1,0,1), "Fade alpha: %.2f", a);
        ImGui::PopStyleVar();

        ImGui::TextColored(ImVec4(0.5f,1,0.5f,1), "Slide offset: %.0f", s);
        ImGui::TextColored(ImVec4(1,0.5f,0.5f,1), "Bounce scale: %.2f", sc);
    });
    win->AddPanel(animPanel);

    // ── Main loop ─────────────────────────────────────────────────────────
    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();

        // Rebuild theme menu each frame (live list)
        win->Render();
        unigui::Render(); frame++;
        if (max_frames > 0 && frame >= max_frames) break;
    }
    unigui::Shutdown();
    std::printf("[v3_overview] Done — %d frames\n", frame);
    fflush(stdout);
    return 0;
}
