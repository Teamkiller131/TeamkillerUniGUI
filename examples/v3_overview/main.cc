// v3_overview — UniGUI v3.2.7 full feature showcase
#include <unigui/unigui.h>
#include <cstdio>
#include <cstdlib>

int main(int argc, char** argv) {
    int max_frames = 0;
    for (int i = 1; i < argc; i++)
        if (std::string(argv[i]) == "--frames" && i+1 < argc) max_frames = std::atoi(argv[++i]);

    unigui::AppConfig cfg; cfg.width = 1300; cfg.height = 800;
    cfg.title = "UniGUI v3.2.7 Widget Showcase";
    if (!unigui::Init(cfg)) return 1;
    std::printf("[v3] Init OK\n"); fflush(stdout);

    auto win = std::make_shared<unigui::Window>("main", "UniGUI v3.2.7 — Group Rows + Full Feature Showcase");
    win->SetSize(1260, 760);

    // P1: DataTable with group rows
    auto pDT = std::make_shared<unigui::Panel>("datatable", "DataTable<T> — Group Rows (click ▶/▼), Multi-Select, Filter, Add/Del, Sort All");
    pDT->SetContentCallback([]() {
        struct Pos { std::string sym; int lv, sv; double pnl, cost; };
        static std::vector<Pos> data;
        static bool init = true;
        if (init) {
            // Group A: rows 0-4
            data.push_back({"IF2406.CF",100,0,8000.0,5000.0}); data.push_back({"IF2409.CF",200,0,1200.0,5100.0});
            data.push_back({"IF2412.CF",0,150,-300.0,4900.0}); data.push_back({"IF2503.CF",50,0,600.0,5050.0});
            data.push_back({"IF2506.CF",0,0,0.0,5000.0});
            // Group B: rows 5-8
            data.push_back({"IH2406.CF",0,300,-500.0,4800.0}); data.push_back({"IH2409.CF",150,0,900.0,5100.0});
            data.push_back({"IH2412.CF",200,100,400.0,5000.0}); data.push_back({"IH2503.CF",0,0,0.0,5000.0});
            // Ungrouped: rows 9-10
            data.push_back({"IC2406.CF",300,0,1500.0,5200.0}); data.push_back({"IC2409.CF",100,200,-200.0,4900.0});
            init = false;
        }

        static char filter[64] = "";
        ImGui::InputTextWithHint("##f", "Filter...", filter, sizeof(filter));
        ImGui::SameLine();
        static unigui::Button addBtn("add","+Add"); addBtn.Render(); ImGui::SameLine();
        static unigui::Button delBtn("del","-Del"); delBtn.Render();

        static unigui::DataTable<Pos> t("持仓",{{"合约",110},{"多头",60},{"空头",60},{"盈亏",80},{"成本",80}});
        t.SetDataSource(&data); t.SetMultiSelect(true); t.SetColumnAutoWidth(0,true);
        t.SetCellFormatter([](int,int c,const Pos& p)->std::string{
            switch(c){case 0:return p.sym;case 1:return p.lv>0?std::to_string(p.lv):"-";
            case 2:return p.sv>0?std::to_string(p.sv):"-";
            case 3:{char b[32];snprintf(b,32,"%.0f",p.pnl);return b;}
            case 4:{char b[32];snprintf(b,32,"%.0f",p.cost);return b;}}return"-";
        });
        t.SetRowColor([](int,const Pos& p){return p.pnl>=0?IM_COL32(0,160,80,80):IM_COL32(220,60,60,80);});
        if(filter[0])t.SetFilterText(filter);else t.SetFilterText("");

        // Group model
        static std::vector<unigui::DataTable<Pos>::GroupInfo> groups;
        static bool ginit = true;
        if(ginit){groups={{.label="IF 品种组",.startRow=0,.endRow=5},{.label="IH 品种组",.startRow=5,.endRow=9}};ginit=false;}
        t.SetGroups(groups);
        t.Render();

        if(addBtn.WasClicked()){int n=(int)data.size();data.push_back({"NEW"+std::to_string(6001+n),500,0,1200.0,5100.0});}
        if(delBtn.WasClicked()){auto sel=t.GetSelectedRows();for(int s:sel)if(s>=0&&s<(int)data.size())data.erase(data.begin()+s);}
        auto sel=t.GetSelectedRows(); if(!sel.empty()) ImGui::Text("Selected %zu rows", sel.size());
    });
    win->AddPanel(pDT);

    // TreeView with custom renderer
    auto pTV = std::make_shared<unigui::Panel>("treeview", "TreeView — Custom Renderer (ProgressBar per node)");
    pTV->SetContentCallback([]() {
        static unigui::TreeNode root; static bool b=false;
        if(!b){root.label="Account Groups";
        unigui::TreeNode g1{"Group A",{},false};
        g1.children.push_back({"acc10001 | 1,000万 | 65%",{},false});g1.children.push_back({"acc10002 | 2,500万 | 42%",{},false});
        unigui::TreeNode g2{"Group B",{},false};
        g2.children.push_back({"acc10003 | 800万 | 88%",{},false});root.children.push_back(g1);root.children.push_back(g2);b=true;}
        static unigui::TreeView tv("tree"); tv.SetRoot(root);
        static std::vector<float> us={0.65f,0.42f,0.88f}; static int ui=0;
        tv.SetNodeRenderer([](int,int d,const unigui::TreeNode&){if(d>0&&ui<(int)us.size()){ImGui::SameLine(ImGui::GetWindowWidth()-180);ImGui::ProgressBar(us[ui],ImVec2(120,14),"");ImGui::SameLine();ImGui::Text("%.0f%%",us[ui]*100);ui++;}});
        ui=0; tv.Render();
    });
    win->AddPanel(pTV);

    // MultiSplitter
    auto pMS = std::make_shared<unigui::Panel>("multisplitter", "MultiSplitter — H+V Nested (3-col H outer, 2-row V inner)");
    pMS->SetContentCallback([]() {
        static unigui::MultiSplitter msH("msH",unigui::MultiSplitter::Horizontal); static bool b=false;
        if(!b){msH.AddPanel(0.25f,[](){static unigui::MultiSplitter msV("msV",unigui::MultiSplitter::Vertical);static bool vb=false;if(!vb){msV.AddPanel(0.4f,[](){ImGui::Text("Pod List");ImGui::Button("Pod 1");ImGui::Button("Pod 2");});msV.AddPanel(0.6f,[](){ImGui::Text("Account Info");});vb=true;}msV.Render();});
        msH.AddPanel(0.35f,[](){ImGui::Text("Positions");ImGui::TextUnformatted("IF2406 Long:100 PnL:+500");});
        msH.AddPanel(0.40f,[](){ImGui::Text("Chart");ImGui::TextUnformatted("[TimeSeriesChart]");});b=true;} msH.Render();
    });
    win->AddPanel(pMS);

    // Buttons + Toast
    auto pBtn = std::make_shared<unigui::Panel>("buttons", "Animated Buttons + Toast");
    pBtn->SetContentCallback([]() {
        static unigui::Button b1("b1","Primary");b1.SetColorVariant(unigui::Button::Primary);b1.Render();ImGui::SameLine();
        static unigui::Button b2("b2","Danger");b2.SetColorVariant(unigui::Button::Danger);b2.Render();ImGui::SameLine();
        static unigui::Button b3("b3","Success");b3.SetColorVariant(unigui::Button::Success);b3.Render();
        if(b1.WasClicked())unigui::Toast::Info("Primary");if(b2.WasClicked())unigui::Toast::Error("Danger");if(b3.WasClicked())unigui::Toast::Success("Success");
    });
    win->AddPanel(pBtn);

    // Badge + GradientText
    auto pBG = std::make_shared<unigui::Panel>("badge","Badge + FormatCN");
    pBG->SetContentCallback([]() {
        unigui::GradientText::RenderHex("UniGUI v3.2.7 — Feature Complete",40,49,237,233,69,96);ImGui::Spacing();
        ImGui::Text("Money: %s", unigui::format::MoneyCN(53000000LL).c_str());
        ImGui::Text("Volume: %s", unigui::format::VolumeCN(1500).c_str());
        static unigui::Badge dot("");dot.SetVariant(unigui::Badge::Dot);dot.Render();ImGui::SameLine(20);
        static unigui::Badge cnt("");cnt.SetCount(12);cnt.SetColor(IM_COL32(233,69,96,255));cnt.Render();ImGui::SameLine(60);
        static unigui::Badge lbl("NEW");lbl.SetColor(IM_COL32(0,180,100,255));lbl.Render();
    });
    win->AddPanel(pBG);

    int frame = 0; bool first=true;
    while (!unigui::ShouldClose()) {
        unigui::NewFrame();
        if(first){win->RestoreLayout(R"({"x":20,"y":20,"w":1260,"h":760})");first=false;}
        win->Render();
        unigui::Render(); frame++;
        if (max_frames > 0 && frame >= max_frames) break;
    }
    std::printf("[v3] Layout: %s\n", win->SaveLayout().c_str()); fflush(stdout);
    unigui::Shutdown();
    std::printf("[v3] Done — %d frames\n", frame); fflush(stdout);
    return 0;
}
