// v3_overview — UniGUI v3.2.6 full feature showcase
#include <unigui/unigui.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--frames" && i+1 < argc)
            max_frames = std::atoi(argv[++i]);

    unigui::AppConfig cfg; cfg.width = 1300; cfg.height = 800;
    cfg.title = "UniGUI v3.2.6 Widget Showcase";
    if (!unigui::Init(cfg)) return 1;
    std::printf("[v3] Init OK\n"); fflush(stdout);

    auto win = std::make_shared<unigui::Window>("main", "UniGUI v3.2.6 Widget Showcase");
    win->SetSize(1260, 760);

    // ──── P1: Theme Switcher ──────────────────────────────────────────
    auto pTheme = std::make_shared<unigui::Panel>("theme", "1. Theme Switcher");
    pTheme->SetContentCallback([]() {
        static int sel = 0; auto names = unigui::theme::ThemeRegistry::Instance().List();
        if (names.empty()) return;
        if (ImGui::BeginCombo("Theme", names[sel].c_str())) {
            for (int i = 0; i < (int)names.size(); i++) {
                bool b = (sel == i);
                if (ImGui::Selectable(names[i].c_str(), &b)) {
                    sel = i; unigui::theme::ThemeRegistry::Instance().Apply(names[i]);
                }
            }
            ImGui::EndCombo();
        }
    });
    win->AddPanel(pTheme);

    // ──── P2: DataTable — multi-select, auto-width, filter, add/del, sort ─
    auto pDT = std::make_shared<unigui::Panel>("datatable",
        "2. DataTable<T> — Multi-Select (CTRL+click), Auto-Width, Sort All, Filter, Add/Del");
    pDT->SetContentCallback([]() {
        struct Pos { std::string sym; int lv, sv; double pnl, cost; };
        static std::vector<Pos> data;
        static bool init = true;
        if (init) { for (int i=0;i<12;i++) data.push_back(Pos{
            std::string(1,'A'+i%4)+std::to_string(6000+i)+".CF",
            (i%3==0)?(i+1)*100:0,(i%3!=0)?(i+1)*100:0,
            (i%2)?(i+1)*500.0:-(i+1)*300.0,5000.0+i*100.0}); init=false; }

        static char filter[64] = "";
        ImGui::InputTextWithHint("##f", "Filter...", filter, sizeof(filter));
        ImGui::SameLine();
        static unigui::Button addBtn("add","+Add"); addBtn.Render(); ImGui::SameLine();
        static unigui::Button delBtn("del","-Del"); delBtn.Render();

        static unigui::DataTable<Pos> t("持仓",{{"合约",110},{"多头",60},{"空头",60},{"盈亏",80},{"成本",80}});
        t.SetDataSource(&data);
        t.SetMultiSelect(true);
        t.SetColumnAutoWidth(0, true);
        t.SetCellFormatter([](int,int c,const Pos& p)->std::string{
            switch(c){case 0:return p.sym;case 1:return p.lv>0?std::to_string(p.lv):"-";
            case 2:return p.sv>0?std::to_string(p.sv):"-";
            case 3:{char b[32];snprintf(b,32,"%.1f",p.pnl);return b;}
            case 4:{char b[32];snprintf(b,32,"%.0f",p.cost);return b;}}return"-";
        });
        t.SetRowColor([](int,const Pos& p){return p.pnl>=0?IM_COL32(0,160,80,80):IM_COL32(220,60,60,80);});
        if(filter[0])t.SetFilterText(filter);else t.SetFilterText("");
        t.SetVirtualScroll(false); t.Render();

        if(addBtn.WasClicked()){int n=(int)data.size();data.push_back(Pos{"NEW"+std::to_string(6001+n),500,0,1200.0,5100.0});}
        if(delBtn.WasClicked()){auto sel=t.GetSelectedRows();for(int s:sel)if(s>=0&&s<(int)data.size())data.erase(data.begin()+s);}
        auto sel = t.GetSelectedRows();
        if(!sel.empty()) ImGui::Text("Selected %zu rows", sel.size());
    });
    win->AddPanel(pDT);

    // ──── P3: TreeView with custom renderer ─────────────────────────────
    auto pTV = std::make_shared<unigui::Panel>("treeview",
        "3. TreeView — Custom Node Renderer (label + funds + usage bar)");
    pTV->SetContentCallback([]() {
        static unigui::TreeNode root;
        static bool built = false;
        if (!built) {
            root.label = "Account Groups";
            unigui::TreeNode g1{"测试一组", {}, false};
            g1.children.push_back({"acc10001 | 1,000万 | 使用率 65%", {}, false});
            g1.children.push_back({"acc10002 | 2,500万 | 使用率 42%", {}, false});
            g1.children.push_back({"acc10003 |   800万 | 使用率 88%", {}, false});
            unigui::TreeNode g2{"测试二组", {}, false};
            g2.children.push_back({"acc10004 | 5,000万 | 使用率 30%", {}, false});
            g2.children.push_back({"acc10005 | 1,200万 | 使用率 55%", {}, false});
            root.children.push_back(g1); root.children.push_back(g2);
            built = true;
        }
        static unigui::TreeView tv("tree");
        tv.SetRoot(root);

        // Custom renderer: draw progress bar per leaf node
        static std::vector<float> usages = {0.65f, 0.42f, 0.88f, 0.30f, 0.55f};
        static int usageIdx = 0;
        tv.SetNodeRenderer([](int, int depth, const unigui::TreeNode& node) {
            if (depth > 0 && usageIdx < (int)usages.size()) {
                ImGui::SameLine(ImGui::GetWindowWidth() - 180);
                ImGui::ProgressBar(usages[usageIdx], ImVec2(120, 14), "");
                ImGui::SameLine();
                ImGui::Text("%.0f%%", usages[usageIdx] * 100);
                usageIdx++;
            }
        });
        usageIdx = 0;
        tv.Render();
    });
    win->AddPanel(pTV);

    // ──── P4: MultiSplitter — 3-column H + V nested ───────────────────
    auto pMS = std::make_shared<unigui::Panel>("multisplitter",
        "4. MultiSplitter — Horizontal 3-col + Vertical nested (FSA layout)");
    pMS->SetContentCallback([]() {
        static unigui::MultiSplitter msH("msH", unigui::MultiSplitter::Horizontal);
        static bool built = false;
        if (!built) {
            // Left: pod list (vertical split nested)
            msH.AddPanel(0.25f, []() {
                static unigui::MultiSplitter msV("msV", unigui::MultiSplitter::Vertical);
                static bool vb = false;
                if(!vb){msV.AddPanel(0.4f,[](){ImGui::Text("Pod List");ImGui::Button("Pod 1");ImGui::Button("Pod 2");});
                        msV.AddPanel(0.6f,[](){ImGui::Text("Account Info");ImGui::Text("Group: A");ImGui::Text("Total: 5,300万");});vb=true;}
                msV.Render();
            });
            // Center: position details
            msH.AddPanel(0.35f, []() {
                ImGui::Text("Position Details");
                ImGui::Separator();
                ImGui::TextUnformatted("IF2406.CF  Long: 100  PnL: +500");
                ImGui::TextUnformatted("IH2406.CF  Short: 200  PnL: -300");
                ImGui::TextUnformatted("IC2406.CF  Long: 300  PnL: +800");
            });
            // Right: chart
            msH.AddPanel(0.40f, []() {
                ImGui::Text("Price Chart (TimeSeriesChart)");
                ImGui::TextUnformatted("[Chart rendering here]");
            });
            built = true;
        }
        msH.Render();
    });
    win->AddPanel(pMS);

    // ──── P5: Buttons + Toast ────────────────────────────────────────────
    auto pBtn = std::make_shared<unigui::Panel>("buttons", "5. Animated Buttons + Toast Notifications");
    pBtn->SetContentCallback([]() {
        static unigui::Button b1("b1","Primary"); b1.SetColorVariant(unigui::Button::Primary);
        static unigui::Button b2("b2","Danger");  b2.SetColorVariant(unigui::Button::Danger);
        static unigui::Button b3("b3","Success"); b3.SetColorVariant(unigui::Button::Success);
        static unigui::Button b4("b4","Default");
        b1.Render();ImGui::SameLine();b2.Render();ImGui::SameLine();b3.Render();ImGui::SameLine();b4.Render();
        if(b1.WasClicked())unigui::Toast::Info("Primary");
        if(b2.WasClicked())unigui::Toast::Error("Danger");
        if(b3.WasClicked())unigui::Toast::Success("Success");
        if(b4.WasClicked())unigui::Toast::Warn("Default");
        ImGui::Spacing();ImGui::Separator();
        static unigui::Button ti("ti","Toast Info");ti.Render();ImGui::SameLine();
        static unigui::Button ts("ts","Toast Success");ts.Render();ImGui::SameLine();
        static unigui::Button tw("tw","Toast Warn");tw.Render();ImGui::SameLine();
        static unigui::Button te("te","Toast Error");te.Render();
        if(ti.WasClicked())unigui::Toast::Info("Info");if(ts.WasClicked())unigui::Toast::Success("Success");
        if(tw.WasClicked())unigui::Toast::Warn("Warn");if(te.WasClicked())unigui::Toast::Error("Error");
    });
    win->AddPanel(pBtn);

    // ──── P6: Badge + GradientText ───────────────────────────────────────
    auto pBadge = std::make_shared<unigui::Panel>("badge", "6. Badge + GradientText");
    pBadge->SetContentCallback([]() {
        unigui::GradientText::RenderHex("UniGUI v3.2.6 — Feature Complete", 40, 49, 237, 233, 69, 96); ImGui::Spacing();
        static unigui::Badge dot("");dot.SetVariant(unigui::Badge::Dot);
        static unigui::Badge cnt("");cnt.SetCount(12);cnt.SetColor(IM_COL32(233,69,96,255));
        static unigui::Badge lbl("NEW");lbl.SetColor(IM_COL32(0,180,100,255));
        ImGui::Text("Dot:");ImGui::SameLine();dot.Render();ImGui::SameLine(80);
        ImGui::Text("Count:");ImGui::SameLine();cnt.Render();ImGui::SameLine(170);
        ImGui::Text("Label:");ImGui::SameLine();lbl.Render();
    });
    win->AddPanel(pBadge);

    // ──── P7: Animation ──────────────────────────────────────────────────
    auto pAnim = std::make_shared<unigui::Panel>("anim", "7. Animation State + Transition");
    pAnim->SetContentCallback([]() {
        static unigui::fx::AnimationState fa,ba; float dt=ImGui::GetIO().DeltaTime;
        static unigui::Button fb("fb","Fade In");fb.Render();ImGui::SameLine();
        static unigui::Button bb("bb","Bounce");bb.Render();
        if(fb.WasClicked())fa.Play(0.5f,unigui::fx::EasingCurve::CubicOut);
        if(bb.WasClicked())ba.Play(0.8f,unigui::fx::EasingCurve::BounceOut);
        float a=fa.Update(dt),sc=unigui::fx::Transition::Scale(ba,0.3f,1.f,0.8f,unigui::fx::EasingCurve::BounceOut,dt);
        ImGui::PushStyleVar(ImGuiStyleVar_Alpha,a);ImGui::TextColored(ImVec4(1,1,0,1),"Fade: %.2f",a);ImGui::PopStyleVar();
        ImGui::TextColored(ImVec4(1,0.5f,0.5f,1),"Bounce: %.2f",sc);
    });
    win->AddPanel(pAnim);

    // ── Layout persistence: save on first frame, restore on restart ────
    static bool saved = false;
    if (!saved) { win->RestoreLayout(R"({"x":20,"y":20,"w":1260,"h":760})"); saved = true; }

    int frame = 0;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();
        win->Render();
        unigui::Render(); frame++;
        if (max_frames > 0 && frame >= max_frames) break;
    }
    // Print layout for persistence
    std::string layout = win->SaveLayout();
    std::printf("[v3] Layout: %s\n", layout.c_str()); fflush(stdout);
    unigui::Shutdown();
    std::printf("[v3] Done — %d frames\n", frame); fflush(stdout);
    return 0;
}
