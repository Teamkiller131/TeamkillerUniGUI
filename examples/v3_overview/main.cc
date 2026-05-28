// v3_overview — UniGUI widget showcase (stable subset)
#include <unigui/unigui.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--frames" && i+1 < argc)
            max_frames = std::atoi(argv[++i]);

    unigui::AppConfig cfg; cfg.width = 1300; cfg.height = 800;
    cfg.title = "UniGUI v3 Widget Showcase";
    if (!unigui::Init(cfg)) return 1;
    std::printf("[v3_overview] Init OK\n"); fflush(stdout);

    auto win = std::make_shared<unigui::Window>("main", "UniGUI v3.2.x Widget Showcase");
    win->SetSize(1260, 760);

    // === P1: Theme Switcher ==============================================
    auto pTheme = std::make_shared<unigui::Panel>("theme", "Theme Switcher");
    pTheme->SetContentCallback([]() {
        static int sel = 0;
        auto names = unigui::theme::ThemeRegistry::Instance().List();
        if (names.empty()) return;
        if (ImGui::BeginCombo("Theme", names[sel].c_str())) {
            for (int i = 0; i < (int)names.size(); i++) {
                bool b = (sel == i);
                if (ImGui::Selectable(names[i].c_str(), &b)) {
                    sel = i;
                    unigui::theme::ThemeRegistry::Instance().Apply(names[i]);
                }
            }
            ImGui::EndCombo();
        }
    });
    win->AddPanel(pTheme);

    // === P2: DataTable + Filter + Add/Del + Row Action ====================
    auto pDT = std::make_shared<unigui::Panel>("datatable",
        "DataTable<T> — Filter, Sort, Row Color, Selection, Add/Del, Row Edit");
    pDT->SetContentCallback([]() {
        struct Pos { std::string sym; int lv, sv; double pnl, cost; };
        static std::vector<Pos> data;
        static bool init = true;
        if (init) {
            for (int i = 0; i < 12; i++)
                data.push_back(Pos{
                    std::string(1, 'A'+i%4) + std::to_string(6000+i) + ".CF",
                    (i%3==0)?(i+1)*100:0, (i%3!=0)?(i+1)*100:0,
                    (i%2)?(i+1)*500.0:-(i+1)*300.0, 5000.0+i*100.0
                });
            init = false;
        }

        static char filter[64] = "";
        ImGui::InputTextWithHint("##f", "Filter symbols...", filter, sizeof(filter));
        ImGui::SameLine();
        static unigui::Button addBtn("add","+Add"); addBtn.Render(); ImGui::SameLine();
        static unigui::Button delBtn("del","-Del"); delBtn.Render();

        static unigui::DataTable<Pos> t("持仓",{{"合约",110},{"多头",60},{"空头",60},{"盈亏",80},{"成本",80}});
        t.SetDataSource(&data);
        t.SetCellFormatter([](int,int c,const Pos& p)->std::string{
            switch(c){case 0:return p.sym;case 1:return p.lv>0?std::to_string(p.lv):"-";
            case 2:return p.sv>0?std::to_string(p.sv):"-";
            case 3:{char b[32];snprintf(b,32,"%.1f",p.pnl);return b;}
            case 4:{char b[32];snprintf(b,32,"%.0f",p.cost);return b;}}return"-";
        });
        t.SetRowColor([](int,const Pos& p){return p.pnl>=0?IM_COL32(0,160,80,80):IM_COL32(220,60,60,80);});
        if(filter[0]) t.SetFilterText(filter); else t.SetFilterText("");
        t.SetVirtualScroll(false);
        t.Render();

        if (addBtn.WasClicked()) { int n=(int)data.size(); data.push_back(Pos{"NEW"+std::to_string(6001+n),500,0,1200.0,5100.0}); }
        if (delBtn.WasClicked()) { int s=t.GetSelectedRow(); if(s>=0&&s<(int)data.size()) data.erase(data.begin()+s); }

        int sel = t.GetSelectedRow();
        if (sel >= 0 && sel < (int)data.size()) {
            ImGui::Text("Selected: %s | PnL: %.1f", data[sel].sym.c_str(), data[sel].pnl);
            static unigui::Button up("up","+Long"); static unigui::Button dn("dn","-Del");
            up.Render(); ImGui::SameLine(); dn.Render();
            if (up.WasClicked()) { data[sel].lv += 100; data[sel].pnl += 50; }
            if (dn.WasClicked()) { data[sel].sv += 100; data[sel].pnl -= 50; }
        }
    });
    win->AddPanel(pDT);

    // === P3: Animated Buttons + Toast ====================================
    auto pBtn = std::make_shared<unigui::Panel>("buttons", "Animated Buttons + Toast Notifications");
    pBtn->SetContentCallback([]() {
        static unigui::Button b1("b1","Primary");  b1.SetColorVariant(unigui::Button::Primary);
        static unigui::Button b2("b2","Danger");   b2.SetColorVariant(unigui::Button::Danger);
        static unigui::Button b3("b3","Success");  b3.SetColorVariant(unigui::Button::Success);
        static unigui::Button b4("b4","Default");
        b1.Render(); ImGui::SameLine(); b2.Render(); ImGui::SameLine();
        b3.Render(); ImGui::SameLine(); b4.Render();
        if (b1.WasClicked()) unigui::Toast::Info("Primary button");
        if (b2.WasClicked()) unigui::Toast::Error("Danger button");
        if (b3.WasClicked()) unigui::Toast::Success("Success button");
        if (b4.WasClicked()) unigui::Toast::Warn("Default button");

        ImGui::Spacing(); ImGui::Separator();
        static unigui::Button ti("ti","Toast Info");    ti.Render(); ImGui::SameLine();
        static unigui::Button ts("ts","Toast Success"); ts.Render(); ImGui::SameLine();
        static unigui::Button tw("tw","Toast Warn");    tw.Render(); ImGui::SameLine();
        static unigui::Button te("te","Toast Error");   te.Render();
        if (ti.WasClicked()) unigui::Toast::Info("Info toast");
        if (ts.WasClicked()) unigui::Toast::Success("Success toast");
        if (tw.WasClicked()) unigui::Toast::Warn("Warning toast");
        if (te.WasClicked()) unigui::Toast::Error("Error toast");
    });
    win->AddPanel(pBtn);

    // === P4: Badge + GradientText =========================================
    auto pBadge = std::make_shared<unigui::Panel>("badge", "Badge + GradientText");
    pBadge->SetContentCallback([]() {
        unigui::GradientText::RenderHex("UniGUI v3.2.x — Widget Showcase", 40, 49, 237, 233, 69, 96);
        ImGui::Spacing();
        static unigui::Badge dot(""); dot.SetVariant(unigui::Badge::Dot);
        static unigui::Badge cnt(""); cnt.SetCount(12); cnt.SetColor(IM_COL32(233,69,96,255));
        static unigui::Badge lbl("NEW"); lbl.SetColor(IM_COL32(0,180,100,255));
        ImGui::Text("Dot:"); ImGui::SameLine(); dot.Render();
        ImGui::SameLine(80); ImGui::Text("Count:"); ImGui::SameLine(); cnt.Render();
        ImGui::SameLine(170); ImGui::Text("Label:"); ImGui::SameLine(); lbl.Render();
    });
    win->AddPanel(pBadge);

    // === P5: Animation ====================================================
    auto pAnim = std::make_shared<unigui::Panel>("anim", "Animation State + Transition");
    pAnim->SetContentCallback([]() {
        static unigui::fx::AnimationState fadeAnim, bounceAnim;
        float dt = ImGui::GetIO().DeltaTime;
        static unigui::Button fb("fb","Fade In"); fb.Render(); ImGui::SameLine();
        static unigui::Button bb("bb","Bounce");  bb.Render();
        if(fb.WasClicked()) fadeAnim.Play(0.5f, unigui::fx::EasingCurve::CubicOut);
        if(bb.WasClicked()) bounceAnim.Play(0.8f, unigui::fx::EasingCurve::BounceOut);
        float a = fadeAnim.Update(dt);
        float sc = unigui::fx::Transition::Scale(bounceAnim,0.3f,1.f,0.8f,unigui::fx::EasingCurve::BounceOut,dt);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, a);
        ImGui::TextColored(ImVec4(1,1,0,1),"Fade alpha: %.2f",a);
        ImGui::PopStyleVar();
        ImGui::TextColored(ImVec4(1,0.5f,0.5f,1),"Bounce scale: %.2f",sc);
    });
    win->AddPanel(pAnim);

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();
        win->Render();
        unigui::Render(); frame++;
        if (max_frames > 0 && frame >= max_frames) break;
    }
    unigui::Shutdown();
    std::printf("[v3_overview] Done — %d frames\n", frame); fflush(stdout);
    return 0;
}
